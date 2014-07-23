#include "Util.h"

using namespace util;

std::string util::indent(std::string str, unsigned int num) {
  std::istringstream ss(str);
  std::string indent(num, ' ');
  std::string strIndented;
  std::string line;

  if (std::getline(ss, line)) {
    strIndented += indent + line;
  }
  while (std::getline(ss, line)) {
    strIndented += "\n" + indent + line;
  }
  return strIndented;
}
