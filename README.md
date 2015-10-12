Graybat
=======

<b>Gr</b>aph <b>A</b>pproach  for Highl<b>y</b>  Generic Communication
Schemes <b>B</b>ased on <b>A</b>daptive <b>T</b>opologies


##Description##

**Graybat** is a C++ library that presents a graph-based communication
approach, which enables a mapping of algorithms to communication
patterns and further a mapping of these communication patterns to
varying hardware architectures. Therefore, a flexible and configurable
communication approach for parallel and distributed
applications. These mappings are established as an intermediate layer
between an application and communication libraries and are dynamically
adptable during run-time.

The graybat API is currently unstable.


##Documentation##

Have a look at the documentation that is available [here](https://ComputationalRadiationPhysics.github.io/graybat)


##Referencing##

Graybat is a scientific project. If you **present and/or publish** scientific
results that used graybat, you should set this as a **reference**.

##Software License##

Graybat is licensed under the <b>LGPLv3+</b>. Please refer to our [LICENSE.md](LICENSE.md)


##Dependencies##

###Mandatory###
 * cmake 3.0.2
 * Boost 1.56.0
 * g++ 5.2.0 or clang 3.5
 * c++14

###Optional###
 * OpenMPI 1.8.0 (mpi communication policy)
 * zeromq 4.1.3 (zeromq communication policy) 
 * metis 5.1 (graph partitioning mapping)
 * [Boost Hana 1.0](https://github.com/ldionne/hana) (zeromq communication policy) 


##Installation##

###System Installion###

Installation into the operating system libery path e.g.
to `/usr/lib/graybat`:

    git clone https://github.com/ComputationalRadiationPhysics/graybat.git
    cd graybat && mkdir build && cd build
	cmake -DCMAKE_INSTALL_DIR=/usr ..
	sudo make install
	
###Package Install###

* Graybat AUR package

##Usage as Library##



###CMAKE-Configfile###
Graybat is a header only library so nothing has to be build.  The most
easy way to include graybat into your application is to use the shiped
[CMAKE-Configfile](https://cmake.org/cmake/help/v3.4/manual/cmake-packages.7.html#config-file-packages),
that can be used if your project is built with CMake.  If graybat was
not installed to a path where CMake usually has a look then the path
to graybat need to be set, otherwise the path to graybat will be
obtained automatically.

     set(graybat_DIR <ABSOLUT-PATH-TO-GRAYBAT-LIB>)

The CMAKE-Configfile of graybat provides the CMAKE variables `${graybat_INCLUDE_DIRS}` and
`${graybat_LIBRARIES}`, which can be used to include graybat headers and to link against
graybat depdendencies.

     find_package(graybat REQUIRED CONFIG)
     include_directories(SYSTEM ${graybat_INCLUDE_DIRS})
     set(LIBS ${LIBS} ${graybat_LIBRARIES})

Finally, the application can use graybat e.g. `#include <graybat/Cage.hpp>`.


##Compiling Tests/Examples##

 * Clone the repository: `git clone https://github.com/computationalradiationphysics/graybat.git`
 * Change directory: `cd graybat`
 * Create the build directory: `mkdir -p build`
 * Change to build directory: `cd build`
 * Set compiler: `export CXX=[g++,clang++]`
 * Create Makefile `cmake ..`
 * Build project : `make [target]`


##Tested Compilers##

 * clang 3.5
 * g++ 5.2.0


### Current Compilation Status:

| *branch* | *state* | *description* |
| -------- | --------| ------------- |
| **master** | [![Build Status](http://haseongpu.mooo.com/api/badge/github.com/erikzenker/GrayBat/status.svg?branch=master)](http://haseongpu.mooo.com/github.com/erikzenker/GrayBat) |  stable releases |
| **dev**  | [![Build Status](http://haseongpu.mooo.com/api/badge/github.com/erikzenker/GrayBat/status.svg?branch=dev)](http://haseongpu.mooo.com/github.com/erikzenker/GrayBat) |development branch |


##Predefined Targets##

 * **example**: All example applications.

 * **test** : Build, unit and integration test.

 * **doc**: Build documentation in `doc/`.

 * **clean**: Cleanup build directory.


##Project Organization##

The project is organized in a couple of subdirectories.

 * The [example](example) directory contains examples produced during development of graybat.
 * The [include](include) directory contains the library itself, which is header only.
 * The [test](test) directory contains unit and integration tests (might me used as examples)
 * The [utils](utils) directory contains cmake modules and doxygen files.


##Related Material##
 * Talk by Erik Zenker of his diploma defence [![DOI](https://zenodo.org/badge/doi/10.5281/zenodo.16306.svg)](http://dx.doi.org/10.5281/zenodo.16306)


##Authors##

 * Erik Zenker (erikzenker@posteo.de)

