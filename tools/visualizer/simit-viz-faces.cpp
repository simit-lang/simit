#include "graph.h"
#include "visualizer/visualizer.h"

using namespace simit;

int main() {
  Set<> points;
  Set<3> faces(points, points, points);

  ElementRef p1 = points.addElement();
  ElementRef p2 = points.addElement();
  ElementRef p3 = points.addElement();
  ElementRef p4 = points.addElement();

  FieldRef<double, 3> x = points.addField<double, 3>("x");
  x.set(p1, {0.0, 0.0, -0.2});
  x.set(p2, {0.5, 0.0, -0.2});
  x.set(p3, {0.0, 0.3, -0.2});
  x.set(p4, {0.75, 0.75, -1.0});

  faces.addElement(p1, p2, p3);
  faces.addElement(p2, p3, p4);

  initDrawing();

  while (true) {
    drawFaces(faces, x, 0.0, 0.0, 1.0, 1.0);
  }
}
