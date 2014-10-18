#include "gtest/gtest.h"

#include <vector>
#include <string>
#include <sstream>
#include <iostream>

#include "types.h"
#include "mesh_loader.h"

using namespace std;
using namespace simit;


TEST(MeshLoader, CubeTest) {
  const string input = R"(    # cube.obj
    #

    g cube

    v  0.0  0.0  0.0 1.0
    v  0.0  0.0  1.0
    v  0.0  1.0  0.0
    v  0.0  1.0  1.0
    v  1.0  0.0  0.0
    v  1.0  0.0  1.0
    v  1.0  1.0  0.0
    v  1.0  1.0  1.0
    v  1.0  1.0  1.0 1.0 1.0

    vn  0.0  0.0  1.0
    vn  0.0  0.0 -1.0
    vn  0.0  1.0  0.0
    vn  0.0 -1.0  0.0
    vn  1.0  0.0  0.0
    vn -1.0  0.0  0.0

    f  1/0/2  7/1/2  5/3/2
    f  1/2  3/2  7/1
    f  1/2/6  4/2/6  3/2/6
    f  1//6  2//6  4//6
    f  1/ 2/ 3/
    f  1/ 2 3//
    f  1     2     3    4
    f  3//3  8//3  7//3
    f  3//3  4//3  8//3
    f  5//5  7//5  8//5
    f  5//5  8//5  6/\
/5
    f  1//4  5//4  6//4
    f  1//4  6//4  2//4
    f  2//1  6//1  8//1
    f  2//1  8//1  4//1 )";

  const string output = R"(v 0 0 0
v 0 0 1
v 0 1 0
v 0 1 1
v 1 0 0
v 1 0 1
v 1 1 0
v 1 1 1
f 1 7 5
f 1 3 7
f 1 4 3
f 1 2 4
f 3 8 7
f 3 4 8
f 5 7 8
f 5 8 6
f 1 5 6
f 1 6 2
f 2 6 8
f 2 8 4
)";

    stringstream inputstream, outputstream;
    inputstream << input;
    Set<> points;
    Set<3> faces(points, points, points);
    loadMesh(inputstream, &points, &faces);
    storeMesh(outputstream, points, faces);
    string outputstring;
    getline(outputstream, outputstring, '\0');
    ASSERT_STREQ(&(output[0]), &(outputstring[0]));
}

TEST(MeshLoader, TriangleTest) {
  const string input = R"(
v 0 0 0
v 0 1 0
v 1 0 0

f 1 2 3)";
  const string output = R"(v 0 0 0
v 0 1 0
v 1 0 0
f 1 2 3
)";
  stringstream inputstream, outputstream;
  inputstream << input;
  Set<> points;
  Set<3> faces(points, points, points);
  loadMesh(inputstream, &points, &faces);
  storeMesh(outputstream, points, faces);
  string outputstring;
  getline(outputstream, outputstring, '\0');
  ASSERT_STREQ(&(output[0]), &(outputstring[0]));
}
