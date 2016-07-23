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

file(STRINGS "${CMAKE_CURRENT_LIST_DIR}/include/graybat/version.hpp" graybat_VERSION_MAJOR_HPP REGEX "#define graybat_VERSION_MAJOR ")
file(STRINGS "${CMAKE_CURRENT_LIST_DIR}/include/graybat/version.hpp" graybat_VERSION_MINOR_HPP REGEX "#define graybat_VERSION_MINOR ")
file(STRINGS "${CMAKE_CURRENT_LIST_DIR}/include/graybat/version.hpp" graybat_VERSION_PATCH_HPP REGEX "#define graybat_VERSION_PATCH ")

string(REGEX MATCH "([0-9]+)" graybat_VERSION_MAJOR  ${graybat_VERSION_MAJOR_HPP})
string(REGEX MATCH "([0-9]+)" graybat_VERSION_MINOR  ${graybat_VERSION_MINOR_HPP})
string(REGEX MATCH "([0-9]+)" graybat_VERSION_PATCH  ${graybat_VERSION_PATCH_HPP})

set(PACKAGE_VERSION "${graybat_VERSION_MAJOR}.${graybat_VERSION_MINOR}.${graybat_VERSION_PATCH}")

# Check whether the requested PACKAGE_FIND_VERSION is exactly the one requested
if("${PACKAGE_VERSION}" EQUAL "${PACKAGE_FIND_VERSION}")
    set(PACKAGE_VERSION_EXACT TRUE)
else()
    set(PACKAGE_VERSION_EXACT FALSE)
endif()

# Check whether the requested PACKAGE_FIND_VERSION is compatible
if("${PACKAGE_VERSION}" VERSION_LESS "${PACKAGE_FIND_VERSION}")
    set(PACKAGE_VERSION_COMPATIBLE FALSE)
else()
    set(PACKAGE_VERSION_COMPATIBLE TRUE)
    if ("${PACKAGE_VERSION}" VERSION_EQUAL "${PACKAGE_FIND_VERSION}")
        set(PACKAGE_VERSION_EXACT TRUE)
    endif()
endif()
