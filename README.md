

#GrayBat#

##Description##

GrayBat is a C++ library that presents the graph-based virtual overlay
network, a flexible description which enables a mapping of algorithms
to communication patterns and further a mapping of these communication
patterns onto the available hardware topologies. These mappings were
established as an intermediate layer between an application and
communication libraries. This approach provides a dynamic remapping at
run-time, which will allow to address load balancing and fault
tolerance for upcoming exascale systems.


##Referencing##

GrayBat is a scientific project. If you **present and/or publish** scientific
results that used GrayBat, you should set this as a **reference**.


##Softeware License##

GrayBat  is licensed under the **GPLv3+**. Please refer to our [LICENSE.md](LICENSE.md)


##Dependencies##

 * cmake 3.0.2
 * Boost 1.57.0
 * MPI 1.8.0
 * g++ 4.9.2
 * Doxygen 1.8.8



##Compiling##

 * mkdir build
 * cd build
 * cmake ..
 * make [target]

##Example Targets##

GrayBat is a library, but some examples are provided to demonstrate
the usage. All targets provide the synopthis of their executables.

 * GoL: Game of Life simulation

 * NBody: N-Body simulation

 * UnitCAL: CAL unit tests

 * UnitGVON: GVON unit tests

 * doc: Build documentation in ../doc/

 * clean: Cleanup build directory



##Authors##

 * Erik Zenker (erikzenker@hotmail.com)
