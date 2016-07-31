#include "inline.h"

#include <vector>
#include <map>

#include "init.h"
#include "temps.h"
#include "flatten.h"
#include "intrinsics.h"
#include "ir_codegen.h"
#include "lattice_ops.h"
#include "tensor_index.h"
#include "stencils.h"

using namespace std;

namespace simit {
namespace ir {

Stmt inlineMapFunction(const Map *map, Var lv, vector<Var> ivs,
                       MapFunctionRewriter &rewriter, Storage* storage);

Stmt MapFunctionRewriter::inlineMapFunc(const Map *map, Var targetLoopVar,
                                        Storage *storage,
                                        Var endpoints, Var locs,
                                        std::map<vector<int>, Expr> clocs,
                                        vector<Var> latticeIndexVars) {
  this->endpoints = endpoints;
  this->locs = locs;
  this->clocs = clocs;
  this->reduction = map->reduction;
  this->targetLoopVar = targetLoopVar;
  this->latticeIndexVars = latticeIndexVars;
  this->storage = storage;

  Func kernel = map->function;
  // TODO: revise this assert given map functions can have many params
  //iassert(kernel.getArguments().size() == 1 || kernel.getArguments().size() == 2)
  //    << "mapped functions must have exactly two arguments";

  iassert(map->vars.size() == kernel.getResults().size());
  for (size_t i=0; i < kernel.getResults().size(); ++i) {
    resultToMapVar[kernel.getResults()[i]] = map->vars[i];
  }

  this->targetSet = map->target;
  this->neighborSet = map->neighbors;
  this->throughSet = map->through;

  iassert(kernel.getArguments().size() >= 1)
      << "The function must have a target argument";

  auto argIt = kernel.getArguments().begin()+map->partial_actuals.size();
  this->target = *argIt++;

  if (kernel.getArguments().size() >= (2+map->partial_actuals.size())) {
    // Bit hacky: distinguish between neighbors arg and through args
    // Neighbors will be a tuple of elements, through args will be
    // two sets.
    auto maybeNeighbors = *argIt;
    if (maybeNeighbors.getType().isTuple()) {
      this->neighbors = maybeNeighbors;
      argIt++;
    }
  }
  if (this->throughSet.defined()) {
    this->throughEdges = *argIt++;
    // TODO: We assume the lattice link set refers to the point set
    // by the extern variable.
    Expr pointsSet = this->throughEdges.getType().toLatticeLinkSet()
        ->latticePointSet.getSet();
    tassert(isa<VarExpr>(pointsSet))
        << "Lattice link set " << this->throughEdges
        << " must refer to underlying point set via the extern variable";
    this->throughPoints = to<VarExpr>(pointsSet)->var;
  }

  return rewrite(kernel.getBody());
}

bool MapFunctionRewriter::isResult(Var var) {
  return resultToMapVar.find(var) != resultToMapVar.end();
}

Var MapFunctionRewriter::getMapVar(Var resultVar) {
  return resultToMapVar[resultVar];
}

void MapFunctionRewriter::visit(const FieldWrite *op) {
  // Write a field from the target set
  if (isa<VarExpr>(op->elementOrSet) &&
      to<VarExpr>(op->elementOrSet)->var == target) {
    Expr setFieldRead = FieldRead::make(targetSet, op->fieldName);
    stmt = TensorWrite::make(setFieldRead, {targetLoopVar}, rewrite(op->value));
  }
  // Write a field from a neighbor set
  else if(isa<TupleRead>(op->elementOrSet) &&
          isa<VarExpr>(to<TupleRead>(op->elementOrSet)->tuple) &&
          to<VarExpr>(to<TupleRead>(op->elementOrSet)->tuple)->var==neighbors) {
    expr = FieldRead::make(neighborSet, op->fieldName);
    Expr setFieldRead = expr;

    Expr index = IRRewriter::rewrite(op->elementOrSet);
    stmt = TensorWrite::make(setFieldRead, {index}, rewrite(op->value));
  }
  else {
    // TODO: Handle the case where the target var was reassigned
    //       tmp = s; ... = tmp.a;
    not_supported_yet;
  }
}

void MapFunctionRewriter::visit(const FieldRead *op) {
  // Read a field from the target set
  if (isa<VarExpr>(op->elementOrSet) &&
      to<VarExpr>(op->elementOrSet)->var == target) {
    Expr setFieldRead = FieldRead::make(targetSet, op->fieldName);
    expr = TensorRead::make(setFieldRead, {targetLoopVar});
  }
  // Read a field from a neighbor set
  else if(isa<TupleRead>(op->elementOrSet) &&
          isa<VarExpr>(to<TupleRead>(op->elementOrSet)->tuple) &&
          to<VarExpr>(to<TupleRead>(op->elementOrSet)->tuple)->var==neighbors) {
    expr = FieldRead::make(neighborSet, op->fieldName);
    Expr setFieldRead = expr;

    Expr index = IRRewriter::rewrite(op->elementOrSet);
    expr = TensorRead::make(setFieldRead, {index});
  }
  // Read a field from a lattice offset element
  else if (isa<SetRead>(op->elementOrSet) &&
           isa<VarExpr>(to<SetRead>(op->elementOrSet)->set)) {
    const SetRead *sr = to<SetRead>(op->elementOrSet);
    // Lattice offset links
    if (to<VarExpr>(sr->set)->var == throughEdges) {
      Expr setFieldRead = FieldRead::make(throughSet, op->fieldName);
      Expr index = IRRewriter::rewrite(op->elementOrSet);
      expr = TensorRead::make(setFieldRead, {index});
    }
    // Lattice offset points
    else if (to<VarExpr>(sr->set)->var == throughPoints) {
      Expr setFieldRead = FieldRead::make(
          throughSet.type().toLatticeLinkSet()->latticePointSet.getSet(),
          op->fieldName);
      Expr index = IRRewriter::rewrite(op->elementOrSet);
      expr = TensorRead::make(setFieldRead, {index});
    }
    else {
      not_supported_yet;
    }
  }
  else {
    // TODO: Handle the case where the target var was reassigned
    //       tmp = s; ... = tmp.a;
    not_supported_yet;
  }
}

void MapFunctionRewriter::visit(const TupleRead *op) {
  iassert(isa<VarExpr>(op->tuple))
      << "This code assumes no expressions return a tuple";

  if (to<VarExpr>(op->tuple)->var == neighbors) {
    const TupleType *tupleType = op->tuple.type().toTuple();
    int cardinality = tupleType->size;

    Expr endpoints = IndexRead::make(targetSet, IndexRead::Endpoints);
    Expr indexExpr = Add::make(Mul::make(targetLoopVar, cardinality),
                               op->index);
    expr = Load::make(endpoints, indexExpr);
  }
  else {
    ierror << "Assumes tuples are only used for neighbor lists";
  }
}

void MapFunctionRewriter::visit(const SetRead *op) {
  iassert(isa<VarExpr>(op->set)) << "Set read set must be a variable";
  const Var& setVar = to<VarExpr>(op->set)->var;
  unsigned dims = throughEdges.getType().toLatticeLinkSet()->dimensions;
  if (setVar == throughEdges) {
    iassert(op->indices.size() == dims*2);
    iassert(latticeIndexVars.size() == dims);
    // Index into link set assuming canonical ordering
    vector<int> offsets = getOffsets(op->indices);
    vector<int> srcOff, sinkOff;
    int dir = -1;
    bool srcBase;
    for (unsigned i = 0; i < dims; ++i) {
      srcOff.push_back(offsets[i]);
      sinkOff.push_back(offsets[dims+i]);
      if (srcOff.back() != sinkOff.back()) {
        iassert(dir == -1)
            << "Cannot have multiple offsets in relative lattice indexing";
        iassert(abs(srcOff.back() - sinkOff.back()) == 1)
            << "Cannot offset by more than 1 in lattice link indexing";
        dir = i;
        srcBase = (srcOff.back() < sinkOff.back());
      }
    }
    iassert(dir != -1) << "Must have an offset in lattice link indexing";
    
    // Convert index offsets to a single offset expr
    vector<Expr> indices, base;
    indices.push_back(dir); // Directional index innermost
    for (int ind : (srcBase ? srcOff : sinkOff)) {
      indices.push_back(ind);
    }
    base.push_back(Expr(0)); // Add directional base 0 value
    for (const Var& v : latticeIndexVars) {
      base.push_back(v);
    }
    iassert(indices.size() == dims+1);
    iassert(base.size() == dims+1);
    
    vector<Expr> finalIndices = getLatticeLinkOffsetIndices(
        base, indices, throughSet);
    expr = getLatticeLinkCoord(finalIndices, throughSet);
  }
  else if (setVar == throughPoints) {
    vector<Expr> indices, base;
    for (int ind : getOffsets(op->indices)) {
      indices.emplace_back(ind);
    }
    for (const Var& v : latticeIndexVars) {
      base.emplace_back(v);
    }
    iassert(indices.size() == dims);
    iassert(base.size() == dims);
    
    vector<Expr> finalIndices = getLatticeOffsetIndices(
        base, indices, throughSet);
    expr = getLatticeCoord(finalIndices, throughSet);
  }
  else {
    not_supported_yet;
  }
}

void MapFunctionRewriter::visit(const VarExpr *op) {
  if (op->var == target) {
    expr = targetLoopVar;
  }
  else if (isResult(op->var)) {
    expr = resultToMapVar[op->var];
  }
  else {
    expr = op;
  }
}

StencilContent* buildStencilLocs(Func kernel, Var stencilVar, Var loopVar,
                                 Var latticeSet,
                                 std::map<vector<int>, Expr> &clocs) {
  StencilContent *content = buildStencil(kernel, stencilVar, latticeSet);
  // Build clocs relative to loop var
  for (auto &kv : content->layout) {
    clocs[kv.first] = loopVar*Expr((int)content->layout.size()) + kv.second;
  }
  return content;
}

/// Inlines the mapped function with respect to the given loop variable over
/// the target set, using the given rewriter.
Stmt inlineMapFunction(const Map *map, Var lv, vector<Var> ivs,
                       MapFunctionRewriter &rewriter, Storage* storage) {
  // Compute locations of the mapped edge
  bool returnsMatrix = false;
  for (auto& result : map->function.getResults()) {
    Type type = result.getType();
    if (type.isTensor() && type.toTensor()->order() == 2) {
      returnsMatrix = true;
      break;
    }
  }

  Expr target = map->target;
  iassert(map->target.type().isSet());
  int cardinality = map->target.type().toUnstructuredSet()->endpointSets.size();
  // Map over edge set to build matrix
  if (returnsMatrix && cardinality > 0) {
    iassert(ivs.size() == 0);
    // Computes the return matrix locations of the endpoints of the edge being
    // assembled.  These are stored in an array and used when storing values to
    // the return matrix.
    /* Build IR for locs and eps:
       var i : int;
       var j : int;
       var eps : tensor[card](int);
       for i in 0:card
         eps(i) = target.endpoints[lv*card + i];
       end;

       var locs : tensor[card,card](int);
       for i in 0:card
         for j in 0:card
           locs(i,j) = loc(eps(i),eps(j),target.nbrs_start,target.nbrs);
         end
       end
     */

    Var i("i", Int);
    Var j("j", Int);

    Var eps(INTERNAL_PREFIX("eps"), TensorType::make(ScalarType::Int,
                                                     {IndexDomain(cardinality)}));
    Expr endpoints = IndexRead::make(target, IndexRead::Endpoints);
    Expr epLoc = Add::make(Mul::make(lv, cardinality), i);
    Expr ep = Load::make(endpoints, epLoc);
    Stmt epsInit = TensorWrite::make(eps, {i}, ep);
    Stmt epsInitLoop = ForRange::make(i, 0, cardinality, epsInit);

    Var locs(INTERNAL_PREFIX("locs"), TensorType::make(ScalarType::Int,
                                                       {IndexDomain(cardinality),
                                                        IndexDomain(cardinality)}));

    Expr nbrs_start = IndexRead::make(target, IndexRead::NeighborsStart);
    Expr nbrs = IndexRead::make(target, IndexRead::Neighbors);

    Var locVar(INTERNAL_PREFIX("locVar"), Int);
    Stmt locStmt = CallStmt::make({locVar}, intrinsics::loc(),
                                  {Load::make(eps,i),
                                   Load::make(eps,j),
                                   nbrs_start, nbrs});

    Stmt locsInit = Block::make({locStmt,
                                TensorWrite::make(locs, {i,j}, locVar)});

    Stmt locsInitLoop = ForRange::make(j, 0, cardinality, locsInit);
    locsInitLoop      = ForRange::make(i, 0, cardinality, locsInitLoop);

    Stmt computeLocs = Block::make({VarDecl::make(eps),
                                    epsInitLoop,
                                    VarDecl::make(locs),
                                    locsInitLoop});

    return Block::make(computeLocs,
                       rewriter.inlineMapFunc(map, lv, storage, eps, locs));
  }
  // Map through local coordinate structure to build matrix
  // TODO: This branching should be handled in a less ad-hoc manner
  else if (returnsMatrix && map->through.defined()) {
    // If we're assemblying using local coordinate structure, we can build
    // locs at compile time (clocs) and use this in lowering the map to generate
    // the proper indices.
    std::map<vector<int>, Expr> clocs;
    Var stencilVar, mapVar;
    iassert(map->vars.size() == map->function.getResults().size());
    for (unsigned i = 0; i < map->vars.size(); ++i) {
      auto var = map->vars[i];
      auto res = map->function.getResults()[i];
      if (storage->getStorage(var).getKind() == TensorStorage::Kind::Stencil) {
        iassert(!stencilVar.defined());
        iassert(storage->getStorage(var).getTensorIndex().getStencilLayout()
                .getStencilFunc() == map->function.getName());
        iassert(storage->getStorage(var).getTensorIndex().getStencilLayout()
                .getStencilVar() == var.getName());
        mapVar = var;
        stencilVar = res;
      }
      else if (storage->getStorage(var).getKind()
               == TensorStorage::Kind::Indexed) {
        iassert(!stencilVar.defined());
        mapVar = var;
        stencilVar = res;
      }
    }
    // Must have exactly one stencil-assembled output
    iassert(stencilVar.defined())
        << "map with through must assemble exactly one stencil-assembled var";
    // Build compile-time locs
    StencilLayout s = buildStencilLocs(
        map->function, stencilVar, lv, to<VarExpr>(map->through)->var, clocs);
    if (kIndexlessStencils) {
      // Resolve StencilLayout in storage
      storage->getStorage(mapVar).getTensorIndex().setStencilLayout(s);
    }
    iassert(ivs.size() > 0);
    return rewriter.inlineMapFunc(map, lv, storage, Var(), Var(), clocs, ivs);
  }
  else {
    return rewriter.inlineMapFunc(map, lv, storage);
  }
}

Stmt inlineMap(const Map *map, MapFunctionRewriter &rewriter,
               Storage* storage) {
  Func kernel = map->function;
  kernel = insertTemporaries(kernel);

  // The function must have at least one argument, namely the target. It may
  // also have a neighbor set, as well as other arguments.
  iassert(kernel.getArguments().size() >= 1)
      << "The function must have a target argument";
  
  Var targetVar = kernel.getArguments()[map->partial_actuals.size()];
  
  Var loopVar(targetVar.getName(), Int);
  int ndims = map->through.defined() ?
      map->through.type().toLatticeLinkSet()->dimensions : 0;
  vector<Var> latticeIndexVars;
  for (int i = 0; i < ndims; ++i) {
    latticeIndexVars.emplace_back(targetVar.getName()+"_d"+to_string(i), Int);
  }

  Stmt inlinedMapFunc = inlineMapFunction(map, loopVar, latticeIndexVars,
                                          rewriter, storage);

  Stmt inlinedMap;
  auto initializers = vector<Stmt>();
  for (size_t i=0; i<map->partial_actuals.size(); i++) {
    Var tvar = kernel.getArguments()[i];
    Expr rval = map->partial_actuals[i];
    initializers.push_back(AssignStmt::make(tvar, rval));
  }

  Stmt loop;
  if (!map->through.defined()) {
    iassert(latticeIndexVars.size() == 0);
    ForDomain domain(map->target);
    loop = For::make(loopVar, domain, inlinedMapFunc);
  }
  else {
    iassert(map->through.type().isLatticeLinkSet());
    initializers.push_back(AssignStmt::make(loopVar, 0));
    int dims = map->through.type().toLatticeLinkSet()->dimensions;
    loop = Block::make(inlinedMapFunc, AssignStmt::make(
        loopVar, 1, CompoundOperator::Add));
    for (int i = 0; i < dims; ++i) {
      loop = ForRange::make(latticeIndexVars[i], 0, IndexRead::make(
          map->through, IndexRead::LatticeDim, i), loop);
    }
  }
  
  if (initializers.size() > 0) {
    auto initializersBlock = Block::make(initializers);
    inlinedMap = Block::make(initializersBlock, loop);
  }
  else {
    inlinedMap = loop;
  }
  
  if (map->reduction.getKind() != ReductionOperator::Undefined) {
    for (auto &var : map->vars) {
      iassert(var.getType().isTensor());
      Stmt init = AssignStmt::make(var, var);
      init = initializeLhsToZero(init);
      inlinedMap = Block::make(init, inlinedMap);
    }
  }

  return inlinedMap;
}

}}
