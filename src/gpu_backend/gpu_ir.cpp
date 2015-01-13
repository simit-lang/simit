#include "gpu_ir.h"

namespace simit {

namespace internal {
  
std::ostream &operator<<(std::ostream &os, const GPUSharding &shard) {
  os << "GPU Sharding [ ";
  if (shard.xSharded) os << shard.xVar << " : " << shard.xDomain;
  if (shard.ySharded) os << ", " << shard.yVar << " : " << shard.yDomain;
  if (shard.zSharded) os << ", " << shard.zVar << " : " << shard.zDomain;
  os << " ]\n";
  return os;
}

bool operator==(const GPUSharding& sharding1, const GPUSharding& sharding2) {
  return ((!sharding1.xSharded && !sharding2.xSharded) ||
          (sharding1.xSharded && sharding2.xSharded &&
          sharding1.xDomain == sharding2.xDomain)) &&
      ((!sharding1.ySharded && !sharding2.ySharded) ||
       (sharding1.ySharded && sharding2.ySharded &&
       sharding1.yDomain == sharding2.yDomain)) &&
      ((!sharding1.zSharded && !sharding2.zSharded) ||
       (sharding1.zSharded && sharding2.zSharded &&
       sharding1.zDomain == sharding2.zDomain));
}


}}  // namespace simit::internal
