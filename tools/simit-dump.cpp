#include <iostream>

#include "ir.h"
#include "sir.h"
#include "sir_printer.h"
#include "sir_codegen.h"
#include "function.h"
#include "frontend.h"
#include "program_context.h"
#include "llvm_backend.h"
#include "errors.h"
#include "util.h"

using namespace std;

void printUsage() {
  cerr << "Usage: simit-dump [options] <simit-source> " << endl << endl
       << "Options:"              << endl
       << "-emit-simit"           << endl
       << "  -emit-tensor-ir"     << endl
       << "  -emit-set-ir"        << endl
       << "-emit-llvm"            << endl
       << "-section=<section>";
}

int main(int argc, const char* argv[]) {
  if (argc < 2) {
    printUsage();
    return 3;
  }

  bool emitTensorIR = false;
  bool emitSetIR = false;
  bool emitLLVM = false;
  std::string section;
  std::string sourceFile;

  // Parse Arguments
  for (int i=1; i < argc; ++i) {
    std::string arg = argv[i];
    if (arg[0] == '-') {
      std::vector<std::string> keyValPair = simit::util::split(arg, "=");
      if (keyValPair.size() == 1) {
        if (arg == "-emit-simit") {
          emitTensorIR = emitSetIR = true;
        }
        else if (arg == "-emit-tensor-ir") {
          emitTensorIR = true;
        }
        else if (arg == "-emit-set-ir") {
          emitSetIR = true;
        }
        else if (arg == "-emit-llvm") {
          emitLLVM = true;
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
  if (!(emitTensorIR || emitSetIR || emitLLVM)) {
    emitTensorIR = emitSetIR = emitLLVM = true;
  }

  std::string source;
  int status = simit::util::loadText(sourceFile, &source);
  if (status != 0) {
    cerr << "Error opening file" << endl;
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
      cerr << "Could not find section " << section << " in " << sourceFile;
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

  simit::internal::LLVMBackend backend;
  simit::ir::SetIRCodeGen setIRCodeGen;
  for (simit::ir::Function *func : ctx.getFunctions()) {
    bool somethingEmitted = false;

    if (emitTensorIR) {
      cout << *func << endl;
      somethingEmitted = true;
    }

    if (emitSetIR) {
      if (somethingEmitted) {
        cout << endl;
      }
      std::unique_ptr<simit::ir::Stmt> stmt = setIRCodeGen.codegen(func);
      cout << *stmt << endl;
      somethingEmitted = true;
    }

    if (emitLLVM) {
      if (somethingEmitted) {
        cout << endl;
      }
      std::string fstr = simit::util::toString(*backend.compile(func));
      cout << simit::util::trim(fstr) << endl;
    }
  }
  return 0;
}
