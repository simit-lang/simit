#include <iostream>

#include "ir.h"
#include "ir_printer.h"
#include "ir_rewriter.h"
#include "lower.h"
#include "temps.h"
#include "flatten.h"
#include "function.h"
#include "frontend.h"
#include "program_context.h"
#include "llvm_backend.h"
#include "error.h"
#include "util.h"
#include "storage.h"

#ifdef GPU
#include "gpu_backend/gpu_backend.h"
#endif

using namespace std;

void printUsage(); // GCC shut up

void printUsage() {
  cerr << "Usage: simit-dump [options] <simit-source> " << endl << endl
       << "Options:"            << endl
       << "-emit-simit"         << endl
       << "-emit-llvm"          << endl
       << "-emit-gpu=<file>"    << endl
       << "-compile"            << endl
       << "-compile=<function>" << endl
       << "-section=<section>";
}

int main(int argc, const char* argv[]) {
  if (argc < 2) {
    printUsage();
    return 3;
  }

  bool emitSimit = false;
  bool emitLLVM = false;
  bool emitGPU = false;
  bool compile = false;

  std::string section;
  std::string function;
  std::string sourceFile;
  std::string gpuOutFile;

  // Parse Arguments
  for (int i=1; i < argc; ++i) {
    std::string arg = argv[i];
    if (arg[0] == '-') {
      std::vector<std::string> keyValPair = simit::util::split(arg, "=");
      if (keyValPair.size() == 1) {
        if (arg == "-emit-simit") {
          emitSimit = true;
        }
        else if (arg == "-emit-llvm") {
          emitLLVM = true;
        }
        else if (arg == "-emit-gpu") {
          emitGPU = true;
        }
        else if (arg == "-compile") {
          compile = true;
        }
        else {
          printUsage();
          return 3;
        }
      }
      else if (keyValPair.size() == 2) {
        if (keyValPair[0] == "-section") {
          section = keyValPair[1];
        }
        else if (keyValPair[0] == "-emit-gpu") {
          emitGPU = true;
          gpuOutFile = keyValPair[1];
        }
        else if (keyValPair[0] == "-compile") {
          compile = true;
          function = keyValPair[1];
        }
        else {
          printUsage();
          return 3;
        }
      }
      else {
        printUsage();
        return 3;
      }
    }
    else {
      if (sourceFile != "") {
        printUsage();
        return 3;
      }
      else {
        sourceFile = arg;
      }
    }
  }
  if (sourceFile == "") {
    printUsage();
    return 3;
  }
  if (!(emitSimit || emitLLVM || emitGPU)) {
    emitSimit = emitLLVM = emitGPU = true;
  }
  if (emitGPU && gpuOutFile == "") {
    gpuOutFile = sourceFile + ".out";
  }

  std::string source;
  int status = simit::util::loadText(sourceFile, &source);
  if (status != 0) {
    cerr << "Error: Could not open file " << sourceFile << endl;
    return 2;
  }

  if (section != "") {
    const string sectionSep = "%%%";
    auto sections = simit::util::split(source, sectionSep, true);

    source = "";
    for (auto &sec : sections) {
      std::istringstream ss(sec);
      std::string header;
      if (!std::getline(ss, header)) {
        ierror << "No text in string";
      }
      header = simit::util::trim(header.substr(3, header.size()-1));

      if (section == header) {
        source = ss.str();
      }
    }
    if (source == "") {
      cerr << "Error: Could not find section " << section <<
              " in " << sourceFile;
    }
  }

  simit::internal::Frontend frontend;
  std::vector<simit::ParseError> errors;
  simit::internal::ProgramContext ctx;

  status = frontend.parseString(source, &ctx, &errors);
  if (status != 0) {
    for (auto &error : errors) {
      cerr << error << endl;
    }
    return 1;
  }

  auto functions = ctx.getFunctions();

  if (emitSimit) {
    for (auto &constant : ctx.getConstants()) {
      std::cout << "const " << constant.first << " = "
                << constant.second << ";" << std::endl;
    }
    if (ctx.getConstants().size() > 0) {
      std::cout << std::endl;
    }

    auto iter = functions.begin();
    while (iter != functions.end()) {
      if (iter->second.getKind() == simit::ir::Func::Internal) {
        cout << iter->second << endl;
        ++iter;
        break;
      }
      ++iter;
    }
    while (iter != functions.end()) {
      if (iter->second.getKind() == simit::ir::Func::Internal) {
        cout << endl << iter->second << endl;
      }
      ++iter;
    }
  }

  if (compile) {
    simit::ir::Func func;
    if (function != "") {
      func = functions[function];
      if (!func.defined()) {
        cerr << "Error: Could not find function " << function <<
                " in " << sourceFile;
        return 4;
      }
    }
    else if (functions.size() == 1) {
      func = functions.begin()->second;
    }

    if (!func.defined()) {
      if (compile) {
        cerr << "Error: choose which function to compile using "
             << "-compile=<function>";
        return 5;
      }
    }

    // Lower while printing intermediate results
    if (emitSimit) {
      cout << endl << endl;
      cout << "--- Compile " << function << endl;
    }

    { class Rewriter : public simit::ir::IRRewriterCallGraph {
        using IRRewriter::visit;
        void visit(const simit::ir::Func *op) {
          if (op->getKind() != simit::ir::Func::Internal) {
            func = *op;
            return;
          }
          func = simit::ir::Func(*op, rewrite(op->getBody()));
          func = insertTemporaries(func);
          func = flattenIndexExpressions(func);
        }
      };
      func = Rewriter().rewrite(func);
    }
    if (emitSimit) {
      cout << "--- Insert Temporaries and Flatten Index Expressions" << endl;
      simit::ir::IRPrinterCallGraph(cout).print(func);
      cout << endl << endl << endl;
    }

    {
      class Rewriter : public simit::ir::IRRewriterCallGraph {
        using IRRewriter::visit;
        void visit(const simit::ir::Func *op) {
          if (op->getKind() != simit::ir::Func::Internal) {
            func = *op;
            return;
          }
          func = simit::ir::Func(*op, rewrite(op->getBody()));
          func.setStorage(getStorage(func));
        }
      };
      func = Rewriter().rewrite(func);
    }
    if (emitSimit) {
      cout << "--- Tensor storage" << endl;
      cout << func.getStorage() << endl << endl << endl;
    }
    
    {
      class Rewriter : public simit::ir::IRRewriterCallGraph {
        using IRRewriter::visit;
        void visit(const simit::ir::Func *op) {
          if (op->getKind() != simit::ir::Func::Internal) {
            func = *op;
            return;
          }
          func = simit::ir::Func(*op, rewrite(op->getBody()));
          func = lowerMaps(func);
        }
      };
      func = Rewriter().rewrite(func);
    }
    if (emitSimit) {
      cout << "--- Lower Maps" << endl;
      simit::ir::IRPrinterCallGraph(cout).print(func);
      cout << endl << endl << endl;
    }

    {
      class Rewriter : public simit::ir::IRRewriterCallGraph {
        using IRRewriter::visit;
        void visit(const simit::ir::Func *op) {
          if (op->getKind() != simit::ir::Func::Internal) {
            func = *op;
            return;
          }
          func = simit::ir::Func(*op, rewrite(op->getBody()));
          func = lowerIndexExpressions(func);
        }
      };
      func = Rewriter().rewrite(func);
    }
    if (emitSimit) {
      cout << "--- Lower Index Expressions" << endl;
      simit::ir::IRPrinterCallGraph(cout).print(func);
      cout << endl << endl << endl;
    }

    {
      class Rewriter : public simit::ir::IRRewriterCallGraph {
        using IRRewriter::visit;
        void visit(const simit::ir::Func *op) {
          if (op->getKind() != simit::ir::Func::Internal) {
            func = *op;
            return;
          }
          func = simit::ir::Func(*op, rewrite(op->getBody()));
          func = lowerTensorAccesses(func);
        }
      };
      func = Rewriter().rewrite(func);
    }
    if (emitSimit) {
      cout << "--- Lower Tensor Reads and Writes" << endl;
      simit::ir::IRPrinterCallGraph(cout).print(func);
      cout << endl;
    }


    // Emit and print llvm code
    // NB: The LLVM code gets further optimized at init time (OSR, etc.)
    if (emitLLVM) {
      simit::internal::LLVMBackend backend;
      std::string fstr = simit::util::toString(*backend.compile(func));
      if (emitSimit) {
        cout << endl << "--- Emitting LLVM" << endl;
      }
      cout << simit::util::trim(fstr) << endl;
    }

    cout << "@@@@@@@@@@@@@@@@" << endl;
#ifdef GPU
    cout << "################: " <<emitGPU<< endl;
    func = shardLoops(func);
    cout << "Finished shardLoops" << endl;
    if (emitGPU) {
      cout << "--- Shard loops for GPU" << endl;
      cout << func << endl;
    }
    
    if (emitGPU) {
      simit::internal::GPUBackend backend;
      std::string fstr = simit::util::toString(*backend.compile(func));
      if (emitSimit) {
        cout << endl << "--- Emitting GPU:" << endl;
      }
      cout << simit::util::trim(fstr) << endl;
    }
#endif
  }

  return 0;
}
