Point the cmake build system to Simit like so:

    export SIMIT_INCLUDE_DIR=<path to simit src dir>
    export SIMIT_LIBRARY_DIR=<path to simit lib dir>

Build the thermic example like so:

    mkdir build
    cd build
    cmake ..
    make

Run the explicit thermic example like so:

    ./thermic ../thermic.sim ../../data/tet-bunny/bunny.1

The thermic code run for 100 time steps and leave 100 .obj files in
your build directory.
