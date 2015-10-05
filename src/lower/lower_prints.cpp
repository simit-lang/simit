#include "lower_prints.h"

#include "storage.h"
#include "ir_builder.h"
#include "ir_rewriter.h"
#include "ir_transforms.h"
#include "inline.h"
#include "path_expressions.h"
#include "tensor_index.h"
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

    Stmt printTensorStmt;
    
    std::vector<IndexSet> dimensions = tensor->getOuterDimensions();
    iassert(order == dimensions.size());

    TensorStorage tensorStorage = isElementTensorType(tensor) ? 
      TensorStorage::Kind::Dense : 
      storage.getStorage(to<VarExpr>(tensorExpr)->var);

    switch (tensorStorage.getKind()) {
      case TensorStorage::Kind::Dense: {
        std::vector<Expr> tensorReadIndices;
      
        for (size_t i = 0; i < order; ++i) {
          tensorReadIndices.push_back(Var(names.getName(), Int));
        }

        Expr readElement = TensorRead::make(tensorExpr, tensorReadIndices);
       
        // Emit code for printing "large" dense tensors; each element of 
        // outermost tensor is printed on separate lines.
        Var shouldPrintNewline = Var(names.getName(), Boolean);
        Stmt maybePrintNewline = IfThenElse::make(
          VarExpr::make(shouldPrintNewline), printNewline,
          AssignStmt::make(shouldPrintNewline, Literal::make(true)));
        Stmt printLargeTensor = isInnermost(tensor) ? Print::make(readElement) :
                                printTensor(readElement, indentLevel + 1);
        if (isInnermost(tensor)) {
          printLargeTensor = Block::make({maybePrintNewline, 
                                          printIndent(indentLevel),
                                          printIndices(tensorReadIndices),
                                          printEqual, printLargeTensor}); 
        } else {
          printLargeTensor = Block::make({maybePrintNewline, 
                                          printIndent(indentLevel),
                                          printIndices(tensorReadIndices),
                                          printEqual, printNewline, 
                                          printLargeTensor});
        }
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
        printLargeTensor = Block::make(
          AssignStmt::make(shouldPrintNewline, Literal::make(false)), 
          printLargeTensor);

        if (isInnermost(tensor)) {
          if (order == 1 && tensor->isColumnVector) {
            // Emit code for printing tensor as column vector if applicable; 
            // as with "large" tensors, each element is printed on a separate 
            // line, except without indices printed as well.
            printTensorStmt = ForRange::make(
              to<VarExpr>(tensorReadIndices[order - 1])->var, Literal::make(0), 
              Length::make(dimensions[order - 1]), 
              Block::make({maybePrintNewline, printIndent(indentLevel),
                           Print::make(readElement, "12.5")})); 
            printTensorStmt = Block::make(
              AssignStmt::make(shouldPrintNewline, Literal::make(false)), 
              printTensorStmt);
          } else {
            // If dense tensor is innermost and not a column vector, then emit 
            // code for printing "small" tensors; matrices and row vectors are 
            // printed in typical format, while higher-order tensors are 
            // "sliced" up into individual matrices.
            Expr printIndentCond = Eq::make(
              to<VarExpr>(tensorReadIndices[order - 1]), Literal::make(0));
            Stmt maybePrintIndent = IfThenElse::make(printIndentCond,
              printIndent(indentLevel + ((order >= 3) ? 1 : 0)));
            Stmt printSmallTensor = ForRange::make(
              to<VarExpr>(tensorReadIndices[order - 1])->var, Literal::make(0), 
              Length::make(dimensions[order - 1]), 
              Block::make({maybePrintIndent, 
                           Print::make(readElement, "12.5"), printSpace})); 
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
              Var shouldPrintNewline = Var(names.getName(), Boolean);
              Stmt maybePrintNewline = IfThenElse::make(
                VarExpr::make(shouldPrintNewline), printNewline,
                AssignStmt::make(shouldPrintNewline, Literal::make(true)));
              Stmt printIndicesStmt = printIndices(tensorReadIndices, 2);
              printSmallTensor = Block::make({maybePrintNewline,
                                              printIndent(indentLevel),
                                              printIndicesStmt,
                                              printSpace, printEqual, 
                                              printNewline, printSmallTensor});
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

            Expr isSmallTensor = Le::make(Length::make(dimensions[order - 1]),
                                          Literal::make(7));
            printTensorStmt = IfThenElse::make(isSmallTensor, printSmallTensor,
                                               printLargeTensor);
          }
        } else {
          printTensorStmt = printLargeTensor;
        }

        break;
      }
      case TensorStorage::Kind::Indexed: {
        iassert(isa<VarExpr>(tensorExpr));
        iassert(order == 2);

        Var tensorVar = to<VarExpr>(tensorExpr)->var;
        const pe::PathExpression pexpr = tensorStorage.getPathExpression();
        
        // Create tensor index for tensor being printed, if necessary.
        if (!environment.hasTensorIndex(pexpr)) {
          environment.addTensorIndex(pexpr, tensorVar);
        }

        // Get coord and sink arrays corresponding to indexed tensor.
        TensorIndex tensorIndex = environment.getTensorIndex(pexpr);
        Expr coordArray = tensorIndex.getCoordArray();
        Expr sinkArray = tensorIndex.getSinkArray();

        Var inductionVar(names.getName(), Int);
        TensorIndexVar tiVar(inductionVar.getName(), tensorVar.getName(), 
                             inductionVar, tensorIndex);
        std::vector<Expr> tensorReadIndices = {VarExpr::make(inductionVar), 
          VarExpr::make(tiVar.getSinkVar())};
        
        Expr readElement = TensorRead::make(tensorExpr, tensorReadIndices);
        
        // Emit code for printing "large" indexed tensors; each element of 
        // outermost tensor is printed on separate lines.
        Var shouldPrintNewline = Var(names.getName(), Boolean);
        Stmt maybePrintNewline = IfThenElse::make(
          VarExpr::make(shouldPrintNewline), printNewline,
          AssignStmt::make(shouldPrintNewline, Literal::make(true)));
        Stmt printLargeTensor = isInnermost(tensor) ? Print::make(readElement) :
                                printTensor(readElement, indentLevel + 1);
        if (isInnermost(tensor)) {
          printLargeTensor = Block::make({tiVar.initSinkVar(), 
                                          maybePrintNewline,
                                          printIndent(indentLevel),
                                          printIndices(tensorReadIndices),
                                          printEqual, printLargeTensor}); 
        } else {
          printLargeTensor = Block::make({tiVar.initSinkVar(), 
                                          maybePrintNewline,
                                          printIndent(indentLevel),
                                          printIndices(tensorReadIndices),
                                          printEqual, printNewline, 
                                          printLargeTensor});
        }
        printLargeTensor = ForRange::make(tiVar.getCoordVar(), 
                                          tiVar.loadCoord(), 
                                          tiVar.loadCoord(1), printLargeTensor);
        printLargeTensor = ForRange::make(inductionVar, Literal::make(0), 
                                          Length::make(dimensions[0]), 
                                          printLargeTensor);
        printLargeTensor = Block::make(
          AssignStmt::make(shouldPrintNewline, Literal::make(false)), 
          printLargeTensor);

        // If tensor is innermost, emit code for printing "small" tensors.
        if (isInnermost(tensor)) {
          Var columnVar = Var(names.getName(), Int);
          Stmt initColumnVar = AssignStmt::make(columnVar, Literal::make(0));
          Stmt incColumnVar = AssignStmt::make(columnVar, 
            Add::make(VarExpr::make(columnVar), Literal::make(1)));
          Stmt printBlankElement = Block::make({printBlank, printSpace, 
                                                incColumnVar});
          Stmt printBlanks = While::make(
            Lt::make(VarExpr::make(columnVar), tiVar.getSinkVar()), 
            printBlankElement);
          Stmt printBlanksEnd = While::make(
            Lt::make(VarExpr::make(columnVar), Length::make(dimensions[1])), 
            printBlankElement);
          Stmt printSmallTensor = Block::make({tiVar.initSinkVar(), printBlanks,
                                               incColumnVar, 
                                               Print::make(readElement, "12.5"), 
                                               printSpace});
          printSmallTensor = ForRange::make(tiVar.getCoordVar(), 
                                            tiVar.loadCoord(), 
                                            tiVar.loadCoord(1), 
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

          Expr isSmallTensor = Le::make(Length::make(dimensions[order - 1]),
                                        Literal::make(7));
          printTensorStmt = IfThenElse::make(isSmallTensor, printSmallTensor,
                                             printLargeTensor);
        } else {
          printTensorStmt = printLargeTensor;
        }

        break;
      }
      case TensorStorage::Kind::Diagonal: {
        std::vector<Expr> tensorReadIndices;
       
        iassert(order == dimensions.size());

        // Expand indices of element being read.
        Var tensorReadIndex = Var(names.getName(), Int);
        for (size_t i = 0; i < order; ++i) {
          tensorReadIndices.push_back(tensorReadIndex);
        }

        Expr readElement = TensorRead::make(tensorExpr, tensorReadIndices);
        
        // Emit code for printing "large" diagonal tensors; each element of 
        // outermost tensor is printed on separate lines.
        Var shouldPrintNewline = Var(names.getName(), Boolean);
        Stmt maybePrintNewline = IfThenElse::make(
          VarExpr::make(shouldPrintNewline), printNewline,
          AssignStmt::make(shouldPrintNewline, Literal::make(true)));
        Stmt printLargeTensor = isInnermost(tensor) ? Print::make(readElement) :
                                printTensor(readElement, indentLevel + 1);
        if (isInnermost(tensor)) {
          printLargeTensor = Block::make({maybePrintNewline, 
                                          printIndent(indentLevel),
                                          printIndices(tensorReadIndices),
                                          printEqual, printLargeTensor}); 
        } else {
          printLargeTensor = Block::make({maybePrintNewline, 
                                          printIndent(indentLevel),
                                          printIndices(tensorReadIndices),
                                          printEqual, printNewline, 
                                          printLargeTensor});
        }
        printLargeTensor = ForRange::make(tensorReadIndex, Literal::make(0), 
                                          Length::make(dimensions[0]), 
                                          printLargeTensor);
        printLargeTensor = Block::make(
          AssignStmt::make(shouldPrintNewline, Literal::make(false)), 
          printLargeTensor);

        // If tensor is innermost and second-order (i.e. a matrix), then emit 
        // code for printing "small" tensors.
        if (isInnermost(tensor) && order == 2) {
          Var inductionVar = Var(names.getName(), Int);
          Stmt printBlankElement = Block::make(printBlank, printSpace);
          Stmt printStartBlanks = ForRange::make(inductionVar, Literal::make(0),
                                                 tensorReadIndex, 
                                                 printBlankElement);
          Stmt printEndBlanks = ForRange::make(inductionVar, 
            Add::make(VarExpr::make(tensorReadIndex), Literal::make(1)),
            Length::make(dimensions[0]), printBlankElement);
          Stmt printSmallTensor = Block::make({maybePrintNewline,
                                               printIndent(indentLevel),
                                               printStartBlanks, 
                                               Print::make(readElement, "12.5"),
                                               printSpace, printEndBlanks});
          printSmallTensor = ForRange::make(tensorReadIndex, 
                                            Literal::make(0),
                                            Length::make(dimensions[0]),
                                            printSmallTensor);
          printSmallTensor = Block::make(
            AssignStmt::make(shouldPrintNewline, Literal::make(false)), 
            printSmallTensor);

          Expr isSmallTensor = Le::make(Length::make(dimensions[1]),
                                        Literal::make(7));
          printTensorStmt = IfThenElse::make(isSmallTensor, printSmallTensor,
                                             printLargeTensor);
        } else {
          printTensorStmt = printLargeTensor;
        }

        break;
      }
      default:
        iassert(false);
        break;
    }

    return printTensorStmt;
  }

  void visit(const Print* op) {
    Expr tensorExpr = op->expr;

    // If printing string, no lowering needed.
    if (!tensorExpr.defined()) {
      stmt = Print::make(op->str);
      return;
    }

    iassert(tensorExpr.type().isTensor());
    stmt = Block::make(printTensor(tensorExpr, 1), printNewline);
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
