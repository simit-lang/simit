#include "fir_intrinsics.h"

#include "intrinsics.h"
#include "types.h"

namespace simit {
namespace fir {

static void
addIntrinsic(std::vector<FuncDecl::Ptr>* intrinsics,
             const std::string& name,
             const std::vector<Type::Ptr> &args,
             const std::vector<Type::Ptr> &results,
             const std::vector<GenericParam::Ptr> &genericParams = {}){
  const auto decl = std::make_shared<FuncDecl>();

  decl->name = std::make_shared<Identifier>();
  decl->name->ident = name;
  decl->type = FuncDecl::Type::EXTERNAL;

  decl->genericParams = genericParams;

  for (unsigned i = 0; i < args.size(); ++i) {
    const auto arg = std::make_shared<Argument>();
    arg->name = std::make_shared<Identifier>();
    arg->name->ident = "a" + to_string(i+1);
    arg->type = args[i];
    decl->args.push_back(arg);
  }

  for (unsigned i = 0; i < results.size(); ++i) {
    const auto res = std::make_shared<IdentDecl>();
    res->name = std::make_shared<Identifier>();
    res->name->ident = "r" + to_string(i+1);
    res->type = results[i];
    decl->results.push_back(res);
  }

  intrinsics->push_back(decl);
}

static void
addScalarIntrinsic(std::vector<FuncDecl::Ptr>* intrinsics,
                   const std::string& name,
                   const std::vector<ScalarType::Type> &args,
                   const std::vector<ScalarType::Type> &results) {
  std::vector<Type::Ptr> argTypes(args.size());
  std::vector<Type::Ptr> resultTypes(results.size());

  for (unsigned i = 0; i < args.size(); ++i) {
    argTypes[i] = makeTensorType(args[i]);
  }
  for (unsigned i = 0; i < results.size(); ++i) {
    resultTypes[i] = makeTensorType(results[i]);
  }

  addIntrinsic(intrinsics, name, argTypes, resultTypes);
}

std::vector<FuncDecl::Ptr> createIntrinsics() {
  std::vector<FuncDecl::Ptr> intrinsics;

  const auto threeDim = std::make_shared<RangeIndexSet>();
  threeDim->range = 3;

  const auto threeByThreeTensorType = std::make_shared<NDTensorType>();
  threeByThreeTensorType->blockType = makeTensorType(ScalarType::Type::FLOAT);
  threeByThreeTensorType->indexSets = {threeDim, threeDim};

  const auto threeVectorType = std::make_shared<NDTensorType>();
  threeVectorType->blockType = makeTensorType(ScalarType::Type::FLOAT);
  threeVectorType->indexSets = {threeDim};

  auto N = std::make_shared<GenericParam>();
  N->type = GenericParam::Type::UNKNOWN;
  N->name = "N";

  auto M = std::make_shared<GenericParam>();
  M->type = GenericParam::Type::UNKNOWN;
  M->name = "M";

  auto K = std::make_shared<GenericParam>();
  K->type = GenericParam::Type::UNKNOWN;
  K->name = "K";

  const auto nDim = std::make_shared<GenericIndexSet>();
  nDim->type = GenericIndexSet::Type::UNKNOWN;
  nDim->setName = N->name;

  const auto mDim = std::make_shared<GenericIndexSet>();
  mDim->type = GenericIndexSet::Type::UNKNOWN;
  mDim->setName = M->name;

  const auto kDim = std::make_shared<fir::GenericIndexSet>();
  kDim->type = fir::GenericIndexSet::Type::UNKNOWN;
  kDim->setName = K->name;

  const auto nVectorType = std::make_shared<NDTensorType>();
  nVectorType->blockType = makeTensorType(ScalarType::Type::FLOAT);
  nVectorType->indexSets = {nDim};

  const auto mVectorType = std::make_shared<NDTensorType>();
  mVectorType->blockType = makeTensorType(ScalarType::Type::FLOAT);
  mVectorType->indexSets = {mDim};

  const auto nnMatrixType = std::make_shared<NDTensorType>();
  nnMatrixType->blockType = makeTensorType(ScalarType::Type::FLOAT);
  nnMatrixType->indexSets = {nDim, nDim};

  const auto nmMatrixType = std::make_shared<NDTensorType>();
  nmMatrixType->blockType = makeTensorType(ScalarType::Type::FLOAT);
  nmMatrixType->indexSets = {nDim, mDim};

  const auto kmMatrixType = std::make_shared<NDTensorType>();
  kmMatrixType->blockType = makeTensorType(ScalarType::Type::FLOAT);
  kmMatrixType->indexSets = {kDim, mDim};

  const auto opaqueType = std::make_shared<OpaqueType>();

  // Add type signatures for intrinsic functions.
  addScalarIntrinsic(&intrinsics,
                     ir::intrinsics::mod().getName(),
                     {ScalarType::Type::INT, ScalarType::Type::INT},
                     {ScalarType::Type::INT});
  addScalarIntrinsic(&intrinsics,
                     ir::intrinsics::sin().getName(),
                     {ScalarType::Type::FLOAT},
                     {ScalarType::Type::FLOAT});
  addScalarIntrinsic(&intrinsics,
                     ir::intrinsics::cos().getName(),
                     {ScalarType::Type::FLOAT},
                     {ScalarType::Type::FLOAT});
  addScalarIntrinsic(&intrinsics,
                     ir::intrinsics::tan().getName(),
                     {ScalarType::Type::FLOAT},
                     {ScalarType::Type::FLOAT});
  addScalarIntrinsic(&intrinsics,
                     ir::intrinsics::asin().getName(),
                     {ScalarType::Type::FLOAT},
                     {ScalarType::Type::FLOAT});
  addScalarIntrinsic(&intrinsics,
                     ir::intrinsics::acos().getName(),
                     {ScalarType::Type::FLOAT},
                     {ScalarType::Type::FLOAT});
  addScalarIntrinsic(&intrinsics,
                     ir::intrinsics::atan2().getName(),
                     {ScalarType::Type::FLOAT, ScalarType::Type::FLOAT},
                     {ScalarType::Type::FLOAT});
  addScalarIntrinsic(&intrinsics,
                     ir::intrinsics::sqrt().getName(),
                     {ScalarType::Type::FLOAT},
                     {ScalarType::Type::FLOAT});
  addScalarIntrinsic(&intrinsics,
                     ir::intrinsics::cbrt().getName(),
                     {ScalarType::Type::FLOAT},
                     {ScalarType::Type::FLOAT});
  addScalarIntrinsic(&intrinsics,
                     ir::intrinsics::log().getName(),
                     {ScalarType::Type::FLOAT},
                     {ScalarType::Type::FLOAT});
  addScalarIntrinsic(&intrinsics,
                     ir::intrinsics::exp().getName(),
                     {ScalarType::Type::FLOAT},
                     {ScalarType::Type::FLOAT});
  addScalarIntrinsic(&intrinsics,
                     ir::intrinsics::pow().getName(),
                     {ScalarType::Type::FLOAT, ScalarType::Type::FLOAT},
                     {ScalarType::Type::FLOAT});

  addScalarIntrinsic(&intrinsics,
                     ir::intrinsics::clock().getName(),
                     {}, {ScalarType::Type::FLOAT});
  addScalarIntrinsic(&intrinsics,
                     ir::intrinsics::storeTime().getName(),
                     {ScalarType::Type::INT, ScalarType::Type::FLOAT},
                     {ScalarType::Type::FLOAT});

  // Local vectors/matrices
  addIntrinsic(&intrinsics,
               ir::intrinsics::det().getName(),
               {threeByThreeTensorType},
               {makeTensorType(ScalarType::Type::FLOAT)});
  addIntrinsic(&intrinsics,
               ir::intrinsics::inv().getName(),
               {threeByThreeTensorType},
               {threeByThreeTensorType});

  // Local vector/vector
  addIntrinsic(&intrinsics,
               ir::intrinsics::cross().getName(),
               {threeVectorType, threeVectorType},
               {threeVectorType});

  // System vectors/matrices
  addIntrinsic(&intrinsics,
               ir::intrinsics::norm().getName(),
               {Type::Ptr()},
               {makeTensorType(ScalarType::Type::FLOAT)});
  addIntrinsic(&intrinsics,
               ir::intrinsics::dot().getName(),
               {Type::Ptr(), Type::Ptr()},
               {makeTensorType(ScalarType::Type::FLOAT)});
  addIntrinsic(&intrinsics,
               ir::intrinsics::lu().getName(),
               {nnMatrixType},
               {opaqueType},
               {N});
  addIntrinsic(&intrinsics,
               ir::intrinsics::lufree().getName(),
               {opaqueType},
               {},
               {});
  addIntrinsic(&intrinsics,
               ir::intrinsics::lusolve().getName(),
               {opaqueType, nVectorType},
               {mVectorType},
               {N, M});
  addIntrinsic(&intrinsics,
               ir::intrinsics::lumatsolve().getName(),
               {opaqueType, nmMatrixType},
               {kmMatrixType},
               {N, M, K});
  addIntrinsic(&intrinsics,
               ir::intrinsics::chol().getName(),
               {nnMatrixType},
               {opaqueType},
               {N});
  addIntrinsic(&intrinsics,
               ir::intrinsics::cholfree().getName(),
               {opaqueType},
               {},
               {});
  addIntrinsic(&intrinsics,
               ir::intrinsics::lltsolve().getName(),
               {opaqueType, nVectorType},
               {mVectorType},
               {N,M});
  addIntrinsic(&intrinsics,
               ir::intrinsics::lltmatsolve().getName(),
               {opaqueType, nmMatrixType},
               {kmMatrixType},
               {N, M, K});

  // Complex numbers
  addScalarIntrinsic(&intrinsics,
                     ir::intrinsics::createComplex().getName(),
                     {ScalarType::Type::FLOAT, ScalarType::Type::FLOAT},
                     {ScalarType::Type::COMPLEX});
  addScalarIntrinsic(&intrinsics,
                     ir::intrinsics::complexNorm().getName(),
                     {ScalarType::Type::COMPLEX},
                     {ScalarType::Type::FLOAT});
  addScalarIntrinsic(&intrinsics,
                     ir::intrinsics::complexGetReal().getName(),
                     {ScalarType::Type::COMPLEX},
                     {ScalarType::Type::FLOAT});
  addScalarIntrinsic(&intrinsics,
                     ir::intrinsics::complexGetImag().getName(),
                     {ScalarType::Type::COMPLEX},
                     {ScalarType::Type::FLOAT});
  addScalarIntrinsic(&intrinsics,
                     ir::intrinsics::complexConj().getName(),
                     {ScalarType::Type::COMPLEX},
                     {ScalarType::Type::COMPLEX});

  return intrinsics;
}

}}
