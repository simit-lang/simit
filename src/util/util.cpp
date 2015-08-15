#include "util.h"

#include <fstream>

namespace simit {
namespace util {

std::string indent(std::string str, unsigned int num) {
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

std::vector<std::string> split(const std::string &str, const std::string &delim,
                               bool keepDelim) {
  std::vector<std::string> results;
  size_t prev = 0;
  size_t next = 0;

  while ((next = str.find(delim, prev)) != std::string::npos) {
    if (next - prev != 0) {
      std::string substr = ((keepDelim) ? delim : "") + str.substr(prev, next-prev);
      results.push_back(substr);
    }
    prev = next + delim.size();
  }

  if (prev < str.size()) {
    std::string substr = ((keepDelim) ? delim : "") + str.substr(prev);
    results.push_back(substr);
  }

  return results;
}

int loadText(const std::string &file, std::string *text) {
  std::ifstream ifs(file);
  if (!ifs.good()) {
    return 1;
  }

  *text = std::string((std::istreambuf_iterator<char>(ifs)),
                      (std::istreambuf_iterator<char>()));
  return 0;
}

std::string trim(const std::string &str, const std::string &ws) {
  const auto strBegin = str.find_first_not_of(ws);
  if (strBegin == std::string::npos)
    return ""; // no content

  const auto strEnd = str.find_last_not_of(ws);
  const auto strRange = strEnd - strBegin + 1;

  return str.substr(strBegin, strRange);
}

}} // namespace simit::util
