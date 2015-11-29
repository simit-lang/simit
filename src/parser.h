#ifndef PARSER_NEW_H
#define PARSER_NEW_H

#include <exception>
#include <vector>
#include <map>
#include <utility>

#include "scanner.h"
#include "program_context.h"
#include "error.h"
#include "ir.h"
#include "types.h"
#include "domain.h"

namespace simit { 
namespace internal {

class ParseException : public std::exception {};

class ParserNew {
public:
  void parse(const TokenList &, ProgramContext *, std::vector<ParseError> *);

private:
  struct Argument {
    ir::Var var;
    bool isInout;

    Argument(ir::Var var, bool isInout) : var(var), isInout(isInout) {}
  };

  struct ForDomain {
    enum class Type {DOMAIN, RANGE};

    static ForDomain make(ir::IndexSet domain) {
      ForDomain newRange;

      newRange.type = Type::DOMAIN;
      newRange.domain = domain;
      
      return newRange;
    }
    static ForDomain make(ir::Expr lower, ir::Expr upper) {
      ForDomain newRange;

      newRange.type = Type::RANGE;
      newRange.lower = lower;
      newRange.upper = upper;

      return newRange;
    }

    Type type;
    ir::IndexSet domain;
    ir::Expr lower;
    ir::Expr upper;
  };

  class CallMap {
    public:
      struct Entry {
        enum class Type {FUNC, MAP};

        static Entry make(ir::Func func, std::vector<ir::Expr> actuals) {
          Entry newEntry;
          
          newEntry.callType = Type::FUNC;
          newEntry.func = func;
          newEntry.actuals = actuals;

          return newEntry;
        }
        static Entry make(ir::Func func, std::vector<ir::Expr> partial_actuals, 
                          ir::Expr target, ir::Expr neighbors,
                          ir::ReductionOperator reduction) {
          Entry newEntry;

          newEntry.callType = Type::MAP;
          newEntry.func = func;
          newEntry.actuals = partial_actuals;
          newEntry.target = target;
          newEntry.neighbors = neighbors;
          newEntry.reduction = reduction;

          return newEntry;
        }

        Type callType;
        ir::Func func;
        std::vector<ir::Expr> actuals;
        ir::Expr target;
        ir::Expr neighbors;
        ir::ReductionOperator reduction;
      };

    public:
      inline void add(ir::Var var, Entry entry) {
        calls[var] = entry;
        orderedCalls.push_back(std::make_pair(var, entry));
      }
      inline bool exists(ir::Var var) const { 
        return calls.find(var) != calls.end();
      }
      inline const Entry get(ir::Var var) const { 
        return calls.find(var)->second;
      }
      inline const std::list<std::pair<ir::Var, Entry>> &getAllInOrder() const {
        return orderedCalls;
      }

    private:
      std::map<ir::Var, Entry> calls;
      std::list<std::pair<ir::Var, Entry>> orderedCalls;
  };

  struct TensorValues {
      enum class Type {UNKNOWN, INT, FLOAT};

      TensorValues() : dimSizes(1), type(Type::UNKNOWN) {};

      void addIntValue(const int &val) {
        switch (type) {
          case Type::UNKNOWN:
            type = Type::INT;
          case Type::INT:
            break;
          default:
            // TODO: raise error
            break;
        }

        intVals.push_back(val);
        dimSizes[dimSizes.size()-1]++;
      }
      void addFloatValue(const double &val) {
        switch (type) {
          case Type::UNKNOWN:
            type = Type::FLOAT;
          case Type::FLOAT:
            break;
          default:
            // TODO: raise error
            break;
        }

        floatVals.push_back(val);
        dimSizes[dimSizes.size() - 1]++;
      }
      void addDimension() { dimSizes.push_back(1); }

      void merge(const TensorValues &other) {
        if (type != other.type) {
          // TODO: raise error
        }

        if (dimSizes.size() - 1 != other.dimSizes.size()) {
          // TODO: raise error
        }

        for (unsigned int i = 0; i < dimSizes.size() - 1; ++i) {
          if (dimSizes[i] != other.dimSizes[i]) {
            // TODO: raise error
          }
        }

        switch (type) {
          case Type::INT:
            intVals.insert(intVals.end(), other.intVals.begin(), 
                           other.intVals.end());
            break;
          case Type::FLOAT:
            floatVals.insert(floatVals.end(), other.floatVals.begin(), 
                             other.floatVals.end());
            break;
          default:
            break;
        }
        dimSizes[dimSizes.size() - 1]++;
      }

      std::vector<unsigned int> dimSizes;
      std::vector<int>           intVals;
      std::vector<double>      floatVals;
      Type                          type;
  };
  
  void parseProgram();
  void parseProgramElement();
  void parseElementTypeDecl();
  std::vector<ir::Field> parseFieldDeclList();
  ir::Field parseFieldDecl();
  void parseExternDecl();
  void parseFunction();
  void parseProcedure();
  ir::Func parseArgsAndResults();
  std::vector<Argument> parseArguments();
  Argument parseArgumentDecl();
  std::vector<ir::Var> parseResults();
  ir::Stmt parseStmtBlock();
  void parseStmt();
  void parseVarDecl();
  void parseConstDecl();
  ir::Var parseIdentDecl();
  void parseWhileStmt();
  void parseDoUntilStmt();
  void parseIfStmt();
  ir::Stmt parseElseClause();
  void parseForStmt();
  ForDomain parseForStmtRange();
  void parsePrintStmt();
  void parseExprOrAssignStmt();
  ir::Expr parseExpr(CallMap &);
  ir::Expr parseMapExpr(CallMap &);
  ir::Expr parseOrExpr(CallMap &);
  ir::Expr parseAndExpr(CallMap &);
  ir::Expr parseXorExpr(CallMap &);
  ir::Expr parseEqExpr(CallMap &);
  ir::Expr parseIneqExpr(CallMap &);
  ir::Expr parseBooleanFactor(CallMap &);
  ir::Expr parseSolveExpr(CallMap &);
  ir::Expr parseAddExpr(CallMap &);
  ir::Expr parseMulExpr(CallMap &);
  ir::Expr parseNegExpr(CallMap &);
  ir::Expr parseExpExpr(CallMap &);
  ir::Expr parseTransposeExpr(CallMap &);
  ir::Expr parseCallOrReadExpr(CallMap &);
  ir::Expr parseFactor(CallMap &);
  std::vector<ir::Expr> parseOptionalExprList(CallMap &);
  std::vector<ir::Expr> parseExprList(CallMap &);
  ir::Expr parseExprListElement(CallMap &);
  ir::Type parseType();
  ir::Type parseElementType();
  ir::Type parseSetType();
  std::vector<ir::Expr> parseEndpoints();
  ir::Type parseTupleType();
  ir::Type parseTensorType();
  ir::Type parseTensorTypeStart();
  std::vector<ir::IndexSet> parseIndexSets();
  ir::IndexSet parseIndexSet();
  ir::Expr parseTensorLiteral();
  ir::Expr parseSignedNumLiteral();
  TensorValues parseDenseTensorLiteral();
  TensorValues parseDenseTensorLiteralInner();
  TensorValues parseDenseMatrixLiteral();
  TensorValues parseDenseVectorLiteral();
  TensorValues parseDenseIntVectorLiteral();
  TensorValues parseDenseFloatVectorLiteral();
  int parseSignedIntLiteral();
  double parseSignedFloatLiteral();
  void parseTest();
  //void parseSystemGenerator();
  //void parseExternAssert();

  void checkValidSubexpr(const ir::Expr, const CallMap &);
  void checkValidReadExpr(const ir::Expr);
  void checkValidWriteExpr(const ir::Expr);
  void emitAssign(const std::vector<ir::Expr> &, ir::Expr, const CallMap &);
  std::vector<ir::Stmt> liftCallsAndMaps(const CallMap &, 
                                         const ir::Expr = ir::Expr());

  inline Token consume(TokenType type) { 
    Token token = peek();
    if (!tokens.consume(type)) {
      throw ParseException();
    }
    return token;
  }
  inline Token peek(unsigned int k = 0) { return tokens.peek(k); }

  TokenList tokens;
  ProgramContext *ctx;
  std::vector<ParseError> *errors;
};

}
}

#endif

