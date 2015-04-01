#GrayBat#



##Description##

**GrayBat** is a C++ library that presents the **graph-based virtual
overlay network**, a flexible description which enables a mapping of
algorithms to communication patterns and further a mapping of these
communication patterns onto the available hardware topologies. These
mappings were established as an intermediate layer between an
application and communication libraries. This approach provides a
dynamic remapping at run-time, which will allow to address load
balancing and fault tolerance for upcoming exascale systems. The
[Game of Life](src/gol.cc) and [N Body](src/nbody.cc) example
simulations provide full demonstrations of utilizing GrayBat in a
scientific simulations.


##Referencing##

GrayBat is a scientific project. If you **present and/or publish** scientific
results that used GrayBat, you should set this as a **reference**.


##Software License##

GrayBat  is licensed under the **GPLv3+**. Please refer to our [LICENSE.md](LICENSE.md)


##Dependencies##

 * cmake 3.0.2
 * Boost 1.57.0
 * OpenMPI 1.8.0
 * g++ 4.9.2
 * Doxygen 1.8.8


##Compiling##

 * clone the repository: `git clone https://github.com/computationalradiationphysics/GrayBat.git`
 * create the build directory: `mkdir -p GrayBat/build`
 * go to build directory: `cd GrayBat/build`
 * set compiler: `export CXX=[g++,clang++]`
 * create Makefile `cmake ..`
 * build project : `make [target]`

##Tested Compilers##

 * clang 3.5
 * g++ 4.9.2

### Current Compilation Status:

| *branch* | *state* | *description* |
| -------- | --------| ------------- |
| **master** | [![Build Status](http://haseongpu.mooo.com/api/badge/github.com/erikzenker/GrayBat/status.svg?branch=master)](http://haseongpu.mooo.com/github.com/erikzenker/GrayBat) |  stable releases |
| **dev**  | [![Build Status](http://haseongpu.mooo.com/api/badge/github.com/erikzenker/GrayBat/status.svg?branch=dev)](http://haseongpu.mooo.com/github.com/erikzenker/GrayBat) |development branch |

##Example Targets##

GrayBat is a library, but some examples are provided to demonstrate
the usage. All targets provide the synopthis of their executables.

 * GoL: Game of Life simulation

 * NBody: N Body simulation

 * doc: Build documentation in ../doc/

 * clean: Cleanup build directory



##Authors##

 * Erik Zenker (erikzenker@hotmail.com)
