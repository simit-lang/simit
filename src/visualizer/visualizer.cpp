#include "visualizer.h"

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
const string& kFacesVertexShader = " \
attribute vec4 position;             \
varying vec4 vert;                   \
void main() {                        \
  vert = position;                   \
  gl_Position = position;            \
}                                    \
";
const string& kFacesFragmentShader = "\
uniform vec4 color;                   \
varying vec4 vert;                    \
void main() {                         \
  gl_FragColor = color * -vert[2];     \
}                                     \
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
  argv[0] = "visualizer";
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
  glutInitWindowSize(640, 480);
  glutCreateWindow("Graph visualization");

  glShadeModel(GL_SMOOTH);
  glEnable(GL_DEPTH_TEST);
}

void drawPoints(const Set<>& points, FieldRef<double,3> coordField,
                float r, float g, float b, float a) {
  GLuint vbo;
  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(GLdouble) * points.getSize() * 3,
               (GLdouble*)coordField.getData(), GL_STREAM_DRAW);

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

void drawEdges(Set<2>& edges, FieldRef<double,3> coordField,
               float r, float g, float b, float a) {
  // FIXME(gkanwar): Hack to copy edge data into a double array
  GLdouble* data = new GLdouble[edges.getSize() * 2 * 3];
  int index = 0;
  for (auto elem = edges.begin(); elem != edges.end(); ++elem) {
    std::cout << "Edge!" << std::endl;
    for (auto endPoint = edges.endpoints_begin(*elem);
         endPoint != edges.endpoints_end(*elem); endPoint++) {
      assert(index < (edges.getSize() * 2 * 3) &&
             "Too many edges in set edge info.");
      TensorRef<double,3> point = coordField.get(*endPoint);
      std::cout << "Endpoint: " << point(0) << "," << point(1) << "," << point(2) << std::endl;
      data[index] = point(0);
      data[index+1] = point(1);
      data[index+2] = point(2);
      index += 3;
      std::cout << index << std::endl;
    }
  }

  GLuint vbo;
  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(GLdouble) * edges.getSize() * 2 * 3,
               data, GL_STREAM_DRAW);

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
  std::cout << "Drawing " << edges.getSize() * 2 << std::endl;
  for (int i = 0; i < edges.getSize(); i++) {
    std::cout << data[i*2*3] << "," << data[i*2*3+1] << "," << data[i*2*3+2] << ":"
              << data[i*2*3+3] << "," << data[i*2*3+4] << "," << data[i*2*3+5] << std::endl;
  }
  glDrawArrays(GL_LINES, 0, edges.getSize() * 2);
  glutSwapBuffers();
  glDisableVertexAttribArray(posAttrib);
}

void drawFaces(Set<3>& faces, FieldRef<double,3> coordField,
               float r, float g, float b, float a) {
  // FIXME(gkanwar): Hack to copy edge data into a double array
  GLdouble* data = new GLdouble[faces.getSize() * 3 * 3];
  int index = 0;
  for (auto elem = faces.begin(); elem != faces.end(); ++elem) {
    std::cout << "Edge!" << std::endl;
    for (auto endPoint = faces.endpoints_begin(*elem);
         endPoint != faces.endpoints_end(*elem); endPoint++) {
      assert(index < (faces.getSize() * 3 * 3) &&
             "Too many faces in set edge info.");
      TensorRef<double,3> point = coordField.get(*endPoint);
      std::cout << "Endpoint: " << point(0) << "," << point(1) << "," << point(2) << std::endl;
      data[index] = point(0);
      data[index+1] = point(1);
      data[index+2] = point(2);
      index += 3;
      std::cout << index << std::endl;
    }
  }

  GLuint vbo;
  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(GLdouble) * faces.getSize() * 3 * 3,
               data, GL_STREAM_DRAW);

  GLuint program = internal::createGLProgram(internal::kFacesVertexShader,
                                             internal::kFacesFragmentShader);
  GLint colorUniform = glGetUniformLocation(program, "color");
  glUniform4f(colorUniform, r, g, b, a);
  GLint posAttrib = glGetAttribLocation(program, "position");
  glVertexAttribPointer(posAttrib, 3, GL_DOUBLE, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(posAttrib);

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  gluLookAt(0.0, 0.0, 1.0,
            0.0, 0.0, 0.0,
            0.0, 1.0, 0.0);
  std::cout << "Drawing " << faces.getSize() * 3 << std::endl;
  for (int i = 0; i < faces.getSize(); i++) {
    std::cout << data[i*2*3] << "," << data[i*2*3+1] << "," << data[i*2*3+2] << ":"
              << data[i*2*3+3] << "," << data[i*2*3+4] << "," << data[i*2*3+5] << std::endl;
  }
  glDrawArrays(GL_TRIANGLES, 0, faces.getSize() * 3);
  glutSwapBuffers();
  glDisableVertexAttribArray(posAttrib);
}

} // namespace simit
