#include "timers.h"

#include "storage.h"
#include "ir_builder.h"
#include "ir_rewriter.h"
#include "ir_transforms.h"
#include "inline.h"

#include "ir.h"
#include "intrinsics.h"

using namespace std;

namespace simit {

void printTimes() {
  const int LINE_LIMIT = 80;
  double percentageSum = 0.0;
  
  for (auto line: simit::ir::TimerStorage::getInstance().getSourceLines()) {
    size_t first = line.find_first_not_of(' ');
    size_t last = line.find_last_not_of('\n');
    std::string test = "";
    if ( first != last ) {
      test = line.substr(first, (last-first+1));
      line = line.substr(0, (last+1));
    }
    int index = simit::ir::TimerStorage::getInstance().getTimedLineIndex(test);
    if ( index >= 0) {
      double percentage = 
          simit::ir::TimerStorage::getInstance().getTimingPercentage(index);
      percentageSum += percentage;
      unsigned long long int timerCount =
          simit::ir::TimerStorage::getInstance().getCounter(index);
      if (line.length() < LINE_LIMIT) {
        line.append(LINE_LIMIT - line.length(), ' '); 
        printf("%s (%f%s, %llu)\n", line.c_str(), percentage , "%", timerCount);
      } else {
        printf("%s (%f%s, %llu)\n", line.substr(0,LINE_LIMIT).c_str(),
               percentage, "%", timerCount);
        for (unsigned x=LINE_LIMIT; x < line.length(); x+= LINE_LIMIT) {
          printf("\t %s\n",line.substr(x, LINE_LIMIT).c_str());
        }
      }
    } else {
      if (line.length() < LINE_LIMIT) {
        line.append(LINE_LIMIT - line.length(), ' '); 
        printf("%s\n", line.c_str());
      } else {
        printf("%s\n", line.substr(0,LINE_LIMIT).c_str());
        for (unsigned x=LINE_LIMIT; x < line.length(); x+= LINE_LIMIT) {
          printf("\t %s\n",line.substr(x, LINE_LIMIT).c_str());
        }
      }
    }
  }
 
  printf("Total Time: %f (seconds)\n", 
      simit::ir::TimerStorage::getInstance().getTotalTime() / 1000000.0);
}

namespace ir {

// Singleton
class InsertTimers : public IRRewriter {
  using IRRewriter::visit;
  public: 
    static InsertTimers& getInstance() {
      static InsertTimers instance; 
      return instance;
    }
    
    void visit(const TensorWrite *op) {
      Var timeStartVar = initTimer(util::toString(*op),stmt);
      stmt = Block::make(stmt,op);
      storeTimer(stmt, timeStartVar);
    }

    void visit(const FieldWrite *op) {
      Var timeStartVar = initTimer(util::toString(*op),stmt);
      stmt = Block::make(stmt,op);
      storeTimer(stmt, timeStartVar);
    }
    
    void visit(const Map *op) {
      Var timeStartVar = initTimer(util::toString(*op),stmt);
      stmt = Block::make(stmt,op);
      storeTimer(stmt, timeStartVar);
    }
    
    void visit(const Store *op) {
      Var timeStartVar = initTimer(util::toString(*op),stmt);
      stmt = Block::make(stmt,op);
      storeTimer(stmt, timeStartVar);
    }
    
    void visit(const CallStmt *op) {
      if (op->callee.getKind() == Func::Intrinsic) { 
        Var timeStartVar = initTimer(util::toString(*op),stmt);
        stmt = Block::make(stmt,op);
        storeTimer(stmt, timeStartVar);
      } else {
        stmt = Block::make(stmt, op);
      }
    }
    
    void visit(const AssignStmt *op) {
      Var timeStartVar = initTimer(util::toString(*op),stmt);
      stmt = Block::make(stmt,op);
      storeTimer(stmt, timeStartVar);
    }
    
    void visit(const IfThenElse *op) {
      Expr start = rewrite(op->condition);
      Stmt thenBody = rewrite(op->thenBody);
      Stmt elseBody = rewrite(op->elseBody);
      
      stmt = IfThenElse::make(op->condition, thenBody, elseBody);
    }
    
    void visit(const ForRange *op) {
      Expr start = rewrite(op->start);
      Expr end = rewrite(op->end);
      Stmt body = rewrite(op->body);
      
      stmt = ForRange::make(op->var, start, end, body);
    }
    
    void visit(const For *op) {
      Stmt body = rewrite(op->body);
      
      stmt = For::make(op->var, op->domain, body);
    }
  
    Var getTimeVar() {
      return timeStartVar;
    }
  private:
    int counter = 0;
    Var timeStartVar = Var("simit_internal_time_var", Float);
    InsertTimers() {};
    InsertTimers(InsertTimers const&)    = delete;
    void operator=(InsertTimers const&)  = delete;

    Var initTimer(string line, Stmt& stmt) {
      TimerStorage::getInstance().addTimedLine(line);
      Expr timeStart = Call::make(intrinsics::simitClock(), {});
      Stmt timeStartStmt = AssignStmt::make(getTimeVar(), timeStart);
      stmt = Block::make(timeStartStmt, stmt);
      return timeStartVar;
    }

    void storeTimer(Stmt& stmt, Var& timeStartVar) {
      Expr subtraction = Sub::make(Call::make(intrinsics::simitClock(), {}),
          VarExpr::make(timeStartVar));
      Stmt store = CallStmt::make({}, intrinsics::simitStoreTime(), 
          {counter, subtraction});
      stmt = Block::make(stmt, store);
      counter++;
    }
};

Func insertTimers(Func func) {
  Var timeStartVar = InsertTimers::getInstance().getTimeVar();
  Func timerFunc = Func(func, Block::make(VarDecl::make(timeStartVar), 
        func.getBody()));
  timerFunc = InsertTimers::getInstance().rewrite(timerFunc);
  return timerFunc;
}

}}
