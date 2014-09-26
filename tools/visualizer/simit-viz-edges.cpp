#include "graph.h"
#include "visualizer/visualizer.h"

using namespace simit;

int main() {
  Set<> points;
  Set<2> edges(points, points);

  ElementRef p1 = points.addElement();
  ElementRef p2 = points.addElement();
  ElementRef p3 = points.addElement();

  FieldRef<double, 3> x = points.addField<double, 3>("x");
  x.set(p1, {0.0, 0.0, 0.0});
  x.set(p2, {0.5, 0.0, 0.0});
  x.set(p3, {0.0, 0.3, 0.0});

  edges.addElement(p1, p2);
  edges.addElement(p2, p3);
  edges.addElement(p3, p1);

  initDrawing();

  while (true) {
    drawEdges(edges, x, 0.0, 1.0, 0.0, 1.0);
  }
}
