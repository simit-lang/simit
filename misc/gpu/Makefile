# TODO: integrate into real build

# CLANG ?= /scratch/llvm32/bin/clang
CLANG ?= clang
TARGET_DIR=../../src/backend/gpu/
ARCHS=compute_20 compute_30 compute_35
LIBDEVICE_BC_FILES=$(ARCHS:%=libdevice.%.bc)
LIBDEVICE_C_FILES=$(LIBDEVICE_BC_FILES:%.bc=$(TARGET_DIR)/initmod.%.cpp)
INITMOD_C_FILES=$(LIBDEVICE_C_FILES) $(TARGET_DIR)/initmod.intrinsics.cpp
LLVM_BINDIR=$(shell llvm-config --bindir)
LLVM_LINK=$(LLVM_BINDIR)/llvm-link
LLVM_AS=$(LLVM_BINDIR)/llvm-as

linalg.ll: linalg.c
	# clean out: target* lines, attributes* lines, and #X attribute references - NVVM doesn't like them
	$(CLANG) -S -emit-llvm $< -o - | grep -v '^target ' | grep -v '^attributes' | sed 's/) \#. {/) alwaysinline {/g' | sed 's/ \#.$$//g' > $@

bitcode2cpp: bitcode2cpp.cpp
	c++ -o $@ $^

$(TARGET_DIR)/initmod.intrinsics.cpp: intrinsics.ll linalg.ll
	cat $^ | ./bitcode2cpp intrinsics > $@

$(TARGET_DIR)/initmod.%.cpp: bitcode2cpp %.bc
	./bitcode2cpp $* < $*.bc > $@

.PHONY: all
all: $(INITMOD_C_FILES)
