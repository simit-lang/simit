#include "graph.h"

namespace simit {

void Set::increaseCapacity() {
  for (auto f : fields) {
    int typeSize = f->sizeOfType;
    f->data = realloc(f->data, (capacity+capacityIncrement) * typeSize);
    memset((char*)(f->data)+capacity*typeSize, 0, capacityIncrement*typeSize);

    for (FieldRefBase *fieldRef : f->fieldReferences) {
      fieldRef->data = f->data;
    }
  }
  capacity += capacityIncrement;
}

} // namespace simit
