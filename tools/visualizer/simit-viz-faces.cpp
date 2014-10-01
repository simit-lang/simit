#include "graph.h"
#include "visualizer/visualizer.h"

using namespace simit;

int main(int argc, char** argv) {
  Set<> points;
  Set<3> faces(points, points, points);

  ElementRef p1 = points.addElement();
  ElementRef p2 = points.addElement();
  ElementRef p3 = points.addElement();
  ElementRef p4 = points.addElement();
  ElementRef p5 = points.addElement();
  ElementRef p6 = points.addElement();
  ElementRef p7 = points.addElement();
  ElementRef p8 = points.addElement();
  ElementRef p9 = points.addElement();

  FieldRef<double, 3> x = points.addField<double, 3>("x");
  x.set(p1, {0.0, 0.0, 0.0});
  x.set(p2, {0.5, 0.0, -0.2});
  x.set(p3, {0.0, 0.5, -0.2});
  x.set(p4, {-0.5, 0.0, -0.2});
  x.set(p5, {0.0, -0.5, -0.2});
  x.set(p6, {0.75, 0.75, -0.5});
  x.set(p7, {0.75, -0.75, -0.5});
  x.set(p8, {-0.75, 0.75, -0.5});
  x.set(p9, {-0.75, -0.75, -0.5});

  faces.addElement(p1, p2, p3);
  faces.addElement(p1, p5, p2);
  faces.addElement(p1, p4, p5);
  faces.addElement(p1, p3, p4);
  faces.addElement(p3, p2, p6);
  faces.addElement(p2, p5, p7);
  faces.addElement(p5, p4, p9);
  faces.addElement(p4, p3, p8);
  
  initDrawing(argc, argv);

  drawFaces(faces, x, 0.0, 0.0, 1.0, 1.0);

  while(true) {}
  exit(0);
}
