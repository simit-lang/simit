Point the cmake build system to Simit like so:

    export SIMIT_INCLUDE_DIR=<path to simit src dir>
    export SIMIT_LIB_DIR=<path to simit lib dir>

Build the lulesh example like so:

    mkdir build
    cd build
    cmake ..
    make

Run the lulesh orginal example like so (only one region):

    ./lulesh-serial -r 1 


Run the lulesh simit version like so :

    ./lulesh-simit 
