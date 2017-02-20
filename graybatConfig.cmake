#
# Copyright 2016-2017 Erik Zenker
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
#  graybat_INCLUDE_DIRS    - include directories for Graybat
#  graybat_LIBRARIES       - libraries to link against
#  graybat_GENERATED_FILES - files to add to targets using Graybat

###############################################################################
# graybat
###############################################################################
cmake_minimum_required(VERSION 3.3.0)
project("graybat")

set(graybat_INCLUDE_DIRS ${graybat_INCLUDE_DIRS} "${graybat_DIR}/include")


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
find_package(Boost 1.56.0 MODULE COMPONENTS mpi serialization system REQUIRED)
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

################################################################################
# Find Protobuf
################################################################################
find_package(Protobuf REQUIRED)
set(graybat_INCLUDE_DIRS ${graybat_INCLUDE_DIRS} ${PROTOBUF_INCLUDE_DIRS})
set(graybat_LIBRARIES ${graybat_LIBRARIES} ${PROTOBUF_LIBRARIES})

################################################################################
# Find GRPC
################################################################################
find_package(GRPC REQUIRED)
set(graybat_INCLUDE_DIRS ${graybat_INCLUDE_DIRS} ${GRPC_INCLUDE_DIRS})
set(graybat_LIBRARIES ${graybat_LIBRARIES} ${GRPC_LIBRARIES})

###############################################################################
# Generate files
###############################################################################
set(graybat_INCLUDE_DIRS ${graybat_INCLUDE_DIRS} ${CMAKE_CURRENT_BINARY_DIR})
file(GLOB ProtoFiles "include/graybat/utils/protos/*.proto")
PROTOBUF_GENERATE_CPP(PROTO_SRCS PROTO_HDRS ${ProtoFiles})
PROTOBUF_GENERATE_GRPC_CPP(GRPC_SRCS GRPC_HDRS ${ProtoFiles})
set(graybat_GENERATED_FILES ${GRPC_SRCS} ${PROTO_SRCS})
