#include "hir.h"
#include "token.h"

namespace simit {
namespace hir {

void HIRNode::setBeginLoc(const internal::Token &token) {
  lineBegin = token.lineBegin;
  colBegin = token.colBegin;
}

void HIRNode::setEndLoc(const internal::Token &token) {
  lineEnd = token.lineEnd;
  colEnd = token.colEnd;
}

void HIRNode::setLoc(const internal::Token &token) {
  setBeginLoc(token);
  setEndLoc(token);
}

void HIRNode::setLoc(HIRNode::Ptr node) {
  lineBegin = node->lineBegin;
  colBegin = node->colBegin;
  lineEnd = node->lineEnd;
  colEnd = node->colEnd;
}

}
}

