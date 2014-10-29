#include <iostream>

#include "ir.h"
#include "ir_printer.h"
#include "lower.h"
#include "function.h"
#include "frontend.h"
#include "program_context.h"
#include "llvm_backend.h"
#include "errors.h"
#include "util.h"

using namespace std;

void printUsage() {
  cerr << "Usage: simit-dump [options] <simit-source> " << endl << endl
       << "Options:"            << endl
       << "-emit-simit"         << endl
       << "-emit-llvm"          << endl
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
  bool compile = false;

  std::string section;
  std::string function;
  std::string sourceFile;

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
  if (!(emitSimit || emitLLVM)) {
    emitSimit = emitLLVM = true;
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
        assert(false && "No text in string");
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
  std::vector<simit::Error> errors;
  simit::internal::ProgramContext ctx;

  status = frontend.parseString(source, &ctx, &errors);
  if (status != 0) {
    for (auto &error : errors) {
      cerr << error << endl;
    }
    return 1;
  }

  auto functions = ctx.getFunctions();
  auto iter = functions.begin();

  if (emitSimit) {
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
      if (func == nullptr) {
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

    // Lower while printing lowered results
    if (emitSimit) {
      cout << endl << endl;
      cout << "--- Compiling " << function << " (inlined):" << endl;
      cout << func << endl << endl;
    }

    func = lowerIndexExpressions(func);
    if (emitSimit) {
      cout << "--- Lowering Index Expressions:" << endl;
      cout << func << endl << endl;;
    }

    func = lowerTensorAccesses(func);
    if (emitSimit) {
      cout << "--- Lowering Tensor Reads and Writes:" << endl;
      cout << func << endl;
    }

    if (emitLLVM) {
      simit::internal::LLVMBackend backend;
      std::string fstr = simit::util::toString(*backend.compile(func));
      if (emitSimit) {
        cout << endl << "--- Emitting LLVM:" << endl;
      }
      cout << simit::util::trim(fstr) << endl;

      // NB: The LLVM code gets further optimized at init time (OSR, etc.)
    }
  }

  return 0;
}
