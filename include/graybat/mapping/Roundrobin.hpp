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

#include <vector>    /* std::vector */

namespace graybat {

    namespace mapping {
	struct Roundrobin {
    
	    template<typename T_Graph>
	    std::vector<typename T_Graph::Vertex> operator()(const unsigned processID, const unsigned processCount, T_Graph &graph){
		typedef typename T_Graph::Vertex Vertex;


		// Distribute and announce vertices
		unsigned vertexCount   = graph.getVertices().size();
		unsigned maxVertex     = ceil((float)vertexCount / processCount);

		std::vector<Vertex> myVertices;
		if(processID <= vertexCount){
		    for(unsigned i = 0; i < maxVertex; ++i){
			unsigned vertex_i = processID + (i * processCount);
			if(vertex_i >= vertexCount){
			    break;
			}
			else {
			    myVertices.push_back(graph.getVertex(vertex_i));
			}
	
		    }
		}
		return myVertices;
	
	    }

	};

    } /* mapping */
    
} /* graybat */
