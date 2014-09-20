#ifndef SIMIT_VISUALIZER_VISUALIZER_H
#define SIMIT_VISUALIZER_VISUALIZER_H

#include "graph.h"

namespace simit {

void initDrawing();
void drawPoints(const Set<>& points, FieldRef<double,3> coordField,
                float r, float g, float b, float a);
void drawEdges(const Set<2>& points, FieldRef<double,3> coordField,
               float r, float g, float b, float a);

} // namespace simit

#endif // SIMIT_VISUALIZER_VISUALIZER_H
