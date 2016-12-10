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
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")


###############################################################################
# MODULES
###############################################################################
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${graybat_DIR}/include/graybat/utils/cmake/modules/" )

###############################################################################
# DEPENDENCIES
###############################################################################

###############################################################################
# METIS LIB
###############################################################################
find_package(METIS MODULE 5.1.0)
set(graybat_INCLUDE_DIRS ${graybat_INCLUDE_DIRS} ${METIS_INCLUDE_DIRS})
set(graybat_LIBRARIES ${graybat_LIBRARIES} ${METIS_LIBRARIES})

###############################################################################
# ZMQ LIB
###############################################################################
find_package(ZMQ MODULE 4.0.0)
set(graybat_INCLUDE_DIRS ${graybat_INCLUDE_DIRS} ${ZMQ_INCLUDE_DIRS})
set(graybat_LIBRARIES ${graybat_LIBRARIES} ${ZMQ_LIBRARIES})

###############################################################################
# Boost LIB
###############################################################################
find_package(Boost 1.56.0 MODULE COMPONENTS mpi serialization REQUIRED)
set(graybat_INCLUDE_DIRS ${graybat_INCLUDE_DIRS} ${Boost_INCLUDE_DIRS})
set(graybat_LIBRARIES ${graybat_LIBRARIES} ${Boost_LIBRARIES})

################################################################################
# MPI LIB
################################################################################
find_package(MPI MODULE)
set(graybat_INCLUDE_DIRS ${graybat_INCLUDE_DIRS} ${MPI_C_INCLUDE_PATH})
set(graybat_LIBRARIES ${graybat_LIBRARIES} ${MPI_C_LIBRARIES})
set(graybat_LIBRARIES ${graybat_LIBRARIES} ${MPI_CXX_LIBRARIES})

################################################################################
# Find PThreads
################################################################################
find_package(Threads MODULE)
set(graybat_LIBRARIES ${graybat_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
