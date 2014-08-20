#include "program.h"
#include <iostream>
using namespace std;

int main(int argc, const char* argv[]) {
  if (argc != 2) {
    cerr << "Usage: simit-check <simit-source>" << endl;
    return 3;
  }

  simit::Program program;
  int status = program.loadFile(argv[1]);
  if (status == 2) {
    cerr << "Error opening file" << endl;
    return 2;
  }
  else if (status != 0) {
    cerr << "Error parsing the program" << endl;
    cerr << program.getErrorString() << endl;
    return 1;
  }

  if (program.verify() != 0) {
    cerr << "Error while running test" << endl;
    cerr << program.getErrorString() << endl;
    return 4;
  }

  cout << "Program checks" << endl;
  return 0;
}
