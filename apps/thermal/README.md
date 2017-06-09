Point the cmake build system to Simit like so:

    export SIMIT_INCLUDE_DIR=<path to simit src dir>
    export SIMIT_LIBRARY_DIR=<path to simit lib dir>

Mandatory: Download and install CGNS  from 
				https://cgns.github.io/
Optional:  Download and install Visit from 
				https://wci.llnl.gov/simulation/computer-codes/visit/
				
Build the thermic example like so:

    mkdir build
    cd build
    cmake ..
    make

Run the explicit thermic example like so:

    ./thermic ../params.therm


