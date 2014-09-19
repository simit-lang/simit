#ifndef SIMIT_GRAPH_VIZ_H
#define SIMIT_GRAPH_VIZ_H

#include "graph.h"

namespace simit {

void initDrawing();
void drawPoints(const Set<>& points, FieldRef<double,3> coordField,
                float r, float g, float b, float a);

} // namespace simit

#endif // SIMIT_GRAPH_VIZ_H
