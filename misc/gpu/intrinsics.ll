; Intentionally blank. We can include LL ASM GPU intrinsic implementations, as well.
; NVVM 7.0 demands a target triple
target triple = "nvptx64-nvidia-cuda"

define float @__test_f(float %x) {
  %xx = fmul float %x, %x
  ret float %xx
}
