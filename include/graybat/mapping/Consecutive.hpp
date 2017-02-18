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

#include <algorithm> /* std::min */
#include <assert.h> /* assert */
#include <vector> /* std::vector */

namespace graybat {

namespace mapping {

struct Consecutive {

    template <typename T_Graph>
    std::vector<typename T_Graph::Vertex>
    operator()(const unsigned processID, const unsigned processCount, T_Graph& graph)
    {

        typedef typename T_Graph::Vertex Vertex;

        unsigned vertexCount = graph.getVertices().size();
        unsigned vertexPerProcess = ceil((float)vertexCount / processCount);

        // More processes than vertices
        if (processID > vertexCount - 1) {
            return std::vector<Vertex>();
        }

        unsigned minVertex = processID * vertexPerProcess;
        unsigned maxVertex = minVertex + vertexPerProcess;

        // Slice maxVertex of last process
        if (minVertex > vertexCount) {
            return std::vector<Vertex>();
        }

        maxVertex = std::min(maxVertex, vertexCount);

        assert(minVertex <= maxVertex);

        std::vector<Vertex> vertices = graph.getVertices();
        std::vector<Vertex> myVertices(vertices.begin() + minVertex, vertices.begin() + maxVertex);
        return myVertices;
    }
};

} /* mapping */

} /* graybat */
