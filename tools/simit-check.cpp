#include "Program.h"
using namespace Simit;

#include <iostream>
#include <fstream>
using namespace std;

int main(int argc, const char* argv[]) {
  if (argc != 2) {
    cerr << "Usage: SimitRun <simit-source>" << endl;
    return 1;
  }

  const char *filename = argv[1];
  ifstream simitFile(filename);
  if (!simitFile.good()) {
    cerr << "Error: Could not open file" << endl;
    return 2;
  }

  Program program;
  if (program.loadFile(filename) != 0) {
    cerr << "Error: Could not parse program" << endl;
    cerr << program.errors() << endl;
    return 3;
  }

  cout << "Program checks" << endl;
  return 0;
}
