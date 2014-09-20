#include "graph.h"
#include "visualizer/visualizer.h"

using namespace simit;

int main() {
  Set<> points;

  FieldRef<double, 3> x = points.addField<double, 3>("x");
  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 4; j++) {
      auto el = points.addElement();
      x.set(el, {i/4.0, j/4.0, 0.0});
    }
  }

  initDrawing();

  while (true) {
    drawPoints(points, x, 1.0, 0.0, 0.0, 1.0);
  }
}
