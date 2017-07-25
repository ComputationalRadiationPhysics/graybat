Graybat
=======

<b>Gr</b>aph <b>A</b>pproach  for Highl<b>y</b>  Generic Communication
Schemes <b>B</b>ased on <b>A</b>daptive <b>T</b>opologies


##Descriptio

**Graybat** is a C++ library that presents a graph-based communication
approach, which enables a mapping of algorithms to communication
patterns and further a mapping of these communication patterns to
varying hardware architectures. Therefore, a flexible and configurable
communication approach for parallel and distributed
applications. These mappings are established as an intermediate layer
between an application and communication libraries and are dynamically
adptable during run-time.

The graybat API is currently unstable.


##Documentation

Have a look at the documentation that is available [here](https://ComputationalRadiationPhysics.github.io/graybat) or
skim through the [examples](example) or [test cases](test) to have a quick start into graybat.


##Referencing

Graybat is a scientific project. If you **present and/or publish** scientific
results that used graybat, you should set this as a **reference**.

##Software License

Graybat is licensed under the <b>LGPLv3+</b>. Please refer to our [LICENSE.md](LICENSE.md)


##Project Organization

The project is organized in a couple of subdirectories.

 * The [example](example) directory contains examples produced during development of graybat.
 * The [include](include) directory contains the library itself, which is header only.
 * The [test](test) directory contains unit and integration tests (might be used as examples)
 * The [utils](utils) directory contains cmake modules and doxygen files.


##Dependencies

###Mandatory
 * cmake 3.0.2
 * Boost 1.61.0
 * g++ 6.0.0 or clang 3.5
 * c++14

###Optional
 * OpenMPI 1.8.0 (mpi communication policy)
 * zeromq 4.1.3 (zeromq communication policy) 
 * metis 5.1 (graph partitioning mapping)


##Roadmap
 * [Dynamic context](https://github.com/ComputationalRadiationPhysics/graybat/milestones/Dynamic%20Context)
 * [Boost::Asio communication policy](https://github.com/ComputationalRadiationPhysics/graybat/milestones/Boost::Asio%20Communication%20Policy)
 * [C++11 thread communication policy](https://github.com/ComputationalRadiationPhysics/graybat/milestones/C++11%20Threads%20Communication%20Policy)


##Installation

###System Installion

Installation into the operating system libery path e.g.
to `/usr/lib/graybat`:

    git clone https://github.com/ComputationalRadiationPhysics/graybat.git
    cd graybat && mkdir build && cd build
	cmake -DCMAKE_INSTALL_DIR=/usr ..
	sudo make install
	
###Package Install

* Graybat [AUR package](https://aur.archlinux.org/packages/graybat-git/)

##Usage as Library


###CMAKE-Configfile
Graybat is a header only library so nothing has to be build.  The most
easy way to include graybat into your application is to use the shiped
[CMAKE-Configfile](https://cmake.org/cmake/help/v3.4/manual/cmake-packages.7.html#config-file-packages),
that can be used if your project is built with CMake.  If graybat was
not installed to a path where CMake usually has a look then you can
instruct CMake to find graybat by adding the absolute path of graybat
to your `CMAKE_PREFIX_PATH`, by setting the path within CMake `find_package()`:

    find_package(graybat 1.0.0 REQUIRED CONFIG PATHS <ABSOLUTE-PATH-TO-GRAYBAT>)

or by setting the `graybat_DIR`:

    set(graybat_DIR <ABSOLUTE-PATH-TO-GRAYBAT>)


The CMAKE-Configfile of graybat provides the CMAKE variables `${graybat_INCLUDE_DIRS}`,
`${graybat_LIBRARIES}`, `${graybat_GENERATED_FILES}`, and `${graybat_DEFINITIONS}`. Where `${graybat_INCLUDE_DIRS}` contains all header files
included by graybat,`${graybat_LIBRARIES}` contains all libraries used by graybat, `${graybat_GENERATED_FILES}`
contains all generated files that need to be compiled with the target, and `${graybat_DEFINITIONS}` contains compile time definitions which can be
used to toggle policies.
The following is a an example of how to embed graybat into your `CMakeLists.txt`

     find_package(graybat REQUIRED CONFIG)
     include_directories(SYSTEM ${graybat_INCLUDE_DIRS})
     set(LIBS ${LIBS} ${graybat_LIBRARIES})
  
     add_executable(myTarget main.cpp ${graybat_GENERATED_FILES})
     target_compile_definitions(myTarget PRIVATE ${graybat_DEFINITIONS})
     target_link_libraries(signaling ${LIBS})

Finally, the application can use graybat e.g. `#include <graybat/graybat.hpp>`.


##Compiling Tests/Examples

 * Clone the repository: `git clone https://github.com/computationalradiationphysics/graybat.git`
 * Change directory: `cd graybat`
 * Create the build directory: `mkdir -p build`
 * Change to build directory: `cd build`
 * Set compiler: `export CXX=[g++,clang++]`
 * Create Makefile `cmake ..`
 * Build project : `make [target]`

##Benchmarks
There exist benchmarks for graybat:
```
make benchmark
./signaling_server&
mpiexec -n 1 ./benchmark

Run on (4 X 2493.8 MHz CPU s)
2016-11-14 19:35:09
Benchmark                                       Time           CPU Iterations
-----------------------------------------------------------------------------
meassureSingleMessageSendBmpi/1             25316 ns      25311 ns      27279
meassureSingleMessageSendBmpi/10            25822 ns      25823 ns      27864
meassureSingleMessageSendBmpi/100           30467 ns      30230 ns      24802
meassureSingleMessageSendBmpi/1000          50411 ns      50008 ns      14374
meassureSingleMessageSendBmpi/9.76562k     247629 ns     247566 ns       2933
meassureSingleMessageSendBmpi/97.6562k    2113853 ns    2113794 ns        329
meassureSingleMessageSendBmpi/976.562k   23066718 ns   22867737 ns         31
meassureSingleMessageSendZmq/1             348564 ns     176029 ns       3992
meassureSingleMessageSendZmq/10            549071 ns     184522 ns       3930
meassureSingleMessageSendZmq/100           363852 ns     188720 ns       3640
meassureSingleMessageSendZmq/1000          418714 ns     209411 ns       3220
meassureSingleMessageSendZmq/9.76562k      606935 ns     394426 ns       1776
meassureSingleMessageSendZmq/97.6562k     2511339 ns    2256736 ns        307
meassureSingleMessageSendZmq/976.562k    23033162 ns   22561309 ns         31

```


##Predefined Targets

 * **example**: All example applications.

 * **test** : Build, unit and integration test.

 * **benchmark** : Benchmarks

 * **signaling** : Signaling server for zeroMQ communication policy.

 * **doc**: Build documentation in `doc/`.

 * **clean**: Cleanup build directory.


##Tested Compilers

 * clang 3.5
 * g++ 5.2.0


##Related Material
 * Talk by Erik Zenker of his diploma defence [![DOI](https://zenodo.org/badge/doi/10.5281/zenodo.16306.svg)](http://dx.doi.org/10.5281/zenodo.16306)


##Authors

 * Erik Zenker (erikzenker@posteo.de)

