/**
 * Copyright 2016 Erik Zenker
 *
 * This file is part of Graybat.
 *
 * Graybat is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Graybat is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with Graybat.
 * If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <graybat/Cage.hpp>

// Graphpolicies
#include <graybat/graphPolicy/BGL.hpp>

// Serializationpolicies
#include <graybat/serializationPolicy/ByteCast.hpp>
#include <graybat/serializationPolicy/Forward.hpp>

// Communicationpolicies
#ifdef graybat_ZMQ_CP_ENABLED
  #include <graybat/communicationPolicy/ZMQ.hpp>
#endif

#ifdef graybat_BMPI_CP_ENABLED
  #include <graybat/communicationPolicy/BMPI.hpp>
#endif

// Mappings
#include <graybat/mapping/Consecutive.hpp>
#include <graybat/mapping/Filter.hpp>
#include <graybat/mapping/Random.hpp>
#include <graybat/mapping/Roundrobin.hpp>

// Pattern
#include <graybat/pattern/BiStar.hpp>
#include <graybat/pattern/Chain.hpp>
#include <graybat/pattern/EdgeLess.hpp>
#include <graybat/pattern/FullyConnected.hpp>
#include <graybat/pattern/Grid.hpp>
#include <graybat/pattern/GridDiagonal.hpp>
#include <graybat/pattern/HyperCube.hpp>
#include <graybat/pattern/InStar.hpp>
#include <graybat/pattern/None.hpp>
#include <graybat/pattern/OutStar.hpp>
#include <graybat/pattern/Random.hpp>
#include <graybat/pattern/Ring.hpp>

