GrayBat
=======

<b>Gr</b>aph <b>A</b>pproach for Highl<b>y</b> Generic Communication Schemes <b>B</b>ased on <b>A</b>daptive <b>T</b>opologies 


##Description##

**GrayBat** is a C++ library that presents a graph-based communication
approach, which enables a mapping of algorithms to communication
patterns and further a mapping of these communication patterns to
varying hardware architectures. Therefore, a flexible and configurable
communication approach for parallel and distributed
applications. These mappings are established as an intermediate layer
between an application and communication libraries and are dynamically
adptable during run-time.

An application supported by GrayBat can be created with the following steps:

1. Decide on how fine grain the application domain should be decomposed
2. Model communication pattern of subdomains as a graph
3. Choose mapping of graph vertices to peers
4. Choose hardware to run the application on

![](utils/doxygen/graphics/overview.svg)

The central role to establish these mappings plays the
[communication and graph environment](utils/doxygen/cage.md)([cage](include/cage.hpp)).
The cage provides both communication and
graph operations to enable communication based on graphs.
The [Game of Life](utils/doxygen/gol.md) and [N Body](src/nbody.cc) example
simulations provide each a full demonstrations for utilizing GrayBat.

##Referencing##

GrayBat is a scientific project. If you **present and/or publish** scientific
results that used GrayBat, you should set this as a **reference**.


##Software License##


GrayBat  is licensed under the <b>GPLv3+</b>. Please refer to our [LICENSE.md](LICENSE.md)


##Dependencies##

 * cmake 3.0.2
 * Boost 1.57.0
 * OpenMPI 1.8.0 (for mpi [communication policy](utils/doxygen/communicationPolicy.md))
 * g++ 4.9.2
 * metis 5.1 (for graph partitioning)

##Compiling##

 * clone the repository: `git clone https://github.com/computationalradiationphysics/graybat.git`
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
its usage. All targets provide the synoptsis of their executables.

 * GoL: Game of Life simulation

 * NBody: N Body simulation

 * doc: Build documentation in ../doc/

 * clean: Cleanup build directory



##Authors##

 * Erik Zenker (erikzenker@posteo.de)
