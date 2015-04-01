project(genericCommunicator)
cmake_minimum_required(VERSION 2.8.5)


###############################################################################
# Boost
###############################################################################
find_package(Boost 1.46.0 COMPONENTS mpi REQUIRED)
include_directories(SYSTEM ${Boost_INCLUDE_DIRS})
set(LIBS ${LIBS} ${Boost_LIBRARIES})

################################################################################
# Find MPI
################################################################################
find_package(MPI REQUIRED)
  include_directories(SYSTEM ${MPI_C_INCLUDE_PATH})
set(LIBS ${LIBS} ${MPI_C_LIBRARIES})

# bullxmpi fails if it can not find its c++ counter part
if(MPI_CXX_FOUND)
  set(LIBS ${LIBS} ${MPI_CXX_LIBRARIES})
endif(MPI_CXX_FOUND)

###############################################################################
# Compiler Flags
###############################################################################
#set(${TEST})
string(FIND ${CMAKE_CXX_COMPILER} "clang++" CXX_COMPILER_IS_CLANG++)
string(FIND ${CMAKE_CXX_COMPILER} "clang++" CXX_COMPILER_IS_G++)

#if(${CLANG})
	#message("Clang++")
#endif(${CLANG})


if(${CXX_COMPILER_IS_G++})
  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wextra")
  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")
  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif(${CXX_COMPILER_IS_G++})

#option(USE_CLANG "Use clang for compilation" OFF)
#if(${CMAKE_CXX_COMPILER} STREQUAL "/usr/bin/clang++")
#  set (USE_CLANG "ON")
#endif(${CMAKE_CXX_COMPILER} STREQUAL "/usr/bin/clang++")

#if(${CMAKE_CXX_COMPILER} STREQUAL "/usr/sbin/clang++")
#  set (USE_CLANG "ON")
#endif(${CMAKE_CXX_COMPILER} STREQUAL "/usr/sbin/clang++")  

if(${CXX_COMPILER_IS_CLANG++})
  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")
endif(${CXX_COMPILER_IS_CLANG++})

###############################################################################
# Build executables
###############################################################################
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include 
		   ${CMAKE_CURRENT_SOURCE_DIR}/include/dout/include )
#add_executable(Main EXCLUDE_FROM_ALL src/main.cc)
add_executable(GoL EXCLUDE_FROM_ALL src/gol.cc )
add_executable(GoLBench EXCLUDE_FROM_ALL src/golBench.cc )
add_executable(GoLFlex EXCLUDE_FROM_ALL src/golFlex.cc )
add_executable(NBody EXCLUDE_FROM_ALL src/nbody.cc )
add_executable(NBodyBench EXCLUDE_FROM_ALL src/nbodyBench.cc )
add_executable(Synthetic EXCLUDE_FROM_ALL src/synthetic.cc )
add_executable(SyntheticOneLookup EXCLUDE_FROM_ALL src/synthetic_one_lookup.cc )
add_executable(SyntheticRandom EXCLUDE_FROM_ALL src/synthetic_random.cc )
add_executable(Gather EXCLUDE_FROM_ALL src/gather.cc )
add_executable(Reduce EXCLUDE_FROM_ALL src/reduce.cc )
add_executable(UnitCAL EXCLUDE_FROM_ALL src/unit_cal.cc )
add_executable(UnitGVON EXCLUDE_FROM_ALL src/unit_gvon.cc )

# add the libraries (Boost...) to this executable
#target_link_libraries(Main ${LIBS})
target_link_libraries(GoL ${LIBS})
target_link_libraries(GoLBench ${LIBS})
target_link_libraries(GoLFlex ${LIBS})
target_link_libraries(NBody ${LIBS})
target_link_libraries(NBodyBench ${LIBS})
target_link_libraries(Synthetic ${LIBS})
target_link_libraries(SyntheticOneLookup ${LIBS})
target_link_libraries(SyntheticRandom ${LIBS})
target_link_libraries(Gather ${LIBS})
target_link_libraries(Reduce ${LIBS})
target_link_libraries(UnitCAL ${LIBS})
target_link_libraries(UnitGVON ${LIBS})

###############################################################################
# Custom targets
###############################################################################

# doc
find_package(Doxygen)
if(DOXYGEN_FOUND)
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/doxygen.conf ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)
  add_custom_target(doc 
    COMMAND ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
    SOURCES ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Generating API documentation with Doxygen" VERBATIM
    )
endif(DOXYGEN_FOUND)
