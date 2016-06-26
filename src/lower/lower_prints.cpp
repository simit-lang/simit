#include "lower_prints.h"

#include "storage.h"
#include "ir_builder.h"
#include "ir_rewriter.h"
#include "ir_transforms.h"
#include "inline.h"
#include "path_expressions.h"
#include "tensor_index.h"
#include "types.h"
#include "index_expressions/loops.h"
#include "util/name_generator.h"
#include "util/util.h"
#include "util/collections.h"

#include <sstream>

namespace simit {
namespace ir {

class LowerPrints : public IRRewriter {
public:
  LowerPrints(Func func) : 
    storage(func.getStorage()), 
    environment(func.getEnvironment()) {} 

private:
  Storage storage;
  Environment environment;
  util::NameGenerator names;

  const Stmt printSpace = Print::make(" ");
  const Stmt printNewline = Print::make("\n");
  const Stmt printEqual = Print::make(" = ");
  const Stmt printBlank = Print::make(std::string(12, '-'));

  Stmt printIndent(unsigned int indentLevel) {
    return (indentLevel == 0) ? Pass::make() : 
           Print::make(std::string(3 * indentLevel, ' '));
  }

  bool isInnermost(const TensorType* tensor) {
    std::vector<IndexDomain> dimensions = tensor->getDimensions();
    
    for (size_t i = 0; i < dimensions.size(); ++i) {
      if (dimensions[i].getIndexSets().size() > 1) {
        return false;
      }
    }

    return true;
  }

  Stmt printIndices(std::vector<Expr> indices, unsigned int skipLast = 0) {
    std::vector<Stmt> stmts;
    std::stringstream oss;

    stmts.push_back(Print::make("("));
    for (size_t i = 0; i < indices.size() - skipLast; ++i) {
      if (i > 0) {
        stmts.push_back(Print::make(", "));
      }
      stmts.push_back(Print::make(indices[i]));
    }
    for (size_t i = 0; i < skipLast; ++i) {
      oss << ", :";
    }
    stmts.push_back(Print::make(oss.str() + ")"));
  
    return Block::make(stmts);
  }

  Stmt printTensor(Expr tensorExpr, unsigned int indentLevel = 0) {
    const TensorType* tensor = tensorExpr.type().toTensor();
    size_t order = tensor->order();
    
    if (order == 0) {
      // Print scalar value.
      return Block::make(printIndent(indentLevel), Print::make(tensorExpr));
    }

    std::vector<IndexSet> dimensions = tensor->getOuterDimensions();
    iassert(order == dimensions.size());

    TensorStorage tensorStorage = isElementTensorType(tensor) ? 
      TensorStorage::Kind::Dense : 
      storage.getStorage(to<VarExpr>(tensorExpr)->var);

    std::vector<Expr> tensorReadIndices;
    TensorIndexVar *tiVar = nullptr;
    Var inductionVar(names.getName(), Int);
    Var tensorReadIndex;
    Stmt printTensorStmt;

    switch (tensorStorage.getKind()) {
      case TensorStorage::Kind::Dense: {
        for (size_t i = 0; i < order; ++i) {
          tensorReadIndices.push_back(Var(names.getName(), Int));
        }

        break;
      }
      case TensorStorage::Kind::Indexed: {
        iassert(isa<VarExpr>(tensorExpr));
        iassert(order == 2);

        Var tensorVar = to<VarExpr>(tensorExpr)->var;

        // Get coord and sink arrays corresponding to indexed tensor.
        TensorIndex tensorIndex = tensorStorage.getTensorIndex();
        Expr coordArray = tensorIndex.getCoordArray();
        Expr sinkArray = tensorIndex.getSinkArray();

        tiVar = new TensorIndexVar(inductionVar.getName(), tensorVar.getName(), 
                                   inductionVar, tensorIndex);
        tensorReadIndices.push_back(VarExpr::make(inductionVar)); 
        tensorReadIndices.push_back(VarExpr::make(tiVar->getSinkVar()));
        
        break;
      }
      case TensorStorage::Kind::Diagonal: {
        iassert(order == dimensions.size());

        // Expand indices of element being read.
        tensorReadIndex = Var(names.getName(), Int);
        for (size_t i = 0; i < order; ++i) {
          tensorReadIndices.push_back(tensorReadIndex);
        }

        break;
      }
      default:
        iassert(false);
        break;
    }
    
    // Emit code for reading a single tensor component.
    Expr readElement = TensorRead::make(tensorExpr, tensorReadIndices);
    const std::string format = tensor->getComponentType() == ScalarType::Int ?
                               "13" : "13.5";
    Stmt printElementFormatted = Print::make(readElement, format);
  
    Var shouldPrintNewline = Var(names.getName(), Boolean);
    Stmt maybePrintNewline = IfThenElse::make(
      VarExpr::make(shouldPrintNewline), printNewline,
      AssignStmt::make(shouldPrintNewline, Literal::make(true)));

    // Handle printing for (innermost) column vectors as a special case. Each 
    // element is printed on a separate line, without indices printed as well.
    if (tensorStorage.getKind() == TensorStorage::Kind::Dense &&
        isInnermost(tensor) && order == 1 && tensor->isColumnVector) {
      printTensorStmt = ForRange::make(
        to<VarExpr>(tensorReadIndices[order - 1])->var, Literal::make(0), 
        Length::make(dimensions[order - 1]), 
        Block::make({maybePrintNewline, printIndent(indentLevel),
                     printElementFormatted})); 
      printTensorStmt = Block::make(
        AssignStmt::make(shouldPrintNewline, Literal::make(false)), 
        printTensorStmt);

      return printTensorStmt;
    }
    
    // Emit code for printing "large" tensors; each component of outermost 
    // tensor is to be printed on a separate line.
    Stmt printLargeTensor = isInnermost(tensor) ? Print::make(readElement) :
                            printTensor(readElement, indentLevel + 1);

    std::vector<Stmt> printLineStmts;
    if (tensorStorage.getKind() == TensorStorage::Kind::Indexed) {
      printLineStmts.push_back(tiVar->initSinkVar());
    }
    printLineStmts.push_back(maybePrintNewline);
    printLineStmts.push_back(printIndent(indentLevel));
    printLineStmts.push_back(printIndices(tensorReadIndices));
    printLineStmts.push_back(printEqual);
    if (!isInnermost(tensor)) {
      printLineStmts.push_back(printNewline);
    }
    printLineStmts.push_back(printLargeTensor);
    printLargeTensor = Block::make(printLineStmts);

    switch (tensorStorage.getKind()) {
      case TensorStorage::Kind::Dense: {
        printLargeTensor = ForRange::make(
          to<VarExpr>(tensorReadIndices[order - 1])->var, Literal::make(0), 
          Length::make(dimensions[order - 1]), printLargeTensor);
        if (order >= 2) {
          for (size_t i = 0; i <= order - 2; ++i) {
            const size_t idx = order - 2 - i;
            Var index = to<VarExpr>(tensorReadIndices[idx])->var;
            printLargeTensor = ForRange::make(index, Literal::make(0), 
                                              Length::make(dimensions[idx]), 
                                              printLargeTensor);
          }
        }

        break;
      }
      case TensorStorage::Kind::Indexed: {
        printLargeTensor = ForRange::make(tiVar->getCoordVar(),  
                                          tiVar->loadCoord(), 
                                          tiVar->loadCoord(1), 
                                          printLargeTensor);
        printLargeTensor = ForRange::make(inductionVar, Literal::make(0), 
                                          Length::make(dimensions[0]), 
                                          printLargeTensor);

        break;
      }
      case TensorStorage::Kind::Diagonal: {
        printLargeTensor = ForRange::make(tensorReadIndex, Literal::make(0), 
                                          Length::make(dimensions[0]), 
                                          printLargeTensor);

        break;
      }
      default:
        iassert(false);
        break;
    }

    printLargeTensor = Block::make(
      AssignStmt::make(shouldPrintNewline, Literal::make(false)), 
      printLargeTensor);

    Stmt printSmallTensor = Stmt();

    // Emit code for printing "small" tensors; vectors and matrices are 
    // formatted as such when printed.
    if (isInnermost(tensor)) {
      switch (tensorStorage.getKind()) {
        case TensorStorage::Kind::Dense: {
          // Column vector printing should have already been handled as a 
          // separate case.
          iassert(order != 1 || !tensor->isColumnVector);

          Expr printIndentCond = Eq::make(
            to<VarExpr>(tensorReadIndices[order - 1]), Literal::make(0));
          Stmt maybePrintIndent = IfThenElse::make(printIndentCond,
            printIndent(indentLevel + ((order >= 3) ? 1 : 0)));

          printSmallTensor = ForRange::make(
            to<VarExpr>(tensorReadIndices[order - 1])->var, Literal::make(0), 
            Length::make(dimensions[order - 1]), 
            Block::make({maybePrintIndent, printElementFormatted, printSpace})); 
          if (order >= 2) {
            printSmallTensor = ForRange::make(
              to<VarExpr>(tensorReadIndices[order - 2])->var, 
              Literal::make(0), Length::make(dimensions[order - 2]), 
              Block::make(maybePrintNewline, printSmallTensor));
            printSmallTensor = Block::make(
              AssignStmt::make(shouldPrintNewline, Literal::make(false)),
              printSmallTensor);
          }
          if (order >= 3) {
            Var shouldPrintNewline(names.getName(), Boolean);
            Stmt maybePrintNewline = IfThenElse::make(
              VarExpr::make(shouldPrintNewline), printNewline,
              AssignStmt::make(shouldPrintNewline, Literal::make(true)));
            Stmt printIndicesStmt = printIndices(tensorReadIndices, 2);

            printSmallTensor = Block::make({maybePrintNewline,
                                            printIndent(indentLevel),
                                            printIndicesStmt, printSpace,
                                            printEqual, printNewline,
                                            printSmallTensor});
            for (size_t i = 0; i <= order - 3; ++i) {
              const size_t idx = order - 3 - i;
              Var index = to<VarExpr>(tensorReadIndices[idx])->var;
              printSmallTensor = ForRange::make(index, Literal::make(0), 
                                                Length::make(dimensions[idx]), 
                                                printSmallTensor);
            }
            printSmallTensor = Block::make(
              AssignStmt::make(shouldPrintNewline, Literal::make(false)),
              printSmallTensor);
          }

          break;
        }
        case TensorStorage::Kind::Indexed: {
          Var columnVar(names.getName(), Int);

          Stmt initColumnVar = AssignStmt::make(columnVar, Literal::make(0));
          Stmt incColumnVar = AssignStmt::make(columnVar, 
            Add::make(VarExpr::make(columnVar), Literal::make(1)));
          Stmt printBlankElement = Block::make({printBlank, printSpace, 
                                                incColumnVar});
          Stmt printBlanks = While::make(
            Lt::make(VarExpr::make(columnVar), tiVar->getSinkVar()), 
            printBlankElement);
          Stmt printBlanksEnd = While::make(
            Lt::make(VarExpr::make(columnVar), Length::make(dimensions[1])), 
            printBlankElement);
          
          printSmallTensor = Block::make({tiVar->initSinkVar(), 
                                               printBlanks, incColumnVar, 
                                               printElementFormatted, 
                                               printSpace});
          printSmallTensor = ForRange::make(tiVar->getCoordVar(), 
                                            tiVar->loadCoord(), 
                                            tiVar->loadCoord(1), 
                                            printSmallTensor);
          printSmallTensor = Block::make({initColumnVar, maybePrintNewline,
                                          printIndent(indentLevel), 
                                          printSmallTensor, printBlanksEnd}); 
          printSmallTensor = ForRange::make(inductionVar, Literal::make(0), 
                                            Length::make(dimensions[0]), 
                                            printSmallTensor);
          printSmallTensor = Block::make( 
            AssignStmt::make(shouldPrintNewline, Literal::make(false)), 
            printSmallTensor);

          break;
        }
        case TensorStorage::Kind::Diagonal: {
          if (order == 2) {
            Stmt printBlankElement = Block::make(printBlank, printSpace);
            Stmt printStartBlanks = ForRange::make(inductionVar, 
                                                   Literal::make(0),
                                                   tensorReadIndex, 
                                                   printBlankElement);
            Stmt printEndBlanks = ForRange::make(inductionVar, 
              Add::make(VarExpr::make(tensorReadIndex), Literal::make(1)),
              Length::make(dimensions[0]), printBlankElement);
            
            printSmallTensor = Block::make({maybePrintNewline,
                                            printIndent(indentLevel),
                                            printStartBlanks, 
                                            printElementFormatted,
                                            printSpace, printEndBlanks});
            printSmallTensor = ForRange::make(tensorReadIndex, 
                                              Literal::make(0),
                                              Length::make(dimensions[0]),
                                              printSmallTensor);
            printSmallTensor = Block::make(
              AssignStmt::make(shouldPrintNewline, Literal::make(false)), 
              printSmallTensor);
          }

          break;
        }
        default:
          iassert(false);
          break;
      }
    }

    // If a tensor can be printed as a "large" or "small" tensor, then emit 
    // code that selects between two at runtime based on tensor size.
    if (printSmallTensor.defined()) {
      Expr isSmallTensor = Le::make(Length::make(dimensions[order - 1]),
                                    Literal::make(6));
      printTensorStmt = IfThenElse::make(isSmallTensor, printSmallTensor,
                                         printLargeTensor);
    } else {
      printTensorStmt = printLargeTensor;
    }

    if (tiVar != nullptr) {
      delete tiVar;
    }

    return printTensorStmt;
  }

  void visit(const Print* op) {
    Expr tensorExpr = op->expr;

    // If printing string, no lowering needed.
    const auto componentType = tensorExpr.type().toTensor()->getComponentType();
    stmt = (componentType == ScalarType::String) ? op : 
           printTensor(tensorExpr, 0);
  }
  
  void visit(const Func* f) {
    Stmt body = rewrite(f->getBody());
    func = Func(f->getName(), f->getArguments(), f->getResults(), body,
                environment);
    func.setStorage(storage);
  }
};

Func lowerPrints(Func func) {
  func = LowerPrints(func).rewrite(func);
  func = insertVarDecls(func);
  return func;
}

}}
