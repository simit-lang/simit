#include <iostream>

#include "function.h"
#include "frontend.h"
#include "program_context.h"
#include "llvm_backend.h"
#include "errors.h"
#include "util.h"

using namespace std;

void printUsage() {
  cerr << "Usage: simit-dump [options] <simit-source> " << endl << endl;
  cerr << "Options:"               << endl;
  cerr << "--emit-simit"           << endl
       << "--emit-llvm"            << endl
       << "--section=<section>";
}

int main(int argc, const char* argv[]) {
  if (argc < 2) {
    printUsage();
    return 3;
  }

  bool emitSimit = false;
  bool emitLLVM = false;
  std::string section;
  std::string sourceFile;

  // Parse Arguments
  for (int i=1; i < argc; ++i) {
    std::string arg = argv[i];
    if (arg.substr(0,2) == "--") {
      std::vector<std::string> keyValPair = simit::util::split(arg, "=");
      if (keyValPair.size() == 1) {
        if (arg == "--emit-simit") {
          emitSimit = true;
        }
        else if (arg == "--emit-llvm") {
          emitLLVM = true;
        }
        else {
          printUsage();
          return 3;
        }
      }
      else if (keyValPair.size() == 2) {
        if (keyValPair[0] == "--section") {
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
  if (!emitSimit && !emitLLVM) {
    emitSimit = emitLLVM = true;
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
  for (simit::internal::Function *func : ctx.getFunctions()) {
    if (emitSimit) {
      cout << *func << endl;
    }

    if (emitLLVM) {
      if (emitSimit) {
        cout << endl;
      }
      std::string fstr = simit::util::toString(*backend.compile(func));
      cout << simit::util::trim(fstr);
    }
  }
  return 0;
}
