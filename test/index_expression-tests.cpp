#include "simit-test.h"

#include "graph.h"
#include "ir.h"
#include "ir_printer.h"
#include "environment.h"
#include "lower/index_expressions/lower_scatter_workspace.h"
#include "path_expression_analysis.h"
#include "path_expressions.h"
#include "util/util.h"
#include "util/collections.h"

using namespace std;
using namespace simit;
using namespace simit::ir;

struct SparseMatrix {
  string name;
  size_t M, N;
  vector<int> rowPtr;
  vector<int> colInd;
  vector<simit_float> vals;

  SparseMatrix(string name, size_t M, size_t N,
               vector<int> rowPtr, vector<int> colInd, vector<simit_float> vals)
      : name(name), M(M), N(N), rowPtr(rowPtr), colInd(colInd), vals(vals) {
    iassert(rowPtr.size() == M+1);
    iassert(colInd.size() == (unsigned)rowPtr[rowPtr.size()-1]);
    iassert(vals.size()%colInd.size() == 0);
  }

  friend ostream& operator<<(ostream& os, const SparseMatrix& m) {
    os << m.name << ": " << endl;
    os << " " << util::join(m.rowPtr) << endl;
    os << " " << util::join(m.colInd) << endl;
    os << " " << util::join(m.vals);
    return os;
  }
};

struct TestParams {
  vector<IndexVar> ivars;
  Expr expr;
  vector<SparseMatrix> operands;
  SparseMatrix expected;

  TestParams(vector<IndexVar> ivars, Expr expr, vector<SparseMatrix> operands,
             SparseMatrix expected)
      : ivars(ivars), expr(expr), operands(operands), expected(expected) {}

  friend ostream& operator<<(ostream& os, const TestParams& p) {
    IRPrinter printer(os);
    printer.skipTopExprParenthesis();
    os << "A(i,j) = ";
    printer.print(p.expr);
    os << endl;
    os << util::join(p.operands, "\n");
    return os;
  }
};

class IndexExpression : public ::testing::TestWithParam<TestParams> {};

static const Type VType = SetType::make(ElementType::make("Vertex", {}), {});
static const Var V("V", VType);

static const IndexDomain dim({V});
static const IndexVar i("i", dim);
static const IndexVar j("j", dim);

static const IndexDomain dimb({IndexSet(V),IndexSet(2)});
static const IndexVar ib("ib", dimb);
static const IndexVar jb("jb", dimb);

Var createMatrixVar(string name, IndexVar i, IndexVar j) {
  return Var(name, ir::TensorType::make(ir::typeOf<simit_float>(),
                                        {i.getDomain(), j.getDomain()}));
}

Expr B(IndexVar i, IndexVar j) {return Expr(createMatrixVar("B",i,j))(i,j);}
Expr C(IndexVar i, IndexVar j) {return Expr(createMatrixVar("C",i,j))(i,j);}
Expr D(IndexVar i, IndexVar j) {return Expr(createMatrixVar("D",i,j))(i,j);}

TEST_P(IndexExpression, Matrix) {
  vector<IndexVar> ivars = GetParam().ivars;
  IndexVar i = ivars[0];
  IndexVar j = ivars[1];

  Expr expr = GetParam().expr;

  map<string,const SparseMatrix*> operandsFromNames;
  for (const SparseMatrix& operand : GetParam().operands) {
    iassert(!util::contains(operandsFromNames, operand.name));
    operandsFromNames.insert({operand.name, &operand});
  }

  // Get the size of each set and verify sizes are consistent
  map<string, size_t> setSizes;
  vector<Var> setVars;
  match(expr,
    std::function<void(const VarExpr*)>([&](const VarExpr* op) {
      const Var& var = op->var;
      iassert(util::contains(operandsFromNames, var.getName()));
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
            if (util::contains(setSizes, setName)) {
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

  vector<Var> vars;
  match(expr,
    std::function<void(const VarExpr*)>([&vars](const VarExpr* op) {
      vars.push_back(op->var);
    })
  );

  Var A = createMatrixVar("A", i,j);

  // Set up environment
  Environment env;
  Storage storage;
  for (auto& setVar : setVars) {
    env.addExtern(setVar);
  }
  env.addExtern(A);
  storage.add(A, TensorStorage::Kind::Indexed);
  for (const Var& var : vars) {
    env.addExtern(var);
    storage.add(var, TensorStorage::Kind::Indexed);
  }

  Expr iexpr = IndexExpr::make({i,j}, expr);
  Stmt loops = lowerScatterWorkspace(A, to<IndexExpr>(iexpr), &env, &storage);

  // TODO: add code to initialize result indices and vals from operands
//  PathExpressionBuilder builder;
//  std::cout << iexpr << std::endl;
//  pe::PathExpression pe = builder.computePathExpression(A,to<IndexExpr>(iexpr));
//  std::cout << pe << std::endl;
  SparseMatrix result = GetParam().expected;
  for (size_t i=0; i < result.vals.size(); ++i) {
    result.vals[i] = 0.0;
  }
  simit::Function function = getTestBackend()->compile(loops, env, storage);

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
  for (auto pair : util::zip(result.vals, expected.vals)) {
    simit_float resultVal = pair.first;
    simit_float expectedVal = pair.second;
    SIMIT_ASSERT_FLOAT_EQ(expectedVal, resultVal)
        << "  Actual: " << util::join(result.vals) << endl
        << "Expected: " << util::join(expected.vals);
  }

  for (auto rowPtrs : util::zip(result.rowPtr, expected.rowPtr)) {
    ASSERT_EQ(rowPtrs.second, rowPtrs.first)
        << result.name << "'s rowPtr index array "
        << "was changed by compute kernel" << endl
        << "  Actual: " << util::join(result.rowPtr) << endl
        << "Original: " << util::join(expected.rowPtr);
  }

  for (auto colInds : util::zip(result.colInd, expected.colInd)) {
    ASSERT_EQ(colInds.second, colInds.first)
        << result.name << "'s colInd index array "
        << "was changed by compute kernel" << endl
        << "  Actual: " << util::join(result.colInd) << endl
        << "Original: " << util::join(expected.colInd);
  }

  // Check that the operands are unchanged
  for (auto operandPair : util::zip(operands, GetParam().operands)) {
    auto& opCopy = operandPair.first;
    auto& opOrig = operandPair.second;

    for (auto rowPtrs : util::zip(opCopy.rowPtr, opOrig.rowPtr)) {
      ASSERT_EQ(rowPtrs.second, rowPtrs.first)
          << operandPair.first.name << "'s rowPtr index array "
          << "was changed by compute kernel" << endl
          << "  Actual: " << util::join(opCopy.rowPtr) << endl
          << "Original: " << util::join(opOrig.rowPtr);
    }

    for (auto colInds : util::zip(opCopy.colInd, opOrig.colInd)) {
      ASSERT_EQ(colInds.second, colInds.first)
          << operandPair.first.name << "'s colInd index array "
          << "was changed by compute kernel" << endl
          << "  Actual: " << util::join(opCopy.colInd) << endl
          << "Original: " << util::join(opOrig.colInd);
    }

    for (auto vals : util::zip(opCopy.vals, opOrig.vals)) {
      SIMIT_ASSERT_FLOAT_EQ(vals.second, vals.first)
          << operandPair.first.name << "'s value array "
          << "was changed by compute kernel" << endl
          << "  Actual: " << util::join(opCopy.vals) << endl
          << "Original: " << util::join(opOrig.vals);
    }
  }
}

INSTANTIATE_TEST_CASE_P(Add, IndexExpression,
                        testing::Values(
                        TestParams({i,j},
                          B(i,j) + C(i,j),
                          {
                            SparseMatrix("B", 3, 3,
                                         {0, 2, 4, 4},
                                         {0,1, 0,1},
                                         {1.0, 2.0,
                                          3.0, 4.0
                                                       }),
                            SparseMatrix("C", 3, 3,
                                         {0, 0, 2, 4},
                                         {1, 2, 1, 2},
                                         {
                                               5.0, 6.0,
                                               7.0, 8.0})
                          },
                          SparseMatrix("A", 3, 3,
                                       {0, 2, 5, 7},
                                       {0,1, 0,1,2, 1,2},
                                       {1.0, 2.0,
                                        3.0, 9.0, 6.0,
                                             7.0, 8.0})
                        )
                        ,
                        TestParams({i,j},
                          B(i,j) + C(i,j) + D(i,j),
                          {
                            SparseMatrix("B", 3, 3,
                                         {0, 2, 4, 4},
                                         {0,1, 0,1},
                                         {1.0, 2.0,
                                          3.0, 4.0
                                                       }),
                            SparseMatrix("C", 3, 3,
                                         {0, 0, 2, 4},
                                         {1,2, 1,2},
                                         {
                                               5.0, 6.0,
                                               7.0, 8.0}),
                            SparseMatrix("D", 3, 3,
                                         {0, 2, 5, 7},
                                         {1,2, 0,1,2, 1,2},
                                         {     0.1, 0.2,
                                          0.3, 0.4, 0.5,
                                               0.6, 0.7})
                          },
                          SparseMatrix("A", 3, 3,
                                       {0, 3, 6, 8},
                                       {0,1,2, 0,1,2, 1,2},
                                       {1.0, 2.1, 0.2,
                                        3.3, 9.4, 6.5,
                                             7.6, 8.7})
                        )
                        ,
                        TestParams({ib,jb},
                          B(ib,jb) + C(ib,jb),
                          {
                            SparseMatrix("B", 3, 3,
                                         {0, 2, 4, 4},
                                         {0,1, 0,1},
                                         {1.1, 1.2, 1.3, 1.4,
                                          2.1, 2.2, 2.3, 2.4,
                                          3.1, 3.2, 3.3, 3.4,
                                          4.1, 4.2, 4.3, 4.4}),
                            SparseMatrix("C", 3, 3,
                                         {0, 0, 2, 4},
                                         {1,2, 1,2},
                                         {5.1, 5.2, 5.3, 5.4,
                                          6.1, 6.2, 6.3, 6.4,
                                          7.1, 7.2, 7.3, 7.4,
                                          8.1, 8.2, 8.3, 8.4})
                          },
                          SparseMatrix("A", 3, 3,
                                       {0, 2, 5, 7},
                                       {0,1, 0,1,2, 1,2},
                                       {1.1, 1.2, 1.3, 1.4,
                                        2.1, 2.2, 2.3, 2.4,
                                        3.1, 3.2, 3.3, 3.4,
                                        9.2, 9.4, 9.6, 9.8,
                                        6.1, 6.2, 6.3, 6.4,
                                        7.1, 7.2, 7.3, 7.4,
                                        8.1, 8.2, 8.3, 8.4})
                        )
                        ));


INSTANTIATE_TEST_CASE_P(Mul, IndexExpression,
                        testing::Values(
                        TestParams({i,j},
                          B(i,j) * C(i,j),
                          {
                            SparseMatrix("B", 3, 3,
                                         {0, 2, 4, 4},
                                         {0, 1, 0, 1},
                                         {1.0, 2.0,
                                          3.0, 4.0
                                                       }),
                            SparseMatrix("C", 3, 3,
                                         {0, 0, 2, 4},
                                         {1, 2, 1, 2},
                                         {
                                               0.1, 0.2,
                                               0.3, 0.4})
                          },
                          SparseMatrix("A", 3, 3,
                                       {0, 0, 1, 1},
                                       {1},
                                       {
                                             0.4
                                                     })
                        ),
                        TestParams({i,j},
                          B(i,j) * C(i,j) * D(i,j),
                          {
                            SparseMatrix("B", 3, 3,
                                         {0, 2, 4, 4},
                                         {0,1, 0,1},
                                         {1.0, 2.0,
                                          3.0, 4.0
                                                       }),
                            SparseMatrix("C", 3, 3,
                                         {0, 0, 2, 4},
                                         {1,2, 1,2},
                                         {
                                               5.0, 6.0,
                                               7.0, 8.0}),
                            SparseMatrix("D", 3, 3,
                                         {0, 2, 5, 7},
                                         {1,2, 0,1,2, 1,2},
                                         {     0.1, 0.2,
                                          0.3, 0.4, 0.5,
                                               0.6, 0.7})
                          },
                          SparseMatrix("A", 3, 3,
                                       {0, 0, 1, 1},
                                       {1},
                                       {
                                             8.0
                                                     })
                        )
                        ));

INSTANTIATE_TEST_CASE_P(Mixed, IndexExpression,
                        testing::Values(
                        TestParams({i,j},
                          (B(i,j) + C(i,j)) * D(i,j),
                          {
                            SparseMatrix("B", 3, 3,
                                         {0, 2, 4, 4},
                                         {0,1, 0,1},
                                         {1.0, 2.0,
                                          3.0, 4.0
                                                       }),
                            SparseMatrix("C", 3, 3,
                                         {0, 0, 2, 4},
                                         {1,2, 1,2},
                                         {
                                               5.0, 6.0,
                                               7.0, 8.0}),
                            SparseMatrix("D", 3, 3,
                                         {0, 2, 5, 7},
                                         {1,2, 0,1,2, 1,2},
                                         {     0.1, 0.2,
                                          0.3, 0.4, 0.5,
                                               0.6, 0.7})
                          },
                          SparseMatrix("A", 3, 3,
                                       {0, 1, 4, 6},
                                       {1, 0,1,2, 1,2},
                                       {     0.2,
                                        0.9, 3.6, 3.0,
                                             4.2, 5.6})
                        ),
                        TestParams({i,j},
                          B(i,j) + (C(i,j) * D(i,j)),
                          {
                            SparseMatrix("B", 3, 3,
                                         {0, 2, 4, 4},
                                         {0,1, 0,1},
                                         {1.0, 2.0,
                                          3.0, 4.0
                                                       }),
                            SparseMatrix("C", 3, 3,
                                         {0, 0, 2, 4},
                                         {1,2, 1,2},
                                         {
                                               5.0, 6.0,
                                               7.0, 8.0}),
                            SparseMatrix("D", 3, 3,
                                         {0, 2, 5, 7},
                                         {1,2, 0,1,2, 1,2},
                                         {     0.1, 0.2,
                                          0.3, 0.4, 0.5,
                                               0.6, 0.7})
                          },
                          SparseMatrix("A", 3, 3,
                                       {0, 2, 5, 7},
                                       {0,1, 0,1,2, 1,2},
                                       {1.0, 2.0,
                                        3.0, 6.0, 3.0,
                                             4.2, 5.6})
                        )
                        ));

TEST(DISABLED_IndexExpression, vecadd) {
  Type VType = SetType::make(ElementType::make("Vertex", {}), {});
  Var V("V", VType);
  IndexDomain dim({V});

  IndexVar i("i", dim);

  Type vectorType = ir::TensorType::make(ScalarType::Float, {dim});

  // a = b+c
  Var  a = Var("a", vectorType);
  Expr b = Var("b", vectorType);
  Expr c = Var("c", vectorType);

  Expr iexpr = IndexExpr::make({i}, b(i)+c(i));

  Environment env;
  env.addExtern(to<VarExpr>(b)->var);
  env.addExtern(to<VarExpr>(c)->var);

  Storage storage;

  Stmt loops = lowerScatterWorkspace(a, to<IndexExpr>(iexpr), &env, &storage);
  std::cout << loops << std::endl;
}

TEST(DISABLED_IndexExpression, gemv) {
  Type VType = SetType::make(ElementType::make("Vertex", {}), {});
  Var V("V", VType);
  IndexDomain dim({V});

  IndexVar i("i", dim);
  IndexVar j("j", dim, ReductionOperator::Sum);

  Type vectorType = ir::TensorType::make(ScalarType::Float, {dim});
  Type matrixType = ir::TensorType::make(ScalarType::Float, {dim,dim});

  // a = Bc
  Var  a = Var("a", vectorType);
  Expr B = Var("B", matrixType);
  Expr c = Var("c", vectorType);

  Expr iexpr = IndexExpr::make({i}, B(i,j)*c(j));

  Environment env;
  env.addExtern(a);
  env.addExtern(to<VarExpr>(B)->var);
  env.addExtern(to<VarExpr>(c)->var);

  Storage storage;

  Stmt loops = lowerScatterWorkspace(a, to<IndexExpr>(iexpr), &env, &storage);
  std::cout << loops << std::endl;
}

