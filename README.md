Simit is a new programming language designed to make it easy to write
high-performance code to compute on sparse systems.  For more information see
[simit-lang.org](http://simit-lang.org).

Building Simit
==============
To build Simit you must install CMake 2.8.3 or later and LLVM 3.4.2.  See
http://www.cmake.org/cmake/resources/software.html for a copy of CMake. If your
system does not have packages for LLVM you can download it at
http://llvm.org/releases/download.html. You must then make sure llvm-config is
available in your path.

If you want to build LLVM yourself you can check it out using SVN:

    svn co https://llvm.org/svn/llvm-project/llvm/branches/release_34/ llvm-3.4.2

Then build it:

    cd llvm-3.4.2
    ./configure --disable-terminfo --enable-optimized --enable-assertions --enable-targets=x86
    make -j8

Then set the LLVM_CONFIG environment variable to point to llvm-config:

    export LLVM_CONFIG=<path to llvm>/Release+Asserts/bin/llvm-config

To perform an out-of-tree build of Simit do:

    cd <simit-directory>
    mkdir build
    cd build
    cmake ..
    make -j8

To run the test suite do (all tests should pass):

    cd <simit-directory>
    ./build/bin/simit-test

To check a Simit program do:

    cd <simit-directory>
    ./build/bin/simit-check <simit-program>

For example:

    ./build/bin/simit-check examples/springs.sim

To make the Simit bin directory part of your PATH:

    cd <simit-directory>
    export PATH="$PATH:`pwd`/build/bin"

To build Simit's documentation do:

    cd <simit-directory>
    doxygen

This will create a doc directory containing HTML documentation.
Open doc/index.html in your browser.

Simit Continuous Testing
========================
Simit is currently tested on a Jenkins instance located at
[builds.simit-lang.org](http://builds.simit-lang.org) . A test build is
triggered on every push to master and each pull request. The status of the test
is marked on the pull request in Github.
