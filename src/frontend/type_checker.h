#ifndef SIMIT_TYPE_CHECKER_H
#define SIMIT_TYPE_CHECKER_H

#include <vector>
#include <memory>
#include <string>
#include <exception>
#include <set>
#include <unordered_map>

#include "domain.h"
#include "error.h"
#include "fir.h"
#include "fir_printer.h"
#include "fir_rewriter.h"
#include "fir_visitor.h"
#include "ir.h"
#include "util/scopedmap.h"

namespace simit {
namespace fir {

// Type checking pass for identifying type errors, redefinitions, and 
// undeclared identifiers.
class TypeChecker : public FIRVisitor {
public:
  TypeChecker(const std::vector<fir::FuncDecl::Ptr> &intrinsics,
              std::vector<ParseError> *errors);

  void check(Program::Ptr);

private:
  virtual void visit(Program::Ptr);
  virtual void visit(StmtBlock::Ptr);
  virtual void visit(SetIndexSet::Ptr);
  virtual void visit(GenericIndexSet::Ptr op) {}
  virtual void visit(ElementType::Ptr);
  virtual void visit(Endpoint::Ptr);
  virtual void visit(UnstructuredSetType::Ptr);
  virtual void visit(LatticeLinkSetType::Ptr);
  virtual void visit(TupleType::Ptr);
  virtual void visit(NDTensorType::Ptr);
  virtual void visit(IdentDecl::Ptr);
  virtual void visit(ElementTypeDecl::Ptr);
  virtual void visit(ExternDecl::Ptr);
  virtual void visit(FuncDecl::Ptr);
  virtual void visit(VarDecl::Ptr);
  virtual void visit(ConstDecl::Ptr);
  virtual void visit(WhileStmt::Ptr);
  virtual void visit(IfStmt::Ptr);
  virtual void visit(IndexSetDomain::Ptr);
  virtual void visit(RangeDomain::Ptr);
  virtual void visit(ForStmt::Ptr);
  virtual void visit(PrintStmt::Ptr);
  virtual void visit(ExprStmt::Ptr);
  virtual void visit(AssignStmt::Ptr);
  virtual void visit(ExprParam::Ptr);
  virtual void visit(MapExpr::Ptr);
  virtual void visit(OrExpr::Ptr);
  virtual void visit(AndExpr::Ptr);
  virtual void visit(XorExpr::Ptr);
  virtual void visit(EqExpr::Ptr);
  virtual void visit(NotExpr::Ptr);
  virtual void visit(AddExpr::Ptr);
  virtual void visit(SubExpr::Ptr);
  virtual void visit(MulExpr::Ptr);
  virtual void visit(DivExpr::Ptr);
  virtual void visit(ElwiseMulExpr::Ptr);
  virtual void visit(ElwiseDivExpr::Ptr);
  virtual void visit(LeftDivExpr::Ptr);
  virtual void visit(NegExpr::Ptr);
  virtual void visit(ExpExpr::Ptr);
  virtual void visit(TransposeExpr::Ptr);
  virtual void visit(CallExpr::Ptr);
  virtual void visit(TensorReadExpr::Ptr);
  virtual void visit(SetReadExpr::Ptr);
  virtual void visit(TupleReadExpr::Ptr);
  virtual void visit(FieldReadExpr::Ptr);
  virtual void visit(ParenExpr::Ptr);
  virtual void visit(VarExpr::Ptr);
  virtual void visit(IntLiteral::Ptr);
  virtual void visit(FloatLiteral::Ptr);
  virtual void visit(BoolLiteral::Ptr);
  virtual void visit(ComplexLiteral::Ptr);
  virtual void visit(StringLiteral::Ptr);
  virtual void visit(IntVectorLiteral::Ptr);
  virtual void visit(FloatVectorLiteral::Ptr);
  virtual void visit(ComplexVectorLiteral::Ptr);
  virtual void visit(NDTensorLiteral::Ptr);
  virtual void visit(ApplyStmt::Ptr);
  virtual void visit(Test::Ptr);

private:
  enum class Access {NONE, READ, WRITE, READ_WRITE};

  class DimError : public std::exception {
    const char *what() const noexcept { return "mismatched dimension sizes"; }
  };
  class TypeError : public std::exception {
    const char *what() const noexcept { return "mismatched element types"; }
  };

  struct DenseTensorType {
    enum class Type {UNKNOWN, INT, FLOAT, COMPLEX};

    DenseTensorType() : dimSizes(1), type(Type::UNKNOWN) {};

    void addDimension() { dimSizes.push_back(1); }
    void addIntValues(unsigned);
    void addFloatValues(unsigned);
    void addComplexValues(unsigned);
    void merge(const DenseTensorType &);

    std::vector<unsigned> dimSizes;
    Type                  type;
  };

  struct ExprType {
    private:
      bool isScalarType(ScalarType::Type scalarType) const {
        return isScalar() && (to<ScalarType>(type[0])->type == scalarType);
      }

      bool isNumericType(ScalarType::Type scalarType) const;

    public:
      typedef std::vector<Type::Ptr> TypeVector;

      ExprType(Access access = Access::NONE) : access(access), defined(false) {}
      ExprType(Type::Ptr type, Access access = Access::READ) : 
          ExprType(TypeVector(1, type), access) {}
      ExprType(const TypeVector &type, Access access = Access::READ) :
          type(type), access(access), defined(true) {}

      bool isVoid() const {
        iassert(defined);
        return type.empty();
      }

      bool isSingleValue() const {
        iassert(defined);
        return type.size() == 1;
      }

      bool isTensor() const {
        return isSingleValue() && isa<TensorType>(type[0]);
      }
      
      bool isScalar() const {
        return isSingleValue() && isa<ScalarType>(type[0]);
      }

      bool isString() const {
        return isScalarType(ScalarType::Type::STRING); 
      }

      bool isScalarInt() const {
        return isScalarType(ScalarType::Type::INT);
      }

      bool isScalarBoolean() const {
        return isScalarType(ScalarType::Type::BOOL);
      }

      bool isScalarNumeric() const {
        return isScalar() && isNumericType(to<ScalarType>(type[0])->type);
      }

      bool isNumericTensor() const;

      bool isReadable() const;
      bool isWritable() const;

      TypeVector type;
      Access     access;
      bool       defined;
  };

  typedef std::unordered_map<std::string, IndexSet::Ptr> SetReplacementMap;
  typedef std::vector<IndexSet::Ptr>                     IndexDomain;
  typedef std::vector<IndexDomain>                       TensorDimensions;
  
  struct ReplaceTypeParams : public FIRRewriter {
    using FIRRewriter::visit;

    ReplaceTypeParams(SetReplacementMap &specializedSets) : 
        specializedSets(specializedSets) {}
    
    virtual void visit(GenericIndexSet::Ptr);
    virtual void visit(IndexSetDomain::Ptr);
    virtual void visit(RangeConst::Ptr);

    SetReplacementMap &specializedSets;
  };

  class Environment {
    private:
      struct SymbolType {
        SymbolType() = default;
        SymbolType(Type::Ptr type, Access access) : 
            type(type), access(access) {}

        Type::Ptr type;
        Access    access;
      };

    public:
      typedef std::unordered_map<std::string, Type::Ptr> ElementMap;
      typedef util::ScopedMap<std::string, SymbolType>   SymbolTable;
      typedef SymbolTable::SearchScope                   Scope;

    private:
      typedef std::unordered_map<std::string, FuncDecl::Ptr> FuncMap;
      typedef std::unordered_map<std::string, ElementMap>    ElementDeclMap;
      typedef std::unordered_map<FIRNode::Ptr, SetType::Ptr> SetDefinitionMap;

    public:
      void scope() { symbolTable.scope(); }
      void unscope() { symbolTable.unscope(); }

      void addFunction(const std::string& name, FuncDecl::Ptr decl) {
        funcDecls[name] = decl;
      }

      bool hasFunction(const std::string& name) const {
        return funcDecls.find(name) != funcDecls.end();
      }

      FuncDecl::Ptr getFunction(const std::string& name) const {
        iassert(hasFunction(name));
        return funcDecls.at(name);
      }

      void addFunctionReplacement(const std::string& sig, FuncDecl::Ptr decl) {
        funcReplace[sig] = decl;
      }

      bool hasFunctionReplacement(const std::string& sig) const {
        return funcReplace.find(sig) != funcReplace.end();
      }

      FuncDecl::Ptr getFunctionReplacement(const std::string& sig) const {
        iassert(hasFunctionReplacement(sig));
        return funcReplace.at(sig);
      }

      void addElementType(const std::string& name, const ElementMap &map) {
        elementDecls[name] = map;
      }

      bool hasElementType(const std::string& name) const {
        return elementDecls.find(name) != elementDecls.end();
      }

      bool hasElementField(const std::string& elemName, 
                           const std::string& fieldName) const {
        iassert(hasElementType(elemName));
        const auto &elemDecl = elementDecls.at(elemName);
        return elemDecl.find(fieldName) != elemDecl.end();
      }

      Type::Ptr getElementField(const std::string& elemName, 
                                const std::string& fieldName) const {
        iassert(hasElementField(elemName, fieldName));
        return elementDecls.at(elemName).at(fieldName);
      }

      void addSymbol(const std::string&, Type::Ptr, Access);

      bool hasSymbol(const std::string& name, Scope scope = Scope::All) const {
        return symbolTable.contains(name, scope);
      }

      Type::Ptr getSymbolType(const std::string& name, 
                              Scope scope = Scope::All) const {
        iassert(hasSymbol(name, scope));
        return symbolTable.get(name, scope).type;
      }

      Access getSymbolAccess(const std::string& name, 
                             Scope scope = Scope::All) const {
        iassert(hasSymbol(name, scope));
        return symbolTable.get(name, scope).access;
      }

      void addSetDefinition(SetIndexSet::Ptr set, SetType::Ptr def) {
        set->setDef = def;
      }

      void addSetDefinition(GenericParam::Ptr set, SetType::Ptr def) {
        setDefs[set] = def;
      }

      bool hasSetDefinition(SetIndexSet::Ptr set) const {
        return (bool)(set->setDef);
      }

      bool hasSetDefinition(GenericParam::Ptr set) const {
        return setDefs.find(set) != setDefs.end();
      }

      SetType::Ptr getSetDefinition(SetIndexSet::Ptr set) const {
        iassert(hasSetDefinition(set));
        return set->setDef;
      }

      SetType::Ptr getSetDefinition(GenericParam::Ptr set) const {
        iassert(hasSetDefinition(set));
        return setDefs.at(set);
      }

      bool compareIndexSets(IndexSet::Ptr, IndexSet::Ptr);
      bool compareDomains(const IndexDomain&, const IndexDomain&);
      bool compareTypes(Type::Ptr, Type::Ptr, bool = false);
      
    private:
      FuncMap          funcDecls;
      FuncMap          funcReplace;
      ElementDeclMap   elementDecls;
      SymbolTable      symbolTable;
      SetDefinitionMap setDefs;
  };
  
  class ComputeSetDefinitions : public FIRVisitor {
    public:
      ComputeSetDefinitions(Environment& env) : env(env) {}

      void compute(Program::Ptr program) { program->accept(this); }

    private:
      virtual void visit(SetIndexSet::Ptr);
      virtual void visit(IdentDecl::Ptr);
      virtual void visit(FieldDecl::Ptr op) { FIRVisitor::visit(op); }
      virtual void visit(FuncDecl::Ptr);
      virtual void visit(VarDecl::Ptr);
      virtual void visit(WhileStmt::Ptr);
      virtual void visit(IfStmt::Ptr);
      virtual void visit(ForStmt::Ptr);
      virtual void visit(AssignStmt::Ptr);

      typedef util::ScopedMap<std::string, SetType::Ptr> SymbolTable;

      SymbolTable  decls;
      Environment &env;
  };

  struct GenericCallTypeChecker {
    GenericCallTypeChecker(Environment& env) : env(env) {}

    void unify(IndexSet::Ptr, IndexSet::Ptr);
    void unify(Type::Ptr, Type::Ptr);

    SetReplacementMap  specializedSets;
    Environment       &env;
  };

private:
  void typeCheckVarOrConstDecl(VarDecl::Ptr, bool = false, bool = false);
  void typeCheckMapOrApply(MapExpr::Ptr, bool = false);
  void typeCheckBinaryElwise(BinaryExpr::Ptr, bool = false);
  void typeCheckBinaryBoolean(BinaryExpr::Ptr);
  void typeCheckDenseTensorLiteral(DenseTensorLiteral::Ptr);

  DenseTensorType getDenseTensorType(DenseTensorLiteral::Ptr);

  ExprType inferType(Expr::Ptr);
  bool     typeCheck(FIRNode::Ptr);
  bool     typeCheckGlobalConstDecl(ConstDecl::Ptr);

  static void getDimensions(TensorType::Ptr, TensorDimensions&);

  static ScalarType::Type getComponentType(TensorType::Ptr);
  static TensorDimensions getDimensions(TensorType::Ptr);
  static unsigned         getOrder(TensorType::Ptr);
  static bool             getTransposed(TensorType::Ptr);

  static std::string getConcretizedTypeSignatureString(FuncDecl::Ptr);

  static std::string toString(ExprType);
  static std::string toString(Type::Ptr, bool = true);
  static std::string toString(ScalarType::Type);
  
  void reportError(const std::string&, FIRNode::Ptr);
  void reportUndeclared(const std::string&, const std::string&, FIRNode::Ptr);
  void reportMultipleDefs(const std::string&, const std::string&, FIRNode::Ptr);

private:
  ExprType retType;
  bool     retTypeChecked; 
  
  bool        skipCheckDeclared;
  Environment env;
  
  std::vector<ParseError> *errors;
};

}
}

#endif

