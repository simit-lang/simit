# Copyright (c) 2013 NVIDIA Corporation
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

"""
FIRST: pip install pynvvm

This sample illustrates a simple LLVM IR -> PTX compiler implemented using
libNVVM. All command-line options are passed along to libNVVM. Arguments that
start with '-' are assumed to be options and are passed along accordingly.
Otherwise, options are treated as file names and are read as IR input(s). All
inputs will be linked together into a final PTX module.

NOTE: If you get errors about not being able to load libnvvm, please make sure
your [DY]LD_LIBRARY_PATH/PATH environment variable points to the libnvvm binary
in your CUDA installation, e.g.

  $ export LD_LIBRARY_PATH=/usr/local/cuda-5.5/nvvm/lib64
"""

import sys
from pynvvm.compiler import Program, ProgramException


if len(sys.argv) < 2:
    print('Usage: %s [options] ir-file-1 [ir-file-2 [ir-file-3 ...]]' % sys.argv[0])
    sys.exit(1)

try:
    # Create program object
    p = Program()
    options = []

    # Parse all options
    for a in sys.argv[1:]:
        if a.startswith('-'):
            # Treat as compiler option
            options.append(a)
        else:
            # Treat as compiler input
            with open(a, 'rb') as f:
                p.add_module(f.read())

    # Verify the input
    p.verify()

    # Run the compile
    ptx = p.compile(options)

    # Dump the output to stdout
    print(ptx)

    sys.exit(0)

except ProgramException as e:
    # An error occurred, dump it to stdout
    print('ERROR:\n%s\n' % repr(e))

