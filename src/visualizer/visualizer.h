#ifndef SIMIT_VISUALIZER_VISUALIZER_H
#define SIMIT_VISUALIZER_VISUALIZER_H

#include <pthread.h>

#include "graph.h"

namespace simit {

void initDrawing(int argc, char** argv);
void drawPoints(const Set<>& points, FieldRef<double,3> coordField,
                float r, float g, float b, float a);
void drawEdges(Set<2>& edges, FieldRef<double,3> coordField,
               float r, float g, float b, float a);
void drawFaces(Set<3>& faces, FieldRef<double,3> coordField,
               float r, float g, float b, float a);

} // namespace simit

#endif // SIMIT_VISUALIZER_VISUALIZER_H
