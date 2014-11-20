#pragma once

#include <algorithm> /* std::min */
#include <vector>    /* std::vector */
#include <assert.h>  /* assert */

/*******************************************************************************
 *
 * VERTEX DISTRIBUTION
 *
 *******************************************************************************/

namespace Distribute {

    template<typename T_Graph>
    std::vector<typename T_Graph::Vertex> test(const unsigned processID, const unsigned processCount, T_Graph &graph){
	typedef typename T_Graph::Vertex Vertex;


	// Distribute and announce vertices
	std::vector<Vertex> myVertices(0);

	if(processID == 0){
	    myVertices = graph.getVertices();
	}
	
	return myVertices;
    }

    
    template<typename T_Graph>
    std::vector<typename T_Graph::Vertex> roundrobin(const unsigned processID, const unsigned processCount, T_Graph &graph){
	typedef typename T_Graph::Vertex Vertex;


	// Distribute and announce vertices
	unsigned vertexCount   = graph.getVertices().size();
	unsigned maxVertex     = ceil((float)vertexCount / processCount);

	std::vector<Vertex> myVertices;
	if(processID < processCount){
	    
	
	    for(unsigned i = 0; i < maxVertex; ++i){
		unsigned vertex_i = processID + (i * processCount);
		if(vertex_i >= vertexCount){
		    break;
		}
		else {
		    myVertices.push_back(graph.getVertices().at(vertex_i));
		}
	
	    }
	}
	return myVertices;
	
    }



    template<typename T_Graph>
    std::vector<typename T_Graph::Vertex> consecutive(const unsigned processID, const unsigned processCount, T_Graph &graph){

	typedef typename T_Graph::Vertex Vertex;

	unsigned vertexCount      = graph.getVertices().size();
	unsigned vertexPerProcess = ceil((float)vertexCount / processCount);

	// More processes than vertices
	if(processID > vertexCount - 1){
	    return std::vector<Vertex>(0);
	}

	unsigned minVertex = processID * vertexPerProcess;
	unsigned maxVertex = minVertex + vertexPerProcess;

	// Slice maxVertex of last process
	if(minVertex > vertexCount){
	    return std::vector<Vertex>(0);
	}
	    
	maxVertex = std::min(maxVertex, vertexCount);
	
	assert(minVertex <= maxVertex);
	
	std::vector<Vertex> vertices = graph.getVertices();
	std::vector<Vertex> myVertices(vertices.begin() + minVertex, vertices.begin() + maxVertex);
	return myVertices;

    }


} /* Distribute */
