project(genericCommunicator)
cmake_minimum_required(VERSION 2.8.5)


###############################################################################
# CMAKE module path in utils/cmake/modules/
###############################################################################
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/utils/cmake/modules/" ${CMAKE_MODULE_PATH})

################################################################################
# Find PThreads
################################################################################
find_package(Threads REQUIRED)
set(LIBS ${LIBS} ${CMAKE_THREAD_LIBS_INIT})

###############################################################################
# METIS
###############################################################################
find_package(METIS 5.1.0)
include_directories(SYSTEM ${METIS_INCLUDE_DIRS})
set(LIBS ${LIBS} ${METIS_LIBRARIES})

###############################################################################
# ZMQ
###############################################################################
find_package(ZMQ 4.0.0)
include_directories(SYSTEM ${ZMQ_INCLUDE_DIRS})
set(LIBS ${LIBS} ${ZMQ_LIBRARIES})

###############################################################################
# Boost
###############################################################################
find_package(Boost 1.58.0 COMPONENTS mpi unit_test_framework serialization REQUIRED)
include_directories(SYSTEM ${Boost_INCLUDE_DIRS})
set(LIBS ${LIBS} ${Boost_LIBRARIES})

################################################################################
# MPI
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
string(FIND ${CMAKE_CXX_COMPILER} "clang++" CXX_COMPILER_IS_CLANG++)
string(FIND ${CMAKE_CXX_COMPILER} "g++" CXX_COMPILER_IS_G++)

if(${CXX_COMPILER_IS_G++})
  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wextra")
  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")
  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")
  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")
endif(${CXX_COMPILER_IS_G++})

if(${CXX_COMPILER_IS_CLANG++})
  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")
  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")
endif(${CXX_COMPILER_IS_CLANG++})

###############################################################################
# Build executables
###############################################################################
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include/)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/utils/hana/include/)

# Tests
file(GLOB TESTS test/*.cpp)
add_executable(check EXCLUDE_FROM_ALL ${TESTS})
target_link_libraries(check ${LIBS})

enable_testing()
add_test(graybat_test_build "${CMAKE_COMMAND}" --build ${CMAKE_BINARY_DIR} --target check)
add_test(graybat_test_run mpiexec -n 2 check --run_test=cage_point_to_point)
SET_TESTS_PROPERTIES (graybat_test_run PROPERTIES DEPENDS graybat_test_build)

# Examples
add_custom_target(example)
file(GLOB EXAMPLES  example/*.cpp)
foreach(EXAMPLE ${EXAMPLES})
	get_filename_component(DEP ${EXAMPLE} NAME_WE)
	add_executable(${DEP} EXCLUDE_FROM_ALL ${EXAMPLE})
	target_link_libraries(${DEP} ${LIBS})	
	add_dependencies(example ${DEP})
endforeach(EXAMPLE)

# ZMQ signaling server
add_executable(zmq_signaling_server EXCLUDE_FROM_ALL utils/zmq_signaling_server.cc )
target_link_libraries(zmq_signaling_server ${LIBS})

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
