# TODO: integrate into real build

LIBDEVICE_MODULES=libdevice.compute_20.10.bc libdevice.compute_30.10.bc libdevice.compute_35.10.bc
LIBDEVICE_C_FILES=$(LIBDEVICE_MODULES:%.10.bc=../src/%.cpp)

bitcode2cpp: bitcode2cpp.cpp
	c++ -o $@ $^

../src/gpu_backend/libdevice.%.cpp: bitcode2cpp libdevice.%.10.bc
	./bitcode2cpp $* < libdevice.$*.10.bc > $@

.PHONY: all
all: $(LIBDEVICE_C_FILES)
