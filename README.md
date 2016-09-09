Simit is a new programming language designed to make it easy to write
high-performance code to compute on sparse systems.  For more information see
[simit-lang.org](http://simit-lang.org).

Build Simit
===========
To build Simit you must install
[CMake 2.8.3 or greater](http//www.cmake.org/cmake/resources/software.html) and
[LLVM 3.7 or greater](http://llvm.org/releases/download.html).
You must then make sure llvm-config is available in your path.

If you want to build LLVM yourself you can check it out using SVN:

    svn co https://llvm.org/svn/llvm-project/llvm/branches/release_37 llvm3.7

then build it:

    cd llvm3.7
    mkdir build
    cd build
    cmake -DLLVM_ENABLE_TERMINFO=OFF -DLLVM_TARGETS_TO_BUILD="X86;NVPTX" -DLLVM_ENABLE_ASSERTIONS=ON -DCMAKE_BUILD_TYPE=Release ..
    make -j8

then point Simit to it:

    export LLVM_CONFIG=<path to llvm>/build/bin/llvm-config

To perform an out-of-tree build of Simit do:

    cd <simit-directory>
    mkdir build
    cd build
    cmake ..
    make -j8

To run the test suite do (all tests should pass, but ignore disabled tests):

    cd <simit-directory>
    ./build/bin/simit-test

To check a Simit program:

    cd <simit-directory>
    ./build/bin/simit-check <simit-program>

For example:

    ./build/bin/simit-check simit-check apps/springs/isprings.sim

To make the Simit bin directory part of your PATH:

    cd <simit-directory>
    export PATH="$PATH:`pwd`/build/bin"

To build Simit's documentation:

    cd <simit-directory>
    doxygen

This will create a doc directory containing HTML documentation.  Open
`doc/index.html` in your browser.

License
=======
Simit is under a permissive MIT license. We encourage you to use it for
research or commercially!

If you do use it, we'd greatly appreciate a note saying what you use it for!
(However, we stress that you're under no obligation to do so.)
