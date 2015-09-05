#include "simit-test.h"

#include "graph.h"
#include "ir.h"
#include "ir_printer.h"
#include "environment.h"
#include "lower/index_expressions/lower_scatter_workspace.h"
#include "util/util.h"
#include "util/collections.h"

using namespace std;
using namespace simit::ir;

struct SparseMatrix {
  string name;
  size_t M, N;
  vector<int> rowPtr;
  vector<int> colInd;
  vector<simit_float> vals;

  SparseMatrix() {} // TODO Remove

  SparseMatrix(string name, size_t M, size_t N,
               vector<int> rowPtr, vector<int> colInd, vector<simit_float> vals)
      : name(name), M(M), N(N), rowPtr(rowPtr), colInd(colInd), vals(vals) {}

  friend ostream& operator<<(ostream& os, const SparseMatrix& m) {
    os << m.name << ": " << endl;
    os << " " << simit::util::join(m.rowPtr) << endl;
    os << " " << simit::util::join(m.colInd) << endl;
    os << " " << simit::util::join(m.vals);
    return os;
  }
};

struct TestParams {
  Expr expr;
  vector<SparseMatrix> operands;
  SparseMatrix expected;

  TestParams(Expr expr, vector<SparseMatrix> operands, SparseMatrix expected)
      : expr(expr), operands(operands), expected(expected) {}

  friend ostream& operator<<(ostream& os, const TestParams& p) {
    IRPrinter printer(os);
    printer.skipTopExprParenthesis();
    os << "A(i,j) = ";
    printer.print(p.expr);
    os << endl;
    os << simit::util::join(p.operands, "\n");
    return os;
  }
};

class IndexExpressionTest : public ::testing::TestWithParam<TestParams> {};

static const Type VType = SetType::make(ElementType::make("Vertex", {}), {});
static const Var V("V", VType);
static const IndexDomain dim({V});

static const IndexVar i("i", dim);
static const IndexVar j("j", dim);

Var createMatrixVar(string name, IndexVar i, IndexVar j) {
  return Var(name, TensorType::make(typeOf<simit_float>(),
                                    {i.getDomain(), j.getDomain()}));
}

Expr B(IndexVar i, IndexVar j) {return Expr(createMatrixVar("B",i,j))(i,j);}
Expr C(IndexVar i, IndexVar j) {return Expr(createMatrixVar("C",i,j))(i,j);}
Expr D(IndexVar i, IndexVar j) {return Expr(createMatrixVar("D",i,j))(i,j);}

TEST_P(IndexExpressionTest, Matrix) {
  Expr expr = GetParam().expr;

  map<string,const SparseMatrix*> operandsFromNames;
  for (const SparseMatrix& operand : GetParam().operands) {
    iassert(!simit::util::contains(operandsFromNames, operand.name));
    operandsFromNames.insert({operand.name, &operand});
  }

  // Get the size of each set and verify sizes are consistent
  map<string, size_t> setSizes;
  vector<Var> setVars;
  match(expr,
    std::function<void(const VarExpr*)>([&](const VarExpr* op) {
      const Var& var = op->var;
      const SparseMatrix* operand = operandsFromNames.at(var.getName());
      iassert(var.getType().isTensor());
      vector<IndexDomain> dims = var.getType().toTensor()->getDimensions();
      for (const IndexDomain& dim : dims) {
        int currDim = 0;
        for (const IndexSet& is : dim.getIndexSets()) {
          if (is.getKind() == IndexSet::Set) {
            Expr setExpr = is.getSet();
            iassert(isa<VarExpr>(setExpr));
            const Var& setVar = to<VarExpr>(setExpr)->var;
            string setName = setVar.getName();
            size_t setSize = (currDim==0) ? operand->M : operand->N;
            if (simit::util::contains(setSizes, setName)) {
              iassert(setSizes.at(setName) == setSize)
                  << "inconsistent dimension size"  ;
            }
            else {
              setSizes.insert({setName, setSize});
              setVars.push_back(setVar);
            }
          }
          ++currDim;
        }
      }
    })
  );

  // TODO: add code to initialize result indices and vals from operands
  SparseMatrix result = GetParam().expected;

  vector<Var> vars;
  match(expr,
    std::function<void(const VarExpr*)>([&vars](const VarExpr* op) {
      vars.push_back(op->var);
    })
  );

  Var A = createMatrixVar("A", i,j);

  // Set up environment
  Environment env;
  for (auto& setVar : setVars) {
    env.addExtern(setVar);
  }
  env.addExtern(A);
  for (const Var& var : vars) {
    env.addExtern(var);
  }

  Expr iexpr = IndexExpr::make({i,j}, expr);
  Stmt loops = lowerScatterWorkspace(A, to<IndexExpr>(iexpr), &env);
//  std::cout << loops << std::endl;

  simit::Function function = getTestBackend()->compile(loops, env);

  // Find and bind set variables
  vector<unique_ptr<simit::Set>> sets;
  for (auto& setSizePair : setSizes) {
    string setName = setSizePair.first;
    size_t setSize = setSizePair.second;
    simit::Set* set = new simit::Set(setName);
    sets.push_back(unique_ptr<simit::Set>(set));
    for (size_t i=0; i < setSize; ++i) {
      set->add();
    }
    function.bind(setName, set);
  }

  // Bind matrices
  vector<SparseMatrix> operands = GetParam().operands;
  for (const SparseMatrix& mat : operands) {
    function.bind(mat.name, mat.rowPtr.data(), mat.colInd.data(),
                  (void*)mat.vals.data());
  }
  function.bind(result.name, result.rowPtr.data(),
                result.colInd.data(), (void*)result.vals.data());

  function.runSafe();

  // Check that the results are correct
  const SparseMatrix& expected = GetParam().expected;
  for (auto pair : simit::util::zip(result.vals, expected.vals)) {
    SIMIT_ASSERT_FLOAT_EQ(pair.first, pair.second)
        << "  Actual: " << simit::util::join(result.vals) << endl
        << "Expected: " << simit::util::join(expected.vals);
  }

  for (auto rowPtrs : simit::util::zip(result.rowPtr, expected.rowPtr)) {
    ASSERT_EQ(rowPtrs.second, rowPtrs.first)
        << result.name << "'s rowPtr index array "
        << "was changed by compute kernel" << endl
        << "  Actual: " << simit::util::join(result.rowPtr) << endl
        << "Original: " << simit::util::join(expected.rowPtr);
  }

  for (auto colInds : simit::util::zip(result.colInd, expected.colInd)) {
    ASSERT_EQ(colInds.second, colInds.first)
        << result.name << "'s colInd index array "
        << "was changed by compute kernel" << endl
        << "  Actual: " << simit::util::join(result.colInd) << endl
        << "Original: " << simit::util::join(expected.colInd);
  }

  // Check that the operands are unchanged
  for (auto operandPair : simit::util::zip(operands, GetParam().operands)) {
    auto& opCopy = operandPair.first;
    auto& opOrig = operandPair.second;

    for (auto rowPtrs : simit::util::zip(opCopy.rowPtr, opOrig.rowPtr)) {
      ASSERT_EQ(rowPtrs.second, rowPtrs.first)
          << operandPair.first.name << "'s rowPtr index array "
          << "was changed by compute kernel" << endl
          << "  Actual: " << simit::util::join(opCopy.rowPtr) << endl
          << "Original: " << simit::util::join(opOrig.rowPtr);
    }

    for (auto colInds : simit::util::zip(opCopy.colInd, opOrig.colInd)) {
      ASSERT_EQ(colInds.second, colInds.first)
          << operandPair.first.name << "'s colInd index array "
          << "was changed by compute kernel" << endl
          << "  Actual: " << simit::util::join(opCopy.colInd) << endl
          << "Original: " << simit::util::join(opOrig.colInd);
    }

    for (auto vals : simit::util::zip(opCopy.vals, opOrig.vals)) {
      SIMIT_ASSERT_FLOAT_EQ(vals.second, vals.first)
          << operandPair.first.name << "'s value array "
          << "was changed by compute kernel" << endl
          << "  Actual: " << simit::util::join(opCopy.vals) << endl
          << "Original: " << simit::util::join(opOrig.vals);
    }
  }
}

INSTANTIATE_TEST_CASE_P(Add, IndexExpressionTest,
                        testing::Values(
                        TestParams(
                          B(i,j) + C(i,j),
                          {
                            // 1.0 2.0 0.0
                            // 3.0 4.0 0.0
                            // 0.0 0.0 0.0
                            SparseMatrix("B", 3, 3,
                                         {0, 2, 4, 4},
                                         {0, 1, 0, 1},
                                         {1.0, 2.0, 3.0, 4.0}),
                            // 0.0 0.0 0.0
                            // 0.0 0.1 0.2
                            // 0.0 0.3 0.4
                            SparseMatrix("C", 3, 3,
                                         {0, 0, 2, 4},
                                         {1, 2, 1, 2},
                                         {0.1, 0.2, 0.3, 0.4})
                          },
                          // 1.0 2.0 0.0
                          // 3.0 4.1 0.2
                          // 0.0 0.3 0.4
                          SparseMatrix("A", 3, 3,
                                       {0, 2, 5, 7},
                                       {0, 1, 0, 1, 2, 1, 2},
                                       {1.0, 2.0, 3.0, 4.1, 0.2, 0.3, 0.4})
                        )
                        ));


INSTANTIATE_TEST_CASE_P(DISABLED_Mul, IndexExpressionTest,
                        testing::Values(
                        TestParams(
                          B(i,j) * C(i,j),
                          {
                            // 1.0 2.0 0.0
                            // 3.0 4.0 0.0
                            // 0.0 0.0 0.0
                            SparseMatrix("B", 3, 3,
                                         {0, 2, 4, 4},
                                         {0, 1, 0, 1},
                                         {1.0, 2.0, 3.0, 4.0}),
                            // 0.0 0.0 0.0
                            // 0.0 0.1 0.2
                            // 0.0 0.3 0.4
                            SparseMatrix("C", 3, 3,
                                         {0, 0, 2, 4},
                                         {1, 2, 1, 2},
                                         {0.1, 0.2, 0.3, 0.4})
                          },
                          // 0.0 0.0 0.0
                          // 0.0 0.4 0.0
                          // 0.0 0.0 0.0
                          SparseMatrix("A", 3, 3,
                                       {0, 0, 1, 1},
                                       {1},
                                       {0.4})
                        )
                        ));

TEST(IndexExpression, mul) {
  Type vertexType = ElementType::make("Vertex", {});
  Type vertexSetType = SetType::make(vertexType, {});
  Var vertexSet("V", vertexSetType);

  IndexDomain dim({vertexSet});
  IndexVar i("i", dim);
  IndexVar j("j", dim);

  Type tensorType = TensorType::make(ScalarType::Float, {dim,dim});
  Var  A = Var("A", tensorType);
  Expr B = Var("B", tensorType);
  Expr C = Var("C", tensorType);
  Expr add = IndexExpr::make({i,j}, B(i,j) * C(i,j));

  Environment env;
  env.addExtern(A);
  env.addExtern(to<VarExpr>(B)->var);
  env.addExtern(to<VarExpr>(C)->var);

  Stmt loops = lowerScatterWorkspace(A, to<IndexExpr>(add), &env);
  std::cout << loops << std::endl;
}

TEST(IndexExpression, addmul) {
  Type vertexType = ElementType::make("Vertex", {});
  Type vertexSetType = SetType::make(vertexType, {});
  Var vertexSet("V", vertexSetType);

  IndexDomain dim({vertexSet});
  IndexVar i("i", dim);
  IndexVar j("j", dim);

  Type tensorType = TensorType::make(ScalarType::Float, {dim,dim});
  Var  A = Var("A", tensorType);
  Expr B = Var("B", tensorType);
  Expr C = Var("C", tensorType);
  Expr D = Var("D", tensorType);
  Expr addmul = IndexExpr::make({i,j}, (B(i,j) + C(i,j)) * D(i,j));

  Environment env;
  env.addExtern(A);
  env.addExtern(to<VarExpr>(B)->var);
  env.addExtern(to<VarExpr>(C)->var);
  env.addExtern(to<VarExpr>(D)->var);

  Stmt loops = lowerScatterWorkspace(A, to<IndexExpr>(addmul), &env);
  std::cout << loops << std::endl;
}

TEST(IndexExpression, muladd) {
  Type vertexType = ElementType::make("Vertex", {});
  Type vertexSetType = SetType::make(vertexType, {});
  Var vertexSet("V", vertexSetType);

  IndexDomain dim({vertexSet});
  IndexVar i("i", dim);
  IndexVar j("j", dim);

  Type tensorType = TensorType::make(ScalarType::Float, {dim,dim});
  Var  A = Var("A", tensorType);
  Expr B = Var("B", tensorType);
  Expr C = Var("C", tensorType);
  Expr D = Var("D", tensorType);
  Expr muladd = IndexExpr::make({i,j}, (B(i,j) * C(i,j)) + D(i,j));

  Environment env;
  env.addExtern(A);
  env.addExtern(to<VarExpr>(B)->var);
  env.addExtern(to<VarExpr>(C)->var);
  env.addExtern(to<VarExpr>(D)->var);

  Stmt loops = lowerScatterWorkspace(A, to<IndexExpr>(muladd), &env);
  std::cout << loops << std::endl;
}
