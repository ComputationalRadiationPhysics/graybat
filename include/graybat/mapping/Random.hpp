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
#include <stdlib.h>  /* srand, rand */

namespace graybat {

    namespace mapping {

	/**
	 * Random distribution of vertices of the *graph* to the
	 * the peers. All peers need to set the same random *seed*.
	 * Thus, all peers have the same random base. Therefore
	 * seeds that depend on varying parameters like time or
	 * pid or not applicable here.
	 *
	 * @param  seed static random seed for all peers
	 * @return random set of vertices of the *graph*
	 *
	 */
	struct Random {

	    Random(int seed)
		: seed(seed){

	    }
	  
	    template<typename T_Graph>
	    std::vector<typename T_Graph::Vertex> operator()(const unsigned processID, const unsigned processCount, T_Graph &graph){


		typedef typename T_Graph::Vertex Vertex;
		srand(seed);
		std::vector<Vertex> myVertices;
		unsigned vertexCount   = graph.getVertices().size();
		

		if(processID <= vertexCount){
		    for(Vertex v: graph.getVertices()){
			unsigned randomID = rand() % processCount;
			if(randomID == processID){
			    myVertices.push_back(v);
			}

		    }
		}
	      

		return myVertices;
	    }
	  
	private:
	    int seed;

	};

    } /* mapping */
    
} /* graybat */
