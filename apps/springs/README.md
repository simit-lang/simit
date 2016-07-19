Point the cmake build system to Simit like so:

    export SIMIT_INCLUDE_DIR=<path to simit src dir>
    export SIMIT_LIBRARY_DIR=<path to simit lib dir>

Build the springs example like so:

    mkdir build
    cd build
    cmake ..
    make

Run the explicit springs example like so:

    ./springs ../esprings.sim ../../data/tet-bunny/bunny.1

Run the implicit springs example like so:

    ./springs ../isprings.sim ../../data/tet-bunny/bunny.1

The springs code run for 100 time steps and leave 100 .obj files in
your build directory.
