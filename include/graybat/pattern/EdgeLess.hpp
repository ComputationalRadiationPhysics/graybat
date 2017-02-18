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

// STL
#include <utility> /* std::make_pair */

// GRAYBAT
#include <graybat/graphPolicy/Traits.hpp>

namespace graybat {

namespace pattern {

template <typename T_GraphPolicy> struct EdgeLess {

    using GraphPolicy = T_GraphPolicy;
    using VertexDescription = graybat::graphPolicy::VertexDescription<GraphPolicy>;
    using EdgeDescription = graybat::graphPolicy::EdgeDescription<GraphPolicy>;
    using GraphDescription = graybat::graphPolicy::GraphDescription<GraphPolicy>;

    const unsigned verticesCount;

    EdgeLess(const unsigned verticesCount)
        : verticesCount(verticesCount)
    {
    }

    GraphDescription operator()()
    {
        std::vector<VertexDescription> vertices(verticesCount);
        assert(vertices.size() == verticesCount);
        std::vector<EdgeDescription> edges;
        return std::make_pair(vertices, edges);
    }
};

} /* pattern */

} /* graybat */
