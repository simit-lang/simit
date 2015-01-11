# TODO: integrate into real build

ARCHS=compute_20 compute_30 compute_35
LIBDEVICE_BC_FILES=$(ARCHS:%=libdevice.%.bc)
INTRINSICS_BC_FILES=intrinsics.bc
INITMOD_BC_FILES=$(LIBDEVICE_BC_FILES) $(INTRINSICS_BC_FILES)
INITMOD_C_FILES=$(INITMOD_BC_FILES:%.bc=../src/gpu_backend/initmod.%.cpp)
LLVM_BINDIR=$(shell llvm-config --bindir)
LLVM_LINK=$(LLVM_BINDIR)/llvm-link
LLVM_AS=$(LLVM_BINDIR)/llvm-as

linalg.ll: linalg.c
	clang -S -emit-llvm $< -o - | grep -v "^target " > $@
	
intrinsics.bc: linalg.ll intrinsics.ll
	$(LLVM_LINK) $^ -o $@

bitcode2cpp: bitcode2cpp.cpp
	c++ -o $@ $^

../src/gpu_backend/initmod.%.cpp: bitcode2cpp %.bc
	./bitcode2cpp $* < $*.bc > $@

.PHONY: all
all: $(INITMOD_C_FILES)
