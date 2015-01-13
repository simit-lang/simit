; Intentionally blank. We can include LL ASM GPU intrinsic implementations, as well.

define float @__test_f(float %x) {
  %xx = fmul float %x, %x
  ret float %xx
}
