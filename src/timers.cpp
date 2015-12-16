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
namespace ir {

vector<string> sourceLines;
vector<string>& getSourceLines() {
  return sourceLines;
}

void addSourceLines(stringstream& ss) {
  for (string line; getline(ss, line); sourceLines.push_back(line));
}

vector<string> timedLines;
void addTimedLine(string line) {
  timedLines.push_back(line);
}

int getTimedLineIndex(string line) {
  int pos = find(timedLines.begin(), timedLines.end(), line.c_str()) - timedLines.begin();
  if (pos >= timedLines.size()){
    pos = -1;
  }
  return pos;
}

void printTimedLines() {
  for (auto& line : timedLines) {
    cout << line << endl;
  }
}

vector<double> timerSums;
vector<unsigned long long int> timerCount;
void storeTime(int index, double time) {
  while ( timerCount.size() < index + 1) {
    timerCount.push_back(0);
    timerSums.push_back(0);
  }
  timerCount[index] += 1;
  timerSums[index] += time;
}

double getTime(int index) {
  return timerSums[index];
}

unsigned long long int getCounter(int index) {
  return timerCount[index];
}

double getTotalTime() {
  double sum = 0;
  for(auto const &time : timerSums) {
    sum += time;
  }
  return sum;
}

double getTimingPercentage(int index) {
  return getTime(index) * 100.0 / getTotalTime(); 
}

class InsertTimers : public IRRewriter {
  using IRRewriter::visit;
  public: 
    static InsertTimers& getInstance() {
            static InsertTimers instance; // Guaranteed to be destroyed.
                                  // Instantiated on first use.
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
  private:
    int counter = 0;
    InsertTimers() {};
    InsertTimers(InsertTimers const&)    = delete;
    void operator=(InsertTimers const&)  = delete;

    Var initTimer(string line, Stmt& stmt) {
      addTimedLine(line);
      Var timeStartVar = Var("simit_internal_time_start_" + simit::util::toString(counter), Float);
      Expr timeStart = Call::make(intrinsics::simitClock(), {});
      Stmt timeStartStmt = AssignStmt::make(timeStartVar, timeStart);
      stmt = Block::make(timeStartStmt, stmt);
      return timeStartVar;
    }

    void storeTimer(Stmt& stmt, Var& timeStartVar) {
      Var time = Var("simit_internal_time_temp_" + simit::util::toString(counter), Float);
      Expr subtraction = Sub::make(Call::make(intrinsics::simitClock(), {}), VarExpr::make(timeStartVar));
      Expr store = Call::make(intrinsics::simitStoreTime(), {counter, subtraction});
      Stmt storeStmt = AssignStmt::make(time, store);
      stmt = Block::make(stmt, storeStmt);
      counter++;
      
      // Expr subtraction = Sub::make(Call::make(intrinsics::simitClock(), {}), VarExpr::make(timeStartVar));
      // Stmt store = CallStmt::make(intrinsics::simitStoreTime(), {counter, subtraction});
      // stmt = Block::make(stmt, store);
      // counter++;
    }
};

Func insertTimers(Func func) {
  func = InsertTimers::getInstance().rewrite(func);
  func = insertVarDecls(func);
  return func;
}

}}
