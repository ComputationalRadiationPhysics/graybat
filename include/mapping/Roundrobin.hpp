#pragma once

#include <vector>    /* std::vector */

namespace graybat {

    namespace mapping {
	struct Roundrobin {
    
	    template<typename T_Cage>
	    std::vector<typename T_Cage::Vertex> operator()(const unsigned processID, const unsigned processCount, T_Cage &cage){
		typedef typename T_Cage::Vertex Vertex;


		// Distribute and announce vertices
		unsigned vertexCount   = cage.getVertices().size();
		unsigned maxVertex     = ceil((float)vertexCount / processCount);

		std::vector<Vertex> myVertices;
		if(processID <= vertexCount){
		    for(unsigned i = 0; i < maxVertex; ++i){
			unsigned vertex_i = processID + (i * processCount);
			if(vertex_i >= vertexCount){
			    break;
			}
			else {
			    myVertices.push_back(cage.getVertices().at(vertex_i));
			}
	
		    }
		}
		return myVertices;
	
	    }

	};

    } /* mapping */
    
} /* graybat */
