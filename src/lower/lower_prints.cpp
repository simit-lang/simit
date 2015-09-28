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
  const Stmt printBlank = Print::make(std::string(10, '-'));

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

  Stmt printInnerMostTensor(Expr tensorExpr, unsigned int indentLevel) {
    const TensorType* tensor = tensorExpr.type().toTensor();
    size_t order = tensor->order();

    if (order == 0) {
      return Block::make(printIndent(indentLevel), Print::make(tensorExpr));
    }

    Stmt printTensorStmt;
    std::vector<IndexSet> dimensions = tensor->getOuterDimensions();
    TensorStorage tensorStorage = isElementTensorType(tensor) ? 
      TensorStorage::Kind::Dense : 
      storage.getStorage(to<VarExpr>(tensorExpr)->var);

    switch (tensorStorage.getKind()) {
      case TensorStorage::Kind::Dense: {
        std::vector<Expr> tensorReadIndices;
       
        iassert(order == dimensions.size());

        for (size_t i = 0; i < order; ++i) {
          tensorReadIndices.push_back(Var(names.getName(), Int));
        }

        Expr readElement = TensorRead::make(tensorExpr, tensorReadIndices);
        
        Var shouldPrintNewline = Var(names.getName(), Boolean);
        Stmt maybePrintNewline = IfThenElse::make(
          VarExpr::make(shouldPrintNewline), printNewline,
          AssignStmt::make(shouldPrintNewline, Literal::make(true)));
        Stmt printElement = Print::make(readElement);
        Stmt printLine = (order == 1 && tensor->isColumnVector) ? 
          Block::make({maybePrintNewline, printIndent(indentLevel), 
                      printElement}) : 
          Block::make({maybePrintNewline, printIndent(indentLevel), 
                       printIndices(tensorReadIndices), printEqual, 
                       printElement});
        Stmt printLargeTensor = ForRange::make(
          to<VarExpr>(tensorReadIndices[order - 1])->var, Literal::make(0), 
          Length::make(dimensions[order - 1]), printLine);
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

        Stmt maybePrintIndent = IfThenElse::make(
          Eq::make(to<VarExpr>(tensorReadIndices[order - 1]), Literal::make(0)),
          printIndent(indentLevel + ((order >= 3) ? 1 : 0)), Pass::make());
        Stmt printSmallTensor = ForRange::make(
          to<VarExpr>(tensorReadIndices[order - 1])->var, Literal::make(0), 
          Length::make(dimensions[order - 1]), 
          Block::make({maybePrintIndent, 
                       Print::make(readElement, "10.5"), printSpace}));
        if (order >= 2) {
          printSmallTensor = ForRange::make(
            to<VarExpr>(tensorReadIndices[order - 2])->var, Literal::make(0),
            Length::make(dimensions[order - 2]), 
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
          printSmallTensor = Block::make({maybePrintNewline,
                                          printIndent(indentLevel),
                                          printIndices(tensorReadIndices, 2),
                                          printSpace, printEqual, printNewline,
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

        Expr isSmallTensor = Le::make(Length::make(dimensions[order - 1]),
                                      Literal::make(8));
        printTensorStmt = IfThenElse::make(isSmallTensor, printSmallTensor,
                                           printLargeTensor);

        break;
      }
      case TensorStorage::Kind::Indexed: {
        iassert(isa<VarExpr>(tensorExpr));
        iassert(order == 2);

        Var tensorVar = to<VarExpr>(tensorExpr)->var;
        const pe::PathExpression pexpr = tensorStorage.getPathExpression();
        
        if (!environment.hasTensorIndex(pexpr)) {
          environment.addTensorIndex(pexpr, tensorVar);
        }

        TensorIndex tensorIndex = environment.getTensorIndex(pexpr);
        Expr coordArray = tensorIndex.getCoordArray();
        Expr sinkArray = tensorIndex.getSinkArray();

        Var inductionVar(names.getName(), Int);
        TensorIndexVar tiVar(inductionVar.getName(), tensorVar.getName(), 
                             inductionVar, tensorIndex);
        std::vector<Expr> tensorReadIndices = {VarExpr::make(inductionVar), 
          VarExpr::make(tiVar.getSinkVar())};

        Expr readElement = TensorRead::make(tensorExpr, tensorReadIndices); 
        
        Var shouldPrintNewline = Var(names.getName(), Boolean);
        Stmt maybePrintNewline = IfThenElse::make(
          VarExpr::make(shouldPrintNewline), printNewline,
          AssignStmt::make(shouldPrintNewline, Literal::make(true)));
        Stmt printElement = Print::make(readElement);
        Stmt printLine = Block::make({tiVar.initSinkVar(), maybePrintNewline, 
                                      printIndent(indentLevel),
                                      printIndices(tensorReadIndices),
                                      printEqual, printElement});
        Stmt printLargeTensor = ForRange::make(tiVar.getCoordVar(), 
                                               tiVar.loadCoord(), 
                                               tiVar.loadCoord(1), printLine);
        printLargeTensor = ForRange::make(inductionVar, Literal::make(0), 
                                          Length::make(dimensions[0]), 
                                          printLargeTensor);
        printLargeTensor = Block::make(
          AssignStmt::make(shouldPrintNewline, Literal::make(false)), 
          printLargeTensor);

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
                                             Print::make(readElement, "10.5"), 
                                             printSpace});
        printSmallTensor = ForRange::make(tiVar.getCoordVar(), 
                                          tiVar.loadCoord(), tiVar.loadCoord(1), 
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
                                      Literal::make(8));
        printTensorStmt = IfThenElse::make(isSmallTensor, printSmallTensor,
                                           printLargeTensor);

        break;
      }
      case TensorStorage::Kind::Diagonal: {
        std::vector<Expr> tensorReadIndices;
       
        iassert(order == dimensions.size());

        Var tensorReadIndex = Var(names.getName(), Int);
        for (size_t i = 0; i < order; ++i) {
          tensorReadIndices.push_back(tensorReadIndex);
        }

        Expr readElement = TensorRead::make(tensorExpr, tensorReadIndices);
        
        Var shouldPrintNewline = Var(names.getName(), Boolean);
        Stmt maybePrintNewline = IfThenElse::make(
          VarExpr::make(shouldPrintNewline), printNewline,
          AssignStmt::make(shouldPrintNewline, Literal::make(true)));
        Stmt printLine = Block::make({maybePrintNewline, 
                                      printIndent(indentLevel),
                                      printIndices(tensorReadIndices),
                                      printEqual, Print::make(readElement)});
        Stmt printLargeTensor = ForRange::make(tensorReadIndex, 
                                               Literal::make(0), 
                                               Length::make(dimensions[0]), 
                                               printLine); 
        printLargeTensor = Block::make(
          AssignStmt::make(shouldPrintNewline, Literal::make(false)), 
          printLargeTensor);

        if (order == 2) {
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
                                               Print::make(readElement, "10.5"),
                                               printSpace, printEndBlanks});
          printSmallTensor = ForRange::make(tensorReadIndex, 
                                            Literal::make(0),
                                            Length::make(dimensions[0]),
                                            printSmallTensor);
          printSmallTensor = Block::make(
            AssignStmt::make(shouldPrintNewline, Literal::make(false)), 
            printSmallTensor);

          Expr isSmallTensor = Le::make(Length::make(dimensions[1]),
                                        Literal::make(8));
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

  Stmt printTensor(Expr tensorExpr, unsigned int indentLevel = 0) {
    const TensorType* tensor = tensorExpr.type().toTensor();
    
    if (isInnermost(tensor)) {
      return printInnerMostTensor(tensorExpr, indentLevel);
    }

    Stmt printTensorStmt;
    std::vector<IndexSet> dimensions = tensor->getOuterDimensions();
    TensorStorage tensorStorage = 
      storage.getStorage(to<VarExpr>(tensorExpr)->var);

    switch (tensorStorage.getKind()) {
      case TensorStorage::Kind::Dense: {
        std::vector<Expr> tensorReadIndices;
        size_t order = dimensions.size();
       
        iassert(order > 0);

        for (size_t i = 0; i < order; ++i) {
          tensorReadIndices.push_back(Var(names.getName(), Int));
        }

        Var shouldPrintNewline = Var(names.getName(), Boolean);
        Stmt maybePrintNewline = IfThenElse::make(
          VarExpr::make(shouldPrintNewline), printNewline,
          AssignStmt::make(shouldPrintNewline, Literal::make(true)));
        Expr readElement = TensorRead::make(tensorExpr, tensorReadIndices);
        printTensorStmt = printTensor(readElement, indentLevel + 1);
        printTensorStmt = Block::make({maybePrintNewline, 
                                       printIndent(indentLevel),
                                       printIndices(tensorReadIndices),
                                       printEqual, printNewline, 
                                       printTensorStmt});
        printTensorStmt = ForRange::make(
          to<VarExpr>(tensorReadIndices[order - 1])->var, Literal::make(0), 
          Length::make(dimensions[order - 1]), printTensorStmt);
        if (order >= 2) {
          for (size_t i = 0; i <= order - 2; ++i) {
            const size_t idx = order - 2 - i;
            Var index = to<VarExpr>(tensorReadIndices[idx])->var;
            printTensorStmt = ForRange::make(index, Literal::make(0), 
                                             Length::make(dimensions[idx]), 
                                             printTensorStmt);
          }
        }
        printTensorStmt = Block::make(
          AssignStmt::make(shouldPrintNewline, Literal::make(false)), 
          printTensorStmt);

        break;
      }
      case TensorStorage::Kind::Indexed: {
        iassert(isa<VarExpr>(tensorExpr));
        iassert(dimensions.size() == 2);

        Var tensorVar = to<VarExpr>(tensorExpr)->var;
        const pe::PathExpression pexpr = tensorStorage.getPathExpression();
        
        if (!environment.hasTensorIndex(pexpr)) {
          environment.addTensorIndex(pexpr, tensorVar);
        }

        TensorIndex tensorIndex = environment.getTensorIndex(pexpr);
        Expr coordArray = tensorIndex.getCoordArray();
        Expr sinkArray = tensorIndex.getSinkArray();

        Var inductionVar(names.getName(), Int);
        TensorIndexVar tiVar(inductionVar.getName(), tensorVar.getName(), 
                             inductionVar, tensorIndex);
        std::vector<Expr> tensorReadIndices = {VarExpr::make(inductionVar), 
          VarExpr::make(tiVar.getSinkVar())};
        
        Var shouldPrintNewline = Var(names.getName(), Boolean);
        Stmt maybePrintNewline = IfThenElse::make(
          VarExpr::make(shouldPrintNewline), printNewline,
          AssignStmt::make(shouldPrintNewline, Literal::make(true)));
        Expr readElement = TensorRead::make(tensorExpr, tensorReadIndices);
        printTensorStmt = printTensor(readElement, indentLevel + 1);
        printTensorStmt = Block::make({tiVar.initSinkVar(), maybePrintNewline,
                                       printIndent(indentLevel),
                                       printIndices(tensorReadIndices),
                                       printEqual, printNewline, 
                                       printTensorStmt});
        printTensorStmt = ForRange::make(tiVar.getCoordVar(), tiVar.loadCoord(), 
                                        tiVar.loadCoord(1), printTensorStmt);
        printTensorStmt = ForRange::make(inductionVar, Literal::make(0), 
                                         Length::make(dimensions[0]), 
                                         printTensorStmt);
        printTensorStmt = Block::make(
          AssignStmt::make(shouldPrintNewline, Literal::make(false)), 
          printTensorStmt);

        break;
      }
      case TensorStorage::Kind::Diagonal: {
        std::vector<Expr> tensorReadIndices;
        size_t order = dimensions.size();
       
        iassert(order > 0);

        Var tensorReadIndex = Var(names.getName(), Int);
        for (size_t i = 0; i < order; ++i) {
          tensorReadIndices.push_back(tensorReadIndex);
        }

        Var shouldPrintNewline = Var(names.getName(), Boolean);
        Stmt maybePrintNewline = IfThenElse::make(
          VarExpr::make(shouldPrintNewline), printNewline,
          AssignStmt::make(shouldPrintNewline, Literal::make(true)));
        Expr readElement = TensorRead::make(tensorExpr, tensorReadIndices);
        printTensorStmt = printTensor(readElement, indentLevel + 1);
        printTensorStmt = Block::make({maybePrintNewline, 
                                       printIndent(indentLevel),
                                       printIndices(tensorReadIndices),
                                       printEqual, printNewline, 
                                       printTensorStmt});
        printTensorStmt = ForRange::make(tensorReadIndex, Literal::make(0), 
                                         Length::make(dimensions[0]), 
                                         printTensorStmt);
        printTensorStmt = Block::make(
          AssignStmt::make(shouldPrintNewline, Literal::make(false)), 
          printTensorStmt);

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

    if (!tensorExpr.defined()) {
      stmt = Print::make(op->str);
      return;
    }

    iassert(tensorExpr.type().isTensor());
    iassert(isa<VarExpr>(tensorExpr));

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
