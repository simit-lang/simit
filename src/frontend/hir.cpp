#include "hir.h"
#include "token.h"
#include "program_context.h"

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

bool Expr::isReadable() {
  switch (access) {
    case internal::Symbol::Read:
    case internal::Symbol::ReadWrite:
      return true;
    default:
      return false;
  }
}

bool Expr::isWritable() {
  switch (access) {
    case internal::Symbol::Write:
    case internal::Symbol::ReadWrite:
      return true;
    default:
      return false;
  }
}

}
}

