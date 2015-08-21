#include <iostream>

#include "ir.h"
#include "ir_visitor.h"
#include "ir_printer.h"
#include "ir_rewriter.h"
#include "lower/lower.h"
#include "temps.h"
#include "flatten.h"
#include "frontend.h"
#include "program_context.h"
#include "error.h"
#include "util.h"
#include "storage.h"

#include "backend/backend_function.h"
#include "backend/llvm/llvm_backend.h"
#ifdef GPU
#include "backend/gpu/gpu_backend.h"
#endif

using namespace std;

void printUsage(); // GCC shut up

void printUsage() {
  cerr << "Usage: simit-dump [options] <simit-source> " << endl << endl
       << "Options:"            << endl
       << "-emit-simit"         << endl
       << "-emit-llvm"          << endl
       << "-emit-asm"           << endl
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
  bool emitASM = false;
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
        else if (arg == "-emit-asm") {
          emitASM = true;
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

    if (emitSimit) {
      cout << endl << endl;
      cout << "--- Compile " << function << endl;
    }

    // Call lower with print=emitSimit
    func = lower(func, emitSimit);

    // Emit and print llvm code
    // NB: The LLVM code gets further optimized at init time (OSR, etc.)

    if (emitLLVM || emitASM) {
      simit::backend::LLVMBackend backend;
      simit::Function llvmFunc(backend.compile(func));

      if (emitLLVM) {
        std::string fstr = simit::util::toString(llvmFunc);
        if (emitSimit) {
          cout << "--- Emitting LLVM" << endl;
        }
        cout << simit::util::trim(fstr) << endl;
      }

      if (emitASM) {
        cout << "--- Emitting Assembly" << endl;
        llvmFunc.printMachine(cout);
      }
    }
  }

  return 0;
}
