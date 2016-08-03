#include <iostream>
#include <fstream>
#include <memory>

#include "ir.h"
#include "ir_visitor.h"
#include "ir_printer.h"
#include "ir_rewriter.h"
#include "lower/lower.h"
#include "temps.h"
#include "flatten.h"
#include "frontend/frontend.h"
#include "program_context.h"
#include "error.h"
#include "util/util.h"
#include "storage.h"

#include "backend/backend.h"
#include "backend/backend_function.h"

using namespace std;
using namespace simit;

static void printUsage() {
  cerr << "Usage: simit-dump [options] <simit-source> " << endl << endl
       << "Options:"            << endl
       << "-emit-all"           << endl
       << "-emit-simit"         << endl
       << "-emit-llvm"          << endl
       << "-emit-asm"           << endl
       << "-files"              << endl
       << "-single-float"       << endl
       << "-compile=<function>" << endl
       << "-section=<section>"  << endl
       << "-gpu";
}
const ios_base::openmode outputMode = ios_base::trunc;

unique_ptr<ostream> irOutputStream(string sourceFile, string suffix="") {
  string filename = (suffix != "") ? sourceFile + ".ir." + suffix : sourceFile;
  return unique_ptr<ostream>(new ofstream(filename, outputMode));
}

int main(int argc, const char* argv[]) {
  if (argc < 2) {
    printUsage();
    return 3;
  }

  bool singleFloat = false;
  bool compile = false;
  bool fileoutput = false;
  bool gpu = false;

  ostream* simitos = nullptr;
  ostream* llvmos  = nullptr;
  ostream* asmos   = nullptr;

  std::unique_ptr<ostream> simitosCleanup;
  std::unique_ptr<ostream> llvmosCleanup;
  std::unique_ptr<ostream> asmosCleanup;

  string section;
  string function;
  string sourceFile;

  // Parse Arguments
  for (int i=1; i < argc; ++i) {
    string arg = argv[i];
    if (arg[0] == '-') {
      std::vector<std::string> keyValPair = simit::util::split(arg, "=");
      if (keyValPair.size() == 1) {
        if (arg == "-emit-all") {
          simitos = &cout;
          llvmos = &cout;
          asmos = &cout;
        }
        else if (arg == "-emit-simit") {
          simitos = &cout;
        }
        else if (arg == "-emit-llvm") {
          llvmos = &cout;
        }
        else if (arg == "-emit-asm") {
          asmos = &cout;
        }
        else if (arg == "-single-float") {
          singleFloat = true;
        }
        else if (arg == "-compile") {
          compile = true;
        }
        else if (arg == "-files") {
          fileoutput = true;
        }
        else if (arg == "-gpu") {
          gpu = true;
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
  if (sourceFile == "") {
    printUsage();
    return 3;
  }

  if (fileoutput) {
    if (simitos) {
      simitosCleanup = irOutputStream(sourceFile, "sim");
      simitos = simitosCleanup.get();
    }
    if (llvmos) {
      llvmosCleanup = irOutputStream(sourceFile, "ll");
      llvmos = llvmosCleanup.get();
    }
    if (asmos) {
      asmosCleanup = irOutputStream(sourceFile, "ll");
      asmos = asmosCleanup.get();
    }
  }

  std::string backend = gpu ? "gpu" : "cpu";
  size_t floatSize = singleFloat ? sizeof(float) : sizeof(double);
  simit::init(backend, floatSize);

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

  if (simitos) {
    for (auto &constant : ctx.getConstants()) {
      *simitos << "const " << constant.first << " = "
              << constant.second << ";" << std::endl;
    }
    if (ctx.getConstants().size() > 0) {
      *simitos << std::endl;
    }

    auto iter = functions.begin();
    while (iter != functions.end()) {
      if (iter->second.getKind() == simit::ir::Func::Internal) {
        *simitos << iter->second << endl;
        ++iter;
        break;
      }
      ++iter;
    }
    while (iter != functions.end()) {
      if (iter->second.getKind() == simit::ir::Func::Internal) {
        *simitos << endl << iter->second << endl;
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

    if (simitos) {
      *simitos << endl << endl;
      *simitos << "% Compile " << function << endl;
    }

    func = lower(func, simitos);

    // Emit and print llvm code
    // NB: The LLVM code gets further optimized at init time (OSR, etc.)
    if (llvmos || asmos) {
      backend::Backend backend("cpu");
      simit::Function  llvmFunc(backend.compile(func));

      if (llvmos) {
        if (!fileoutput && simitos) {
          *llvmos << "--- Emitting LLVM" << endl;
        }
        *llvmos << util::trim(util::toString(llvmFunc)) << endl;
      }

      if (asmos) {
        if (!fileoutput && (simitos || llvmos)) {
          *asmos << "--- Emitting Assembly" << endl;
        }
        llvmFunc.printMachine(*asmos);
      }
    }
    else if (gpu) {
      backend::Backend backend("gpu");
      simit::Function llvmFunc(backend.compile(func));

      if (!fileoutput && simitos) {
        cout << "--- Emitting GPU" << endl;
      }
      cout << util::trim(util::toString(llvmFunc)) << endl;
    }
  }

  return 0;
}
