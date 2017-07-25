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
#  graybat_DEFINTIONS      - compile definitions provided by graybat
#                            (graybat_ZMQ_CP_ENABLED, graybat_BMPI_CP_ENABLED)

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
if(METIS_FOUND)
  set(graybat_INCLUDE_DIRS ${graybat_INCLUDE_DIRS} ${METIS_INCLUDE_DIRS})
  set(graybat_LIBRARIES ${graybat_LIBRARIES} ${METIS_LIBRARIES})
endif(METIS_FOUND)

###############################################################################
# Boost LIB
###############################################################################
find_package(Boost 1.56.0 COMPONENTS mpi)
if(Boost_MPI_FOUND)
    set(graybat_INCLUDE_DIRS ${graybat_INCLUDE_DIRS} ${Boost_INCLUDE_DIRS})
    set(graybat_LIBRARIES ${graybat_LIBRARIES} ${Boost_LIBRARIES})
endif()

find_package(Boost 1.56.0 REQUIRED system serialization)
if(Boost_SYSTEM_FOUND AND Boost_SERIALIZATION_FOUND)
    set(graybat_INCLUDE_DIRS ${graybat_INCLUDE_DIRS} ${Boost_INCLUDE_DIRS})
    set(graybat_LIBRARIES ${graybat_LIBRARIES} ${Boost_LIBRARIES})
endif()

################################################################################
# MPI LIB
################################################################################
find_package(MPI MODULE)
if(MPI_FOUND)
    set(graybat_INCLUDE_DIRS ${graybat_INCLUDE_DIRS} ${MPI_C_INCLUDE_PATH})
    set(graybat_LIBRARIES ${graybat_LIBRARIES} ${MPI_C_LIBRARIES})
    set(graybat_LIBRARIES ${graybat_LIBRARIES} ${MPI_CXX_LIBRARIES})
endif()

################################################################################
# Find PThreads
################################################################################
find_package(Threads MODULE)
set(graybat_LIBRARIES ${graybat_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})

###############################################################################
# ZMQ LIB
###############################################################################
find_package(ZMQ MODULE 4.0.0)
if(ZMQ_FOUND)
    set(graybat_INCLUDE_DIRS ${graybat_INCLUDE_DIRS} ${ZMQ_INCLUDE_DIRS})
    set(graybat_LIBRARIES ${graybat_LIBRARIES} ${ZMQ_LIBRARIES})
endif(ZMQ_FOUND)

################################################################################
# Find Protobuf
################################################################################
find_package(Protobuf)
if(Protobuf_FOUND)
    set(graybat_INCLUDE_DIRS ${graybat_INCLUDE_DIRS} ${PROTOBUF_INCLUDE_DIRS})
    set(graybat_LIBRARIES ${graybat_LIBRARIES} ${PROTOBUF_LIBRARIES})
endif(Protobuf_FOUND)

################################################################################
# Find GRPC
################################################################################
find_package(GRPC)
if(GRPC_FOUND)
    set(graybat_INCLUDE_DIRS ${graybat_INCLUDE_DIRS} ${GRPC_INCLUDE_DIRS})
    set(graybat_LIBRARIES ${graybat_LIBRARIES} ${GRPC_LIBRARIES})
endif(GRPC_FOUND)


###############################################################################
# Generate files
###############################################################################
if(Protobuf_FOUND AND GRPC_FOUND)
    set(graybat_INCLUDE_DIRS ${graybat_INCLUDE_DIRS} ${CMAKE_CURRENT_BINARY_DIR})
    file(GLOB ProtoFiles "${graybat_DIR}/include/graybat/utils/protos/*.proto")
    PROTOBUF_GENERATE_CPP(PROTO_SRCS PROTO_HDRS ${ProtoFiles})
    PROTOBUF_GENERATE_GRPC_CPP(GRPC_SRCS GRPC_HDRS ${ProtoFiles})
    set(graybat_GENERATED_FILES ${GRPC_SRCS} ${PROTO_SRCS})
endif()

###############################################################################
# Set Compile Definitions
###############################################################################
if(MPI_FOUND AND Boost_MPI_FOUND)
    LIST(APPEND graybat_DEFINITIONS "graybat_BMPI_CP_ENABLED")
    SET(graybat_BMPI_CP_ENABLED TRUE)
endif()

if(ZMQ_FOUND AND GRPC_FOUND AND Protobuf_FOUND)
    LIST(APPEND graybat_DEFINITIONS "graybat_ZMQ_CP_ENABLED")
    SET(graybat_ZMQ_CP_ENABLED TRUE)
endif()

###############################################################################
# FindPackage options
###############################################################################
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(
    "graybat"
    FOUND_VAR graybat_FOUND
    REQUIRED_VARS graybat_INCLUDE_DIRS)