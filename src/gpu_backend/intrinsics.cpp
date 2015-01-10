#include <memory>
#include <fstream>

namespace simit {
namespace internal {

int simit_gpu_initmod_intrinsics_length;

std::unique_ptr<char[]> get_simit_gpu_initmod_intrinsics() {
  std::ifstream in;
  in.open("../support/intrinsics.ll");
  in.seekg(0, std::ios::end);
  simit_gpu_initmod_intrinsics_length = in.tellg();
  in.seekg(0, std::ios::beg);
  std::unique_ptr<char[]> buffer(new char[
      simit_gpu_initmod_intrinsics_length]);
  in.read(buffer.get(), simit_gpu_initmod_intrinsics_length);
  in.close();
  return buffer;
}

}}  // namespace simit::internal
