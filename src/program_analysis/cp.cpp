#include "program_analysis.h"
#include "intrusive_ptr_hash.h"
#include "ir_pattern_matching.h"
#include "ir_builder.h"
#include "node_replacer.h"
#include "util/collections.h"

#include <cstring>
#include <unordered_map>
#include <unordered_set>

using namespace std;
using namespace simit::util;
using namespace simit::ir;
using namespace simit::ir::program_analysis;

typedef vector<IndexVar> IndexVars;

static bool exprEq(Expr a, Expr b) {
  if (isa<VarExpr>(a) && isa<VarExpr>(b)) {
    const VarExpr* ea = to<VarExpr>(a);
    const VarExpr* eb = to<VarExpr>(b);
    return ea->var == eb->var;
  } else if (isa<Literal>(a) && isa<Literal>(b)) {
    const Literal* ea = to<Literal>(a);
    const Literal* eb = to<Literal>(b);
    return *ea == *eb;
  }
  return false;
}

namespace {
struct IndexVarComp {
  bool operator()(const IndexVar& a, const IndexVar& b) const {
    return a.ptr == b.ptr;
  }
};
}

typedef unordered_map<IndexVar, IndexVar, hash<IndexVar>, IndexVarComp> Mapping;

static bool isSurjective(const IndexVar& domain, const IndexVar& image,
                         Mapping& mapping, Mapping& /*dummy*/) {
  iassert(!domain.isReductionVar());

  if (domain.isFixed()) {
    if (!IndexVarComp()(domain, image)) {
      return false;
    }
  } else {
    auto it = mapping.find(domain);
    if (it != mapping.end()) {
      if (!IndexVarComp()(it->second, image)) {
        return false;
      }
    } else {
      mapping[domain] = image;
    }
  }
  return true;
}

static bool isBijective(const IndexVar& domain, const IndexVar& image,
                        Mapping& mapping, Mapping& reverseMapping) {
  return isSurjective(domain, image, mapping, mapping) &&
         isSurjective(image, domain, reverseMapping, reverseMapping);
}

static bool checkIndicesMapping(const IndexVars& domain, const IndexVars& image,
    bool(*projection)(const IndexVar&, const IndexVar&, Mapping&, Mapping&)) {
  if (domain.size() != image.size()) {
    return false;
  }

  Mapping mapping, reverseMapping;
  for (auto&& pair : ZipConstIterable<IndexVars, IndexVars>(domain, image)) {
    if (!projection(pair.first, pair.second, mapping, reverseMapping)) {
      return false;
    }
  }
  return true;
}

static
bool checkIndicesMapping(const IndexVars& domain1, const IndexVars& domain2,
                         const IndexVars& image1, const IndexVars& image2,
    bool(*projection)(const IndexVar&, const IndexVar&, Mapping&, Mapping&)) {
  if (domain1.size() != image1.size() || domain2.size() != image2.size()) {
    return false;
  }

  Mapping mapping, reverseMapping;
  for (auto&& pair : ZipConstIterable<IndexVars, IndexVars>(domain1, image1)) {
    if (!projection(pair.first, pair.second, mapping, reverseMapping)) {
      return false;
    }
  }
  for (auto&& pair : ZipConstIterable<IndexVars, IndexVars>(domain2, image2)) {
    if (!projection(pair.first, pair.second, mapping, reverseMapping)) {
      return false;
    }
  }

  return true;
}

static bool mapIndices(const IndexVars& domain1, const IndexVars& domain2,
                       const IndexVars& image1, IndexVars& image2,
    bool(*projection)(const IndexVar&, const IndexVar&, Mapping&, Mapping&)) {
  if (domain1.size() != image1.size()) {
    return false;
  }

  Mapping mapping, reverseMapping;
  for (auto&& pair : ZipConstIterable<IndexVars, IndexVars>(domain1, image1)) {
    if (!projection(pair.first, pair.second, mapping, reverseMapping)) {
      return false;
    }
  }

  for (const IndexVar& domain : domain2) {
    iassert(!domain.isReductionVar());
  }

  image2.clear();
  for (const IndexVar& domain : domain2) {
    if (domain.isFixed()) {
      image2.push_back(domain);
    } else {
      IndexVar image;
      auto it = mapping.find(domain);
      if (it != mapping.end()) {
        image = it->second;
      } else {
        image = domain;
      }
      image2.push_back(image);
    }
  }
  return true;
}

namespace {
class UseDefGraph {
  struct Node {
    Node* head;
    vector<Node*> tails;
    Expr value;
    IndexVars indices;

    Node(Expr value, const IndexVars& indices)
      : head(nullptr), value(value), indices(indices) {
      iassert(isa<VarExpr>(value) || isa<Literal>(value));
    }

    bool operator==(const Node& other) const {
      return exprEq(this->value, other.value);
    }
  };

  struct NodeIterator {
    Node* node;

    NodeIterator(Node* node) : node(node) {}

    bool operator!=(const NodeIterator& other) const {
      return node != other.node;
    }

    void operator++() {
      node = node->head;
    }

    Node* operator*() {
      return node;
    }

    NodeIterator begin() {
      return NodeIterator(node);
    }

    NodeIterator end() {
      return NodeIterator(nullptr);
    }
  };

  struct NodeHash {
    size_t operator() (const Node* const& node) const {
      if (isa<VarExpr>(node->value)) {
        const VarExpr* e = to<VarExpr>(node->value);
        return (size_t) e->var.ptr;
      } else {
        iassert(isa<Literal>(node->value));
        return 0;
      }
    }

    bool operator() (const Node* const& a, const Node* const& b) const {
      return a == b || a && b && *a == *b;
    }
  };

  unordered_multiset<Node*, NodeHash, NodeHash> data;
public:
  UseDefGraph() = default;

  UseDefGraph(const UseDefGraph& other) {
    unordered_map<const Node*, Node*> oldNewMap;
    for (const Node* oldNode : other.data) {
      Node* newNode = new Node(oldNode->value, oldNode->indices);
      oldNewMap[oldNode] = newNode;
      data.insert(newNode);
    }

    for (const Node* oldNode : other.data) {
      Node* newNode = oldNewMap[oldNode];
      newNode->head = oldNewMap[oldNode->head];
      if (newNode->head) {
        newNode->head->tails.push_back(newNode);
      }
    }
  }

  ~UseDefGraph() {
    for (Node* node : data) {
      delete node;
    }
  }

  void swap(UseDefGraph& other) {
    data.swap(other.data);
  }

  void insert(Expr destVal, const IndexVars& destIndices,
              Expr srcVal, const IndexVars& srcIndices) {
    // Due to bug in self assignment (A = A'), if dest == src, then dest is
    // invalidated regardless of indices, unless indices exactly match
    iassert(isa<VarExpr>(destVal));

    if (exprEq(destVal, srcVal) && destIndices == srcIndices) {
      return;
    }

    // handle repeated assignment
    Node dest1(destVal, destIndices);
    for (auto it_pair = data.equal_range(&dest1);
         it_pair.first != it_pair.second; ++it_pair.first) {
      Node* destOriginal = *it_pair.first;
      if (checkIndicesMapping(destOriginal->indices, destIndices, isSurjective)) {
        for (Node* srcOriginal : NodeIterator(destOriginal)) {
          if (exprEq(srcVal, srcOriginal->value) &&
              checkIndicesMapping(destOriginal->indices, srcOriginal->indices,
                                  destIndices, srcIndices, isSurjective)) {
            return;
          }
        }
      }
    }

    erase(destVal, destIndices);
    Node src(srcVal, srcIndices);
    Node* headCandidate = nullptr;
    IndexVars mappedIndicesCandidate;

    for (auto it_pair = data.equal_range(&src);
         it_pair.first != it_pair.second; ++it_pair.first) {
      Node* head = *it_pair.first;
      IndexVars mappedIndices;
      if (mapIndices(srcIndices, destIndices,
                     head->indices, mappedIndices, isBijective)) {
        iassert(!headCandidate);
        headCandidate = head;
        mappedIndicesCandidate = mappedIndices;
      }
    }

    Node* dest;
    if (headCandidate) {
      dest = new Node(destVal, mappedIndicesCandidate);
    } else {
      dest = new Node(destVal, destIndices);
      headCandidate = new Node(srcVal, srcIndices);
      data.insert(headCandidate);
    }
    data.insert(dest);
    dest->head = headCandidate;
    headCandidate->tails.push_back(dest);
  }

  void erase(Expr val, const IndexVars& indices = {}) {
    // Erase if var match && index var interfere
    Node node(val, indices);
    for (auto it_pair = data.equal_range(&node);
         it_pair.first != it_pair.second;
         it_pair.first = data.erase(it_pair.first)) {
      const Node* value = *it_pair.first;
      if (Node* head = value->head) {
        head->tails.erase(find(head->tails.begin(), head->tails.end(), value));
        head->tails.insert(head->tails.end(),
                           value->tails.begin(), value->tails.end());
      }
      for (Node* other : value->tails) {
        other->head = value->head;
      }
      delete value;
    }
  }

  bool get(Expr val, const IndexVars& indices,
           Expr& resultVar, IndexVars& resultIndices) {
    Node node(val, indices);
    for (auto it_pair = data.equal_range(&node);
         it_pair.first != it_pair.second; ++it_pair.first) {
      const Node* tail = *it_pair.first;
      const Node* head = tail;
      while (head->head) {
        head = head->head;
      }
      // tail->indices contains more info than indices
      if (!exprEq(val, head->value) &&
          mapIndices(tail->indices, head->indices,
                     indices, resultIndices, isSurjective)) {
        resultVar = head->value;
        return true;
      }
    }
    return false;
  }

  bool merge(const UseDefGraph& other) {
    bool merged = true;

    for (Node* selfDest : data) {
      bool notFound = true;
      for (auto it_pair = other.data.equal_range(selfDest);
           notFound && it_pair.first != it_pair.second; ++it_pair.first) {
        const Node* otherDest = *it_pair.first;
        if (checkIndicesMapping(selfDest->indices, otherDest->indices,
                                isBijective)) {
          if (selfDest->head) {
            NodeIterator selfSrcIterator(selfDest->head);
            for (auto selfSrcBegin = selfSrcIterator.begin(),
                 selfSrcEnd = selfSrcIterator.end();
                 notFound && selfSrcBegin != selfSrcEnd; ++selfSrcBegin) {
              Node* selfSrc = *selfSrcBegin;
              NodeIterator otherSrcIterator(otherDest->head);
              for (auto otherSrcBegin = otherSrcIterator.begin(),
                   otherSrcEnd = otherSrcIterator.end();
                   notFound && otherSrcBegin != otherSrcEnd; ++otherSrcBegin) {
                Node* otherSrc = *otherSrcBegin;
                if (exprEq(selfSrc->value, otherSrc->value) &&
                    checkIndicesMapping(selfDest->indices, selfSrc->indices,
                        otherDest->indices, otherSrc->indices, isBijective)) {
                  notFound = false;
                }
              }
            }
          } else {
            notFound = false;
          }
        }
      }

      if (notFound) {
        merged = false;
        if (Node* selfSrc = selfDest->head) {
          selfSrc->tails.erase(find(selfSrc->tails.begin(),
                                    selfSrc->tails.end(), selfDest));
          selfDest->head = nullptr;
        }
      }
    }

    for (auto selfIt = data.begin(); selfIt != data.end();) {
      Node* selfNode = *selfIt;
      if (!selfNode->head && !selfNode->tails.size()) {
        delete selfNode;
        selfIt = data.erase(selfIt);
        merged = false;
      } else {
        ++selfIt;
      }
    }

    return merged;
  }
};
}

static bool isReduction(const IndexVar& iv) {
  return iv.isReductionVar();
}

static Expr getBuffer(Expr e) {
  struct : public IRVisitor {
    Expr result;

    virtual void visit(const VarExpr* e) {
      result = e;
    }

    virtual void visit(const Load* op) { op->buffer.accept(this); }

    virtual void visit(const FieldRead* op) { op->elementOrSet.accept(this); }

    virtual void visit(const IndexRead* op) { op->edgeSet.accept(this); }

    virtual void visit(const Store* op) { op->buffer.accept(this); }

    virtual void visit(const FieldWrite* op) { op->elementOrSet.accept(this); }

    virtual void visit(const UnnamedTupleRead* op) { op->tuple.accept(this); }

    virtual void visit(const NamedTupleRead* op) { op->tuple.accept(this); }

    virtual void visit(const SetRead* op) { op->set.accept(this); }

    virtual void visit(const TensorRead* op) { op->tensor.accept(this); }

    virtual void visit(const TensorWrite* op) { op->tensor.accept(this); }
  } visitor;
  e.accept(&visitor);
  return visitor.result;
}

Func CSE::rewrite(Func func) {
  struct : public NodeReplacer {
    UseDefGraph state;

    virtual void visit(const VarExpr* op) {
      Expr mappedVar;
      IndexVars mappedIndices;
      if (state.get(op, {}, mappedVar, mappedIndices)) {
        Expr replace = mappedVar;
        if (mappedIndices.size()) {
          replace = IndexedTensor::make(replace, mappedIndices);
        }
        exprReplacement = replace;
      }
    }

    virtual void visit(const IndexedTensor* op) {
      Expr originalVar, mappedVar;
      IndexVars originalIndices, mappedIndices;

      auto&& matchIT = [&](const IndexedTensor* e) {
        originalIndices = e->indexVars;
      };

      auto&& matchVE = [&](const VarExpr* e) {
        originalVar = e;
        return e->type.isTensor();
      };

      if (PatternMatch<IndexedTensor(VarExpr)>::match(
            op, make_tuple(matchIT, make_tuple(matchVE))) &&
          state.get(originalVar, originalIndices, mappedVar, mappedIndices)) {
        exprReplacement = IndexedTensor::make(mappedVar, mappedIndices);
      } else {
        NodeReplacer::visit(op);
      }
    }

    virtual void visit(const AssignStmt *op) {
      NodeReplacer::visit(op);

      Expr src, dest;
      IndexVars srcIndices, destIndices;

      auto&& matchAssign = [&](const AssignStmt* stmt) {
        dest = VarExpr::make(stmt->var);
        return stmt->cop == CompoundOperator::None;
      };

      auto&& matchIE = [&](const IndexExpr* e) {
        destIndices = e->resultVars;
        return none_of(e->resultVars.begin(), e->resultVars.end(), isReduction);
      };

      auto&& matchIT = [&](const IndexedTensor* e) {
        srcIndices = e->indexVars;
        return none_of(e->indexVars.begin(), e->indexVars.end(), isReduction);
      };

      auto&& matchVE = [&](const VarExpr* e) {
        src = e;
        return e->type.isTensor();
      };

      auto&& matchLiteral = [&](const Literal* e) {
        src = e;
        return e->type.isTensor();
      };

      if (PatternMatch<AssignStmt(IndexExpr(IndexedTensor(VarExpr)))>::match
          (op, make_tuple(matchAssign, make_tuple(matchIE, make_tuple(matchIT,
              make_tuple(matchVE))))) ||
          PatternMatch<AssignStmt(IndexExpr(IndexedTensor(Literal)))>::match
          (op, make_tuple(matchAssign, make_tuple(matchIE, make_tuple(matchIT,
              make_tuple(matchLiteral))))) ||
          PatternMatch<AssignStmt(VarExpr)>::match(op, make_tuple(matchAssign,
              make_tuple(matchVE))) ||
          PatternMatch<AssignStmt(Literal)>::match(op, make_tuple(matchAssign,
              make_tuple(matchLiteral)))) {
        if (isa<Literal>(src) && isScalar(src.type()) &&
            dest.type().toTensor()->order() > 0) {
          if (isFixedSizeTensor(dest.type())) {
            // Expand constant scalar to constant tensor
            const Literal* srcLiteral = to<Literal>(src);
            const size_t n = dest.type().toTensor()->size();
            const size_t size = srcLiteral->size;
            uint8_t* value = new uint8_t[n * size];
            for (size_t i = 0; i < n; i++) {
              memcpy(value + i * size, srcLiteral->data, size);
            }
            src = Literal::make(dest.type(), value, n * size);
            delete[] value;

            auto&& indexSets = dest.type().toTensor()->getOuterDimensions();
            IndexVarFactory ivf;
            for (auto&& indexSet : indexSets) {
              IndexVar iv = ivf.createIndexVar(IndexDomain(indexSet));
              srcIndices.push_back(iv);
              destIndices.push_back(iv);
            }
          } else {
            /// Limitation: can't do constant folding for variable sized tensor
            state.erase(dest);
            return;
          }
        }
        state.insert(dest, destIndices, src, srcIndices);
      } else {
        state.erase(dest);
      }
    }

    virtual void visit(const IfThenElse *op) {
      rewrite(op->condition);

      UseDefGraph copyState = state;
      rewrite(op->thenBody);
      state.swap(copyState);
      if (op->elseBody.defined()) {
        rewrite(op->elseBody);
      }
      state.merge(copyState);
    }

    virtual void visit(const ForRange *op) {
      rewrite(op->start);
      rewrite(op->end);

      disable();
      state.erase(op->var);

      bool merged;
      do {
        UseDefGraph copyState = state;
        rewrite(op->body);
        state.erase(op->var);
        merged = copyState.merge(state);
        state.swap(copyState);
      } while (!merged);

      enable();
      UseDefGraph copyState = state;
      rewrite(op->body);
      state.swap(copyState);
    }

    virtual void visit(const For *op) {
      disable();
      state.erase(op->var);

      bool merged;
      do {
        UseDefGraph copyState = state;
        rewrite(op->body);
        state.erase(op->var);
        merged = copyState.merge(state);
        state.swap(copyState);
      } while (!merged);

      enable();
      UseDefGraph copyState = state;
      rewrite(op->body);
      state.swap(copyState);
    }

    virtual void visit(const While *op) {
      disable();

      bool merged;
      do {
        UseDefGraph copyState = state;
        rewrite(op->condition);
        rewrite(op->body);
        merged = copyState.merge(state);
        state.swap(copyState);
      } while (!merged);

      enable();
      rewrite(op->condition);
      UseDefGraph copyState = state;
      rewrite(op->body);
      state.swap(copyState);
    }

    virtual void visit(const CallStmt *op) {
      NodeReplacer::visit(op);
      for (Var v : op->results) {
        state.erase(v);
      }
    }

    virtual void visit(const Store *op) {
      NodeReplacer::visit(op);
      state.erase(getBuffer(op->buffer));
    }

    virtual void visit(const FieldWrite *op) {
      NodeReplacer::visit(op);
      state.erase(getBuffer(op->elementOrSet));
    }

    virtual void visit(const TensorWrite *op) {
      NodeReplacer::visit(op);
      state.erase(getBuffer(op->tensor));
    }

    virtual void visit(const Map *op) {
      NodeReplacer::visit(op);
      for (Var v : op->vars) {
        state.erase(v);
      }
    }
  } visitor;

  return visitor.rewrite(func);
}
