#
# Copyright 2016 Erik Zenker
#
# This file is part of Graybat.
#
# Graybat is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# Graybat is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with Graybat.
# If not, see <http://www.gnu.org/licenses/>.
#

# - Config file for the graybat package
# It defines the following variables
#  graybat_INCLUDE_DIRS - include directories for FooBar
#  graybat_LIBRARIES    - libraries to link against

###############################################################################
# graybat
###############################################################################
cmake_minimum_required(VERSION 3.3.0)
project("graybat")

set(graybat_INCLUDE_DIRS ${graybat_INCLUDE_DIRS} "${graybat_DIR}/include")


###############################################################################
# COMPILER FLAGS
###############################################################################
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")


###############################################################################
# MODULES
###############################################################################
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${graybat_DIR}/include/graybat/utils/cmake/modules/" )


###############################################################################
# DEPENDENCIES
###############################################################################

###############################################################################
# Conan 
# - Resolves dependencies of ZMQ
###############################################################################
if(EXISTS "${CMAKE_BINARY_DIR}/conanbuildinfo.cmake")
  include("${CMAKE_BINARY_DIR}/conanbuildinfo.cmake")
  conan_basic_setup()
endif()  


###############################################################################
# METIS LIB
###############################################################################
find_package(METIS MODULE 5.1.0)
if(METIS_FOUND)
  set(graybat_INCLUDE_DIRS ${graybat_INCLUDE_DIRS} ${METIS_INCLUDE_DIRS})
  set(graybat_LIBRARIES ${graybat_LIBRARIES} ${METIS_LIBRARIES})
else()
  message(WARNING "METIS not found (not necessary).")
endif()


###############################################################################
# ZMQ LIB
###############################################################################
find_package(ZMQ MODULE 4.0.0)
if(ZMQ_FOUND)
  set(graybat_INCLUDE_DIRS ${graybat_INCLUDE_DIRS} ${ZMQ_INCLUDE_DIRS})
  set(graybat_LIBRARIES ${graybat_LIBRARIES} ${ZMQ_LIBRARIES})
else()
  message(FATAL_ERROR "ZMQ not found.")
endif()
  

###############################################################################
# Boost LIB
###############################################################################
find_package(Boost 1.56.0 MODULE COMPONENTS mpi serialization REQUIRED)
if(Boost_FOUND)
  set(graybat_INCLUDE_DIRS ${graybat_INCLUDE_DIRS} ${Boost_INCLUDE_DIRS})
  set(graybat_LIBRARIES ${graybat_LIBRARIES} ${Boost_LIBRARIES})
else()
  message(FATAL_ERROR "Boost not found.")
endif()


################################################################################
# MPI LIB
################################################################################
find_package(MPI MODULE)
if(MPI_FOUND)
  set(graybat_INCLUDE_DIRS ${graybat_INCLUDE_DIRS} ${MPI_C_INCLUDE_PATH})
  set(graybat_LIBRARIES ${graybat_LIBRARIES} ${MPI_C_LIBRARIES})
  set(graybat_LIBRARIES ${graybat_LIBRARIES} ${MPI_CXX_LIBRARIES})
else()
  message(FATAL_ERROR "MPI not found.")
endif()


################################################################################
# Find PThreads
################################################################################
find_package(Threads MODULE)
if(Threads_FOUND)
  set(graybat_LIBRARIES ${graybat_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
else()
  message(FATAL_ERROR "Threads not found.")
endif()
