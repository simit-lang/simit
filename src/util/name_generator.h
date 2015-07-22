#ifndef SIMIT_NAME_GENERATOR_H
#define SIMIT_NAME_GENERATOR_H

#include <string>
#include <map>

namespace simit {
namespace util {

class NameGenerator {
public:
  NameGenerator() : NameGenerator("tmp") {}
  NameGenerator(const std::string &defaultName) : defaultName(defaultName) {}

  std::string getName();
  std::string getName(const std::string &suggestion);

private:
  std::map<std::string,unsigned> takenNames;
  std::string defaultName;
};

}}
#endif
