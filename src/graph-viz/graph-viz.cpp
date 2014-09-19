#include "graph-viz.h"

#include <cassert>
#include <iostream>
#include <string>

#include <GL/gl.h>
#include <GL/glext.h>
#include <GL/glut.h>

using namespace std;

namespace simit {

namespace internal {

const string& kPointsVertexShader = "\
attribute vec4 position;             \
void main() {                        \
  gl_Position = position;            \
}                                    \
";
const string& kPointsFragmentShader = "\
uniform vec4 color;                    \
void main() {                          \
  gl_FragColor = color;                \
}                                      \
";

GLuint createGLProgram(const string& vertexShaderStr,
                       const string& fragmentShaderStr) {
  GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
  GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
  const char* vShaderCstr = vertexShaderStr.c_str();
  const char* fShaderCstr = fragmentShaderStr.c_str();
  glShaderSource(vertexShader, 1, &vShaderCstr, NULL);
  glShaderSource(fragmentShader, 1, &fShaderCstr, NULL);

  // Compile
  GLint success;
  glCompileShader(vertexShader);
  glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
  assert(success &&
         "Vertex shader failed to compile");
  glCompileShader(fragmentShader);
  glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
  assert(success &&
         "Fragment shader failed to compile");

  // Make, link, and use the program
  GLuint program = glCreateProgram();
  glAttachShader(program, vertexShader);
  glAttachShader(program, fragmentShader);
  glLinkProgram(program);
  glUseProgram(program);

  return program;
}

} // namespace simit::internal

void initDrawing() {
  int argc = 1;
  char* argv[1];
  argv[0] = "graph_viz";
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
  glutInitWindowSize(640, 480);
  glutCreateWindow("Graph visualization");

  glShadeModel(GL_SMOOTH);
  glEnable(GL_DEPTH_TEST);
}

void drawPoints(const Set<>& points, FieldRef<double,3> coordField,
                float r, float g, float b, float a) {
  RawDataFieldRef<double,3> xData(&coordField);

  GLuint vbo;
  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(GLdouble) * points.getSize() * 3,
               (GLdouble*)xData.getDataPtr(), GL_STREAM_DRAW);

  GLuint program = internal::createGLProgram(internal::kPointsVertexShader,
                                             internal::kPointsFragmentShader);
  GLint colorUniform = glGetUniformLocation(program, "color");
  glUniform4f(colorUniform, r, g, b, a);
  GLint posAttrib = glGetAttribLocation(program, "position");
  glVertexAttribPointer(posAttrib, 3, GL_DOUBLE, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(posAttrib);

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  gluLookAt(0.0, 0.0, 1.0,
            0.0, 0.0, 0.0,
            0.0, 1.0, 0.0);
  glDrawArrays(GL_POINTS, 0, points.getSize());
  glutSwapBuffers();
  glDisableVertexAttribArray(posAttrib);
}

} // namespace simit
