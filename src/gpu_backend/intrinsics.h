#ifndef INTRINSICS_2_H
#define INTRINSICS_2_H

namespace simit {
namespace internal {

extern int simit_gpu_initmod_intrinsics_length;
// Load the intrinsincs module and return the char[] representation.
// Length is available after this function is called.
std::unique_ptr<char[]> get_simit_gpu_initmod_intrinsics();

}}  // namespace simit::internal

#endif
