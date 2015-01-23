#include "function.h"

#include <iostream>
using namespace std;

#include "graph.h"
#include "types.h"
#include "ir.h"
#include "ir_visitor.h"
#include "types.h"
#include "indices.h"

using namespace simit;

namespace simit {
namespace internal {

Function::Function(const simit::ir::Func &simitFunc)
    : funcPtr(NULL), initRequired(true) {
  for (auto &arg : simitFunc.getArguments()) {
    formals.push_back(arg.getName());
    actuals[arg.getName()] = Actual(arg.getType());
  }
  for (auto &res : simitFunc.getResults()) {
    // Skip results that alias an argument
    if (actuals.find(res.getName()) != actuals.end()) {
      actuals[res.getName()].setOutput(true);
      continue;
    }
    formals.push_back(res.getName());
    actuals[res.getName()] = Actual(res.getType(), true);
  }

  // Gather the Simit literal expressions and store them in an array in the
  // function, to prevent their memory from being reclaimed if the IR is
  // deleted. This is necessary because compiled functions are expected to
  // retrieve these values when being run.
  class GatherLiteralsVisitor : private simit::ir::IRVisitor {
  public:
    vector<simit::ir::Expr> gather(simit::ir::Func func) {
      literals.clear();
      for (auto &global : func.getEnvironment().globals) {
        global.second.accept(this);
      }

      func.accept(this);
      return literals;
    }
  private:
    vector<simit::ir::Expr> literals;
    using simit::ir::IRVisitor::visit;
    void visit(const simit::ir::Literal *op) {
      literals.push_back(op);
    }
  };
  literals = GatherLiteralsVisitor().gather(simitFunc);
}

Function::~Function() {
}

void Function::bind(const std::string &argName, ir::Expr *tensor) {
  uassert(actuals.find(argName) != actuals.end())
      << "no argument of this name in function";

  // Check that the tensor matches the argument type
  uassert(tensor->type() == actuals[argName].getType())
      << "tensor type " << tensor->type()
      << "does not match function argument type" << actuals[argName].getType();

  uassert(ir::to<ir::Literal>(*tensor) != nullptr);

  actuals[argName].bind(tensor);
  initRequired = true;
}

void Function::bind(const std::string &argName, Set *set) {
  uassert(actuals.find(argName) != actuals.end())
      << "No argument of this name in function";

  // Check that the set matches the argument type
  ir::Type argType = actuals[argName].getType();
  uassert(argType.isSet()) << "Argument is not a set";
  const ir::SetType *argSetType = argType.toSet();
//  auto &argFieldsMap = argSetType->elementType.toElement()->fields;
  const ir::ElementType *elemType = argSetType->elementType.toElement();

  // Type check
  for (size_t i=0; i < set->fields.size(); ++i) {
    Set::FieldData *fieldData = set->fields[i];
    uassert(elemType->hasField(fieldData->name)) << "Field " <<
      fieldData->name << " not found in set";

    const Set::FieldData::TensorType *setFieldType = fieldData->type;
    const ir::TensorType *elemFieldType =
        elemType->field(fieldData->name).type.toTensor();

    ir::ScalarType setFieldTypeComponentType;
    switch (setFieldType->getComponentType()) {
      case ComponentType::INT:
        setFieldTypeComponentType = ir::ScalarType(ir::ScalarType::Int);
        break;
      case ComponentType::FLOAT:
        setFieldTypeComponentType = ir::ScalarType(ir::ScalarType::Float);
        break;
      case ComponentType::BOOLEAN:
        setFieldTypeComponentType = ir::ScalarType(ir::ScalarType::Boolean);
        break;
    }

    uassert(setFieldTypeComponentType == elemFieldType->componentType)
        << "field type"
        << "does not match function argument type" << *elemFieldType;

    uassert(setFieldType->getOrder() == elemFieldType->order())
        << "field type"
        << "does not match function argument type" << *elemFieldType;

    const vector<ir::IndexDomain> &argFieldTypeDims = elemFieldType->dimensions;
    for (size_t i=0; i < elemFieldType->order(); ++i) {
      uassert(argFieldTypeDims[i].getIndexSets().size() == 1)
          << "field type"
          << "does not match function argument type" << *elemFieldType;

      size_t argFieldRange = argFieldTypeDims[i].getIndexSets()[0].getSize();

      uassert(setFieldType->getDimension(i) == argFieldRange)
          << "field type"
          << "does not match function argument type" << *elemFieldType;
    }
  }

  actuals[argName].bind(set);
  // All sets are externs, and should be considered outputs
  actuals[argName].setOutput(true);
  initRequired = true;
}

static int size(const ir::IndexSet &indexSet,
                const std::map<std::string, Set*> &sets) {
  switch (indexSet.getKind()) {
    case ir::IndexSet::Range:
      return indexSet.getSize();
    case ir::IndexSet::Set: {
      ir::Expr set = indexSet.getSet();
      iassert(ir::isa<ir::VarExpr>(set))
          << "Attempting to get the size of a runtime dynamic set: " << set;

      std::string varName = ir::to<ir::VarExpr>(set)->var.getName();
      iassert(sets.find(varName) != sets.end()) << "set not found in function";

      return sets.at(varName)->getSize();
    }
    case ir::IndexSet::Single:
    case ir::IndexSet::Dynamic:
      not_supported_yet;
      return 0;
  }
}

static int size(const ir::IndexDomain &dimension,
                const std::map<std::string, Set*> &sets) {
  size_t result = 1;
  for (const ir::IndexSet &indexSet : dimension.getIndexSets()) {
    result *= size(indexSet, sets);
  }
  return result;
}

// essentially a runtime version of LLVMBackend::emitComputeLen
size_t Function::size(const ir::TensorType &type,
                      const ir::TensorStorage &storage) const {
  switch (storage.getKind()) {
    case ir::TensorStorage::DenseRowMajor: {
      size_t result = 1;

      map<string,Set*> sets;
      for (pair<string,Actual> actual : actuals) {
        if (actual.second.getType().isSet()) {
          sets[actual.first] = actual.second.getSet();
        }
      }

      for (const ir::IndexDomain &dimension : type.dimensions) {
        result *= simit::internal::size(dimension, sets);
      }
      return result;
    }
    case ir::TensorStorage::SystemReduced: {
      ir::Expr targetSetVar = storage.getSystemTargetSet();
      ir::Expr storageSetVar = storage.getSystemStorageSet();

      string targetSetName = ir::to<ir::VarExpr>(targetSetVar)->var.getName();
      string storageSetName = ir::to<ir::VarExpr>(storageSetVar)->var.getName();

      // compute neighbor index size
      Set *targetSet = const_cast<Actual&>(actuals.at(targetSetName)).getSet();
      const internal::NeighborIndex *neighborIndex = targetSet->getNeighborIndex();
      size_t len = neighborIndex->getSize();

      // compute block size
      ir::Type blockType = type.blockType();
      if (!ir::isScalar(blockType)) {
        // TODO: The following assumes all blocks are dense row major. The right
        //       way to assign a storage order for every block in the tensor
        //       represented by a TensorStorage
        size_t blockSize = size(*blockType.toTensor(), ir::TensorStorage::DenseRowMajor);
        len *= blockSize;
      }
      return len;
    }
    case ir::TensorStorage::SystemDiagonal: {
      iassert(type.dimensions.size() > 0);

      // Just need on outer dimension because diagonal
      ir::IndexSet indexSet = type.outerDimensions()[0];
      size_t len = 1;
      switch (indexSet.getKind()) {
        case ir::IndexSet::Range: {
          len *= indexSet.getSize();
          break;
        }
        case ir::IndexSet::Set: {
          ir::Expr setExpr = indexSet.getSet();
          iassert(ir::isa<ir::VarExpr>(setExpr));
          string setName = ir::to<ir::VarExpr>(setExpr)->var.getName();
          Set *set = const_cast<Actual&>(actuals.at(setName)).getSet();
          len *= set->getSize();
          break;
        }
        case ir::IndexSet::Single: unreachable;
        case ir::IndexSet::Dynamic: not_supported_yet;
        default: unreachable;
      }
      // Block size
      ir::Type blockType = type.blockType();
      if (!ir::isScalar(blockType)) {
        // TODO: The following assumes all blocks are dense row major. The right
        //       way to assign a storage order for every block in the tensor
        //       represented by a TensorStorage
        size_t blockSize = size(*blockType.toTensor(), ir::TensorStorage::DenseRowMajor);
        len *= blockSize;
      }
      return len;
    }
    case ir::TensorStorage::SystemNone:
      return 0;
    case ir::TensorStorage::Undefined:
      unreachable;
      return 0;
  }
}

}}
