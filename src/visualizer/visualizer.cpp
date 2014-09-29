#include "visualizer.h"

#include <cassert>
#include <cmath>
#include <functional>
#include <mutex>
#include <pthread.h>
#include <iostream>
#include <queue>
#include <string>

#include <GL/gl.h>
#include <GL/glext.h>
#include <GL/glut.h>

using namespace std;

namespace simit {

namespace internal {

const string& kConstVertexShader = "\
attribute vec4 position;            \
uniform mat4 transMat;              \
void main() {                       \
  gl_Position = position * transMat;\
}                                   \
";
const string& kConstFragmentShader = "\
uniform vec4 color;                  \
void main() {                        \
  gl_FragColor = color;              \
}                                    \
";
const string& kFlatVertexShader = "\
attribute vec3 position;           \
attribute vec3 normal;             \
uniform mat4 transMat;             \
uniform mat4 invTransMat;          \
varying vec3 vert;                 \
varying vec3 normalV;              \
void main() {                      \
  gl_Position = vec4(position[0], position[1], position[2], 1.0)\
      * transMat;                  \
  vert = vec3(gl_Position[0], gl_Position[1], gl_Position[2]);\
  vec4 normal4 = vec4(normal[0], normal[1], normal[2], 1.0)\
      * invTransMat;\
  normalV = vec3(normal4[0], normal4[1], normal4[2]);\
}                                  \
";
const string& kFlatFragmentShader = "\
uniform vec4 color;                  \
varying vec3 vert;                   \
varying vec3 normalV;                \
void main() {                        \
  vec3 Lpos = vec3(-0.2, 1.0, 1.0);  \
  vec3 L = normalize(Lpos - vert);   \
  vec3 E = normalize(-vert);         \
  vec3 R = normalize(-reflect(L, normalV));\
                                     \
  vec4 Iamb = vec4(0.2, 0.2, 0.2, 1.0);\
  vec4 Idiff = vec4(0.2, 0.2, 0.2, 1.0)\
    * max(dot(normalV,L), 0.0);\
  Idiff = clamp(Idiff, 0.0, 1.0);    \
                                     \
  gl_FragColor = color + Iamb + Idiff;\
}                                    \
";
const string& kPhongVertexShader = "                 \
attribute vec4 position;                             \
varying vec3 vert;                                   \
varying vec3 normal;                                 \
void main() {                                        \
  vert = vec3(position[0], position[1], position[2]);\
  normal = normalize(gl_NormalMatrix * gl_Normal);   \
  gl_Position = position;                            \
}                                                    \
";
const string& kPhongFragmentShader = "\
uniform vec4 color;                   \
varying vec3 vert;                    \
varying vec3 normal;                  \
void main() {                         \
  vec3 L = normalize(gl_LightSource[0].position.xyz - vert);\
  vec3 E = normalize(-vert);                    \
  vec3 R = normalize(-reflect(L, normal));      \
                                                \
  vec4 Iamb = gl_FrontLightProduct[0].ambient;  \
  vec4 Idiff = gl_FrontLightProduct[0].diffuse  \
       * max(dot(normal,L), 0.0);               \
  Idiff = clamp(Idiff, 0.0, 1.0);               \
  vec4 Ispec = gl_FrontLightProduct[0].specular \
       * pow(max(dot(R,E),0.0), 0.3);           \
  Ispec = clamp(Ispec, 0.0, 1.0);               \
                                                \
  gl_FragColor = color + Iamb + Idiff + Ispec;  \
}                                               \
";

// Current glut main loop thread. Should be joined before exiting.
pthread_t glutThread;
// Draw function, set by the latest draw call, so that we can redraw upon
// POV rotation/pan. Default is to draw nothing.
std::function<void(void)> drawFunc = [](){};
// Lock the draw function, so we can change it or replace the data under it
// without race conditions.
std::mutex drawFuncLock;
// Data references held by GL based on the previous call to draw*.
std::queue<void*> heldReferences;
// Eye theta
double eyeTheta = 0.0;
// Current transformation matrix
GLfloat transMat[16];

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
  if (!success) {
    GLint logSize;
    glGetShaderiv(vertexShader, GL_INFO_LOG_LENGTH, &logSize);
    char* errorLog = new char[logSize];
    glGetShaderInfoLog(vertexShader, logSize, &logSize, &errorLog[0]);
    std::cerr << errorLog << std::endl;
    assert(false &&
           "Vertex shader failed to compile");
    delete errorLog;
  }
  glCompileShader(fragmentShader);
  glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
  if (!success) {
    GLint logSize;
    glGetShaderiv(fragmentShader, GL_INFO_LOG_LENGTH, &logSize);
    char* errorLog = new char[logSize];
    glGetShaderInfoLog(fragmentShader, logSize, &logSize, &errorLog[0]);
    std::cerr << errorLog << std::endl;
    assert(false &&
           "Fragment shader failed to compile");
    delete errorLog;
  }

  // Make, link, and use the program
  GLuint program = glCreateProgram();
  glAttachShader(program, vertexShader);
  glAttachShader(program, fragmentShader);
  glLinkProgram(program);
  glUseProgram(program);

  return program;
}

// Writes 16 doubles into matrix, representing the 4x4 transformation
// matrix associated with a rotation about the Y axis of theta (rad).
void buildYRotMatrix(double theta, GLfloat matrix[16]) {
  std::cout << "buildYRotMatrix: " << theta << std::endl;
  GLfloat outMat[16] = {
    (float)cos(theta), (float)-sin(theta), 0, 0,
    (float)sin(theta), (float)cos(theta), 0, 0,
    0, 0, 1, 0,
    0, 0, 0, 1
  };
  memcpy(matrix, outMat, sizeof(outMat));
}

// Invert a 4x4 matrix explicitly. Writes 16 doubles into inverse.
// Code based on StackOverflow response based on MESA implementation:
// http://stackoverflow.com/questions/1148309/inverting-a-4x4-matrix
void invertMatrix(const GLfloat matrix[16], GLfloat inverse[16]) {
  double inv[16], det;
  int i;

  inv[0] =
      matrix[5]  * matrix[10] * matrix[15] - 
      matrix[5]  * matrix[11] * matrix[14] - 
      matrix[9]  * matrix[6]  * matrix[15] + 
      matrix[9]  * matrix[7]  * matrix[14] +
      matrix[13] * matrix[6]  * matrix[11] - 
      matrix[13] * matrix[7]  * matrix[10];
  inv[4] =
      -matrix[4]  * matrix[10] * matrix[15] + 
      matrix[4]  * matrix[11] * matrix[14] + 
      matrix[8]  * matrix[6]  * matrix[15] - 
      matrix[8]  * matrix[7]  * matrix[14] - 
      matrix[12] * matrix[6]  * matrix[11] + 
      matrix[12] * matrix[7]  * matrix[10];
  inv[8] =
      matrix[4]  * matrix[9] * matrix[15] - 
      matrix[4]  * matrix[11] * matrix[13] - 
      matrix[8]  * matrix[5] * matrix[15] + 
      matrix[8]  * matrix[7] * matrix[13] + 
      matrix[12] * matrix[5] * matrix[11] - 
      matrix[12] * matrix[7] * matrix[9];
  inv[12] =
      -matrix[4]  * matrix[9] * matrix[14] + 
      matrix[4]  * matrix[10] * matrix[13] +
      matrix[8]  * matrix[5] * matrix[14] - 
      matrix[8]  * matrix[6] * matrix[13] - 
      matrix[12] * matrix[5] * matrix[10] + 
      matrix[12] * matrix[6] * matrix[9];
  inv[1] =
      -matrix[1]  * matrix[10] * matrix[15] + 
      matrix[1]  * matrix[11] * matrix[14] + 
      matrix[9]  * matrix[2] * matrix[15] - 
      matrix[9]  * matrix[3] * matrix[14] - 
      matrix[13] * matrix[2] * matrix[11] + 
      matrix[13] * matrix[3] * matrix[10];
  inv[5] =
      matrix[0]  * matrix[10] * matrix[15] - 
      matrix[0]  * matrix[11] * matrix[14] - 
      matrix[8]  * matrix[2] * matrix[15] + 
      matrix[8]  * matrix[3] * matrix[14] + 
      matrix[12] * matrix[2] * matrix[11] - 
      matrix[12] * matrix[3] * matrix[10];
  inv[9] =
      -matrix[0]  * matrix[9] * matrix[15] + 
      matrix[0]  * matrix[11] * matrix[13] + 
      matrix[8]  * matrix[1] * matrix[15] - 
      matrix[8]  * matrix[3] * matrix[13] - 
      matrix[12] * matrix[1] * matrix[11] + 
      matrix[12] * matrix[3] * matrix[9];
  inv[13] =
      matrix[0]  * matrix[9] * matrix[14] - 
      matrix[0]  * matrix[10] * matrix[13] - 
      matrix[8]  * matrix[1] * matrix[14] + 
      matrix[8]  * matrix[2] * matrix[13] + 
      matrix[12] * matrix[1] * matrix[10] - 
      matrix[12] * matrix[2] * matrix[9];
  inv[2] =
      matrix[1]  * matrix[6] * matrix[15] - 
      matrix[1]  * matrix[7] * matrix[14] - 
      matrix[5]  * matrix[2] * matrix[15] + 
      matrix[5]  * matrix[3] * matrix[14] + 
      matrix[13] * matrix[2] * matrix[7] - 
      matrix[13] * matrix[3] * matrix[6];
  inv[6] =
      -matrix[0]  * matrix[6] * matrix[15] + 
      matrix[0]  * matrix[7] * matrix[14] + 
      matrix[4]  * matrix[2] * matrix[15] - 
      matrix[4]  * matrix[3] * matrix[14] - 
      matrix[12] * matrix[2] * matrix[7] + 
      matrix[12] * matrix[3] * matrix[6];
  inv[10] =
      matrix[0]  * matrix[5] * matrix[15] - 
      matrix[0]  * matrix[7] * matrix[13] - 
      matrix[4]  * matrix[1] * matrix[15] + 
      matrix[4]  * matrix[3] * matrix[13] + 
      matrix[12] * matrix[1] * matrix[7] - 
      matrix[12] * matrix[3] * matrix[5];
  inv[14] =
      -matrix[0]  * matrix[5] * matrix[14] + 
      matrix[0]  * matrix[6] * matrix[13] + 
      matrix[4]  * matrix[1] * matrix[14] - 
      matrix[4]  * matrix[2] * matrix[13] - 
      matrix[12] * matrix[1] * matrix[6] + 
      matrix[12] * matrix[2] * matrix[5];
  inv[3] =
      -matrix[1] * matrix[6] * matrix[11] + 
      matrix[1] * matrix[7] * matrix[10] + 
      matrix[5] * matrix[2] * matrix[11] - 
      matrix[5] * matrix[3] * matrix[10] - 
      matrix[9] * matrix[2] * matrix[7] + 
      matrix[9] * matrix[3] * matrix[6];
  inv[7] =
      matrix[0] * matrix[6] * matrix[11] - 
      matrix[0] * matrix[7] * matrix[10] - 
      matrix[4] * matrix[2] * matrix[11] + 
      matrix[4] * matrix[3] * matrix[10] + 
      matrix[8] * matrix[2] * matrix[7] - 
      matrix[8] * matrix[3] * matrix[6];
  inv[11] =
      -matrix[0] * matrix[5] * matrix[11] + 
      matrix[0] * matrix[7] * matrix[9] + 
      matrix[4] * matrix[1] * matrix[11] - 
      matrix[4] * matrix[3] * matrix[9] - 
      matrix[8] * matrix[1] * matrix[7] + 
      matrix[8] * matrix[3] * matrix[5];
  inv[15] =
      matrix[0] * matrix[5] * matrix[10] - 
      matrix[0] * matrix[6] * matrix[9] - 
      matrix[4] * matrix[1] * matrix[10] + 
      matrix[4] * matrix[2] * matrix[9] + 
      matrix[8] * matrix[1] * matrix[6] - 
      matrix[8] * matrix[2] * matrix[5];

  det = matrix[0] * inv[0] +
      matrix[1] * inv[4] +
      matrix[2] * inv[8] +
      matrix[3] * inv[12];

  std::cout << "Invert: " << std::endl
            << matrix[0] << ","
            << matrix[1] << ","
            << matrix[2] << ","
            << matrix[3] << std::endl
            << matrix[4] << ","
            << matrix[5] << ","
            << matrix[6] << ","
            << matrix[7] << std::endl
            << matrix[8] << ","
            << matrix[9] << ","
            << matrix[10] << ","
            << matrix[11] << std::endl
            << matrix[12] << ","
            << matrix[13] << ","
            << matrix[14] << ","
            << matrix[15] << std::endl;
  assert(det != 0 &&
         "Non-invertible transformation matrix");

  det = 1.0 / det;

  for (i = 0; i < 16; i++)
    inverse[i] = inv[i] * det;
}

void *handleWindowEvents(void *arg) {
  // Let the glut main loop dispatch all events to registered handlers
  glutMainLoop();
  return arg;
}

void handleDraw() {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  // Update transformation matrix
  buildYRotMatrix(eyeTheta, transMat);
  // Assumes that the relevant VBO setup for the draw function has been
  // set up before this draw function was set.
  drawFuncLock.lock();
  drawFunc();
  drawFuncLock.unlock();
  glutSwapBuffers();
}

void handleKeyboardEvent(unsigned char key, int x, int y) {
  if (key == 'z') {
    std::cout << "Resetting viewpoint" << std::endl;
    // TODO(gkanwar): Implement this!
  }
}

void handleSpecialKeyEvent(int key, int x, int y) {
  std::cout << "Special key!" << std::endl;
  if (key == GLUT_KEY_RIGHT) {
    std::cout << "Right" << std::endl;
    eyeTheta += 0.1;
    if (eyeTheta > 2*M_PI) {
      eyeTheta -= 2*M_PI;
    }
    glutPostRedisplay();
  } else if (key == GLUT_KEY_LEFT) {
    std::cout << "Left" << std::endl;
    eyeTheta -= 0.1;
    if (eyeTheta < 0) {
      eyeTheta += 2*M_PI;
    }
    glutPostRedisplay();
  }
  std::cout << "Eye theta: " << eyeTheta << std::endl;
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

  glutDisplayFunc(internal::handleDraw);
  glutSpecialFunc(internal::handleSpecialKeyEvent);
  glutKeyboardFunc(internal::handleKeyboardEvent);

  glShadeModel(GL_SMOOTH);
  glEnable(GL_DEPTH_TEST);

  // Initialize transformation matrix
  // internal::buildYRotMatrix(internal::eyeTheta, internal::transMat);

  int ret;
  ret = pthread_create(&internal::glutThread, NULL,
                       internal::handleWindowEvents, NULL);
  assert(!ret &&
         "Could not create event handler thread");
}

void drawPoints(const Set<>& points, FieldRef<double,3> coordField,
                float r, float g, float b, float a) {
  internal::drawFuncLock.lock();
  while (!internal::heldReferences.empty()) {
    delete internal::heldReferences.front();
    internal::heldReferences.pop();
  }
  GLdouble* pointData = new GLdouble[points.getSize() * 3];
  internal::heldReferences.push(pointData);
  memcpy(pointData, coordField.getData(), sizeof(GLdouble)*points.getSize()*3);

  GLuint vbo;
  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(GLdouble) * points.getSize() * 3,
               pointData, GL_STREAM_DRAW);

  GLuint program = internal::createGLProgram(internal::kConstVertexShader,
                                             internal::kConstFragmentShader);
  GLint colorUniform = glGetUniformLocation(program, "color");
  glUniform4f(colorUniform, r, g, b, a);
  GLint transMatUniform = glGetUniformLocation(program, "transMat");
  glUniformMatrix4fv(transMatUniform, 1, GL_FALSE, internal::transMat);
  GLint posAttrib = glGetAttribLocation(program, "position");
  glVertexAttribPointer(posAttrib, 3, GL_DOUBLE, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(posAttrib);

  // Set the draw func, to be repeatedly called
  int arraySize = points.getSize();
  internal::drawFunc = [arraySize](){
    glDrawArrays(GL_POINTS, 0, arraySize);
  };
  internal::drawFuncLock.unlock();

  glutPostRedisplay();
}

void drawEdges(Set<2>& edges, FieldRef<double,3> coordField,
               float r, float g, float b, float a) {
  internal::drawFuncLock.lock();
  while (!internal::heldReferences.empty()) {
    delete internal::heldReferences.front();
    internal::heldReferences.pop();
  }
  GLdouble* data = new GLdouble[edges.getSize() * 2 * 3];
  internal::heldReferences.push(data);
  int index = 0;
  for (auto elem = edges.begin(); elem != edges.end(); ++elem) {
    // std::cout << "Edge!" << std::endl;
    for (auto endPoint = edges.endpoints_begin(*elem);
         endPoint != edges.endpoints_end(*elem); endPoint++) {
      assert(index < (edges.getSize() * 2 * 3) &&
             "Too many edges in set edge info.");
      TensorRef<double,3> point = coordField.get(*endPoint);
      // std::cout << "Endpoint: " << point(0) << "," << point(1) << "," << point(2) << std::endl;
      data[index] = point(0);
      data[index+1] = point(1);
      data[index+2] = point(2);
      index += 3;
      // std::cout << index << std::endl;
    }
  }

  GLuint vbo;
  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(GLdouble) * edges.getSize() * 2 * 3,
               data, GL_STREAM_DRAW);

  GLuint program = internal::createGLProgram(internal::kConstVertexShader,
                                             internal::kConstFragmentShader);
  GLint colorUniform = glGetUniformLocation(program, "color");
  glUniform4f(colorUniform, r, g, b, a);
  GLint transMatUniform = glGetUniformLocation(program, "transMat");
  glUniformMatrix4fv(transMatUniform, 1, GL_FALSE, internal::transMat);
  GLint posAttrib = glGetAttribLocation(program, "position");
  glVertexAttribPointer(posAttrib, 3, GL_DOUBLE, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(posAttrib);

  // Set the draw func, to be repeatedly called
  int arraySize = edges.getSize() * 2;
  internal::drawFunc = [arraySize]() {
    glDrawArrays(GL_LINES, 0, arraySize);
  };
  internal::drawFuncLock.unlock();

  glutPostRedisplay();
}

void drawFaces(Set<3>& faces, FieldRef<double,3> coordField,
               float r, float g, float b, float a) {
  internal::drawFuncLock.lock();
  // FIXME(gkanwar): Hack to copy edge data into a double array
  while (!internal::heldReferences.empty()) {
    delete internal::heldReferences.front();
    internal::heldReferences.pop();
  }
  GLdouble* posData = new GLdouble[faces.getSize() * 3 * 3];
  GLfloat* normData = new GLfloat[faces.getSize() * 3 * 3];
  internal::heldReferences.push(posData);
  internal::heldReferences.push(normData);

  int index = 0;
  for (auto elem = faces.begin(); elem != faces.end(); ++elem) {
    // std::cout << "Edge!" << std::endl;
    for (auto endPoint = faces.endpoints_begin(*elem);
         endPoint != faces.endpoints_end(*elem); endPoint++) {
      assert(index < (faces.getSize() * 3 * 3) &&
             "Too many faces in set edge info.");
      TensorRef<double,3> point = coordField.get(*endPoint);
      // std::cout << "Endpoint: " << point(0) << "," << point(1) << "," << point(2) << std::endl;
      posData[index] = point(0);
      posData[index+1] = point(1);
      posData[index+2] = point(2);
      index += 3;
      // std::cout << index << std::endl;
    }
    // Compute face normal, assuming right-hand convention
    float deltaAB[3] = {
      (float)(posData[index-6] - posData[index-9]),
      (float)(posData[index-6+1] - posData[index-9+1]),
      (float)(posData[index-6+2] - posData[index-9+2])
    };
    float deltaAC[3] = {
      (float)(posData[index-3] - posData[index-9]),
      (float)(posData[index-3+1] - posData[index-9+1]),
      (float)(posData[index-3+2] - posData[index-9+2])
    };
    float norm[3] = {
      deltaAB[1]*deltaAC[2] - deltaAB[2]*deltaAC[1],
      deltaAB[2]*deltaAC[0] - deltaAB[0]*deltaAC[2],
      deltaAB[0]*deltaAC[1] - deltaAB[1]*deltaAC[0]
    };
    float normMag = sqrt(norm[0]*norm[0] + norm[1]*norm[1] + norm[2]*norm[2]);
    // Set the norm on all three vertices
    normData[index-9] = normData[index-6] = normData[index-3] =
        norm[0] / normMag;
    normData[index-9+1] = normData[index-6+1] = normData[index-3+1] =
        norm[1] / normMag;
    normData[index-9+2] = normData[index-6+2] = normData[index-3+2] =
        norm[2] / normMag;
  }

  GLuint posVbo;
  glGenBuffers(1, &posVbo);
  glBindBuffer(GL_ARRAY_BUFFER, posVbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(GLdouble) * faces.getSize() * 3 * 3,
               posData, GL_STREAM_DRAW);
  GLuint normVbo;
  glGenBuffers(1, &normVbo);
  glBindBuffer(GL_ARRAY_BUFFER, normVbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * faces.getSize() * 3 * 3,
               normData, GL_STREAM_DRAW);

  GLuint program = internal::createGLProgram(internal::kFlatVertexShader,
                                             internal::kFlatFragmentShader);
  GLint colorUniform = glGetUniformLocation(program, "color");
  glUniform4f(colorUniform, r, g, b, a);
  GLint transMatUniform = glGetUniformLocation(program, "transMat");
  glUniformMatrix4fv(transMatUniform, 1, GL_FALSE, internal::transMat);
  GLint invTransMatUniform = glGetUniformLocation(program, "invTransMat");
  GLfloat invTransMat[16];
  internal::invertMatrix(internal::transMat, invTransMat);
  glUniformMatrix4fv(invTransMatUniform, 1, GL_FALSE, invTransMat);
  GLint posAttrib = glGetAttribLocation(program, "position");
  glBindBuffer(GL_ARRAY_BUFFER, posVbo);
  glVertexAttribPointer(posAttrib, 3, GL_DOUBLE, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(posAttrib);
  GLint normAttrib = glGetAttribLocation(program, "normal");
  glBindBuffer(GL_ARRAY_BUFFER, normVbo);
  glVertexAttribPointer(normAttrib, 3, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(normAttrib);

  int arraySize = faces.getSize() * 3;
  internal::drawFunc = [arraySize]() {
    glDrawArrays(GL_TRIANGLES, 0, arraySize);
  };
  internal::drawFuncLock.unlock();

  glutPostRedisplay();
}

} // namespace simit
