#include "program.h"
using namespace simit;

#include <iostream>
#include <fstream>
using namespace std;

int main(int argc, const char* argv[]) {
  if (argc != 2) {
    cerr << "Usage: SimitRun <simit-source>" << endl;
    return 3;
  }

  Program program;
  const char *filename = argv[1];
  int status = program.loadFile(filename);

  if (status == 2) {
    cerr << "Error: Could not open file" << endl;
    return 2;
  }

  else if (status != 0) {
    cerr << "Error: Could not parse program" << endl;
    cerr << program.getErrorString() << endl;
    return 1;
  }

  cout << "Program checks" << endl;
  return 0;
}
