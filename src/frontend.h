#ifndef SIMIT_FRONTEND_H
#define SIMIT_FRONTEND_H

#include <vector>
#include <string>
#include <fstream>

#include "symboltable.h"

namespace simit {
class Error;

namespace internal {
class Function;
class IRNode;
class Test;

/// Provides methods to convert Simit-formated strings and files to Simit IR.
///
/// Strings and files can be parsed using the \ref parseString and
/// \ref parseFile methods and the resulting IR can be retrieved using the
/// \ref getIR method. If the parse methods return an error value information
/// about the errors can be retrieved using the getErrors method.
class Frontend {
 public:
  /// Parses, typechecks and turns a given Simit-formated stream into Simit IR.
  int parseStream(std::istream                      &programStream,
                  std::map<std::string, Function *> *functions,
                  std::vector<Error>                *errors,
                  std::vector<Test*>                *tests);

  /// Parses, typechecks and turns a given Simit-formated string into Simit IR.
  int parseString(const std::string                &programString,
                  std::map<std::string, Function*> *functions,
                  std::vector<Error>               *errors,
                  std::vector<Test*>               *tests);

  /// Parses, typechecks and turns a given Simit-formated file into Simit IR.
  int parseFile(const std::string                 &filename,
                std::map<std::string, Function *> *functions,
                std::vector<Error>                *errors,
                std::vector<Test*>                *tests);

 private:
  SymbolTable<std::shared_ptr<IRNode>> symtable;
  SymbolTable<bool>                    columnVectors;
};

}} // namespace simit::internal

#endif

