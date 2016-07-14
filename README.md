# graw-merger

This repository contains the source code for the `graw2hdf` tool, which can be used to merge the GRAW files produced
by the GET electronics into an [HDF5](https://www.hdfgroup.org/HDF5/) file.

## Compiling

[CMake](https://cmake.org/) is required to build the project. In addition to that, the following external libraries must be installed:

- Boost libraries, version 1.55 or later. Download from http://www.boost.org/ and compile at least the System, Program Options, Filesystem, Thread, and Log libraries. If you install them somewhere bizarre, make sure to use the `-DBOOST_ROOT` option to tell CMake where you put them.

- [Armadillo](http://arma.sourceforge.net/), a linear algebra library.

To build the code, do this in the root of the repository:
```bash
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release -DBOOST_ROOT=/path/to/boost ..
make
make install  # sudo might be required
```

## Usage

`graw2hdf` can be used as follows:

```bash
graw2hdf [-v] --lookup LOOKUP INPUT [OUTPUT]
```

The `lookup` argument takes the path to the pad map lookup table, as csv. The `INPUT` positional argument should be the path to a directory containing GRAW files for a run. The `OUTPUT` argument is the path where the output HDF5 file should be created. If no output path is given, a file will be created next to the `INPUT` directory with the same name as that directory and the extension `.h5`.
