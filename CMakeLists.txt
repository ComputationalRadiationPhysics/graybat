project(genericCommunicator)
cmake_minimum_required(VERSION 2.8.5)

###############################################################################
# Boost
###############################################################################
find_package(Boost 1.48.0 REQUIRED)
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
# DO not compile with -OX with x > 0, can lead to segfaults

if(CMAKE_COMPILER_IS_GNUCXX)
  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wextra")
  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0")
  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif(CMAKE_COMPILER_IS_GNUCXX)

###############################################################################
# Build executables
###############################################################################
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include 
		   ${CMAKE_CURRENT_SOURCE_DIR}/include/dout/include )
add_executable(Main src/main.cc)

# add the libraries (Boost...) to this executable
target_link_libraries(Main ${LIBS})

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

# update
add_custom_target(update
	COMMAND fish -c "update make ${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cc"
	COMMENT "Build project automatically on changes of source and header files")
