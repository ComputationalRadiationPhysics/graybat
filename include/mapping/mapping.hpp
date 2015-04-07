#pragma once

#include <algorithm> /* std::min */
#include <vector>    /* std::vector */
#include <assert.h>  /* assert */
#include <stdlib.h>  /* srand, rand */

/*******************************************************************************
 *
 * VERTEX DISTRIBUTION
 *
 *******************************************************************************/

namespace graybat {

    namespace mapping {


	/**
	 * Partitioning of the communication graph
	 * into k parts. k is set either to the 
	 * number of peers that want to take
	 * part in communication or is given
	 * as an input parameter.
	 *
	 * @param  nParts number of parts to partition
	 * @return set of vertices that belong "together"
	 */
	struct GraphPartition {

	    GraphPartition() : GraphPartition(0){

	    }
	    
	    GraphPartition(unsigned nParts)
		: nParts(nParts){

	    }

	    /**
	     * @Brief Translates the graph into the compressed row storage
	     *        format (CSR) which can be parsed by Metis.
	     *
	     * @See http://en.wikipedia.org/wiki/Sparse_matrix#Compressed_row_Storage_.28CRS_or_CSR.29
	     *
	     */
	    template<typename T_Graph>
	    std::pair<std::vector<idx_t>, std::vector<idx_t> > toCompressedRowStorage(T_Graph &graph) {
		
		typedef typename T_Graph::Vertex Vertex;
		typedef typename T_Graph::Edge   Edge;

		unsigned i = 0;

		std::vector<idx_t> xadj(1,i);
		std::vector<idx_t> adjncy;

		
		for(Vertex v : graph.getVertices()){
		    for(auto link : graph.getOutEdges(v)){
			Vertex destVertex = link.first;
			Edge   destEdge   = link.second;

			adjncy.push_back(destVertex.id);
			i++;
			
		    }
		    xadj.push_back(i);

		}
		
		return std::make_pair(xadj, adjncy);
	    }

	    template<typename T_Graph>
	    std::vector<typename T_Graph::Vertex> operator()(const unsigned processID, const unsigned processCount, T_Graph &graph){

		typedef typename T_Graph::Vertex Vertex;
		std::vector<Vertex> myVertices;
		auto csr = toCompressedRowStorage(graph);

		if(nParts == 0){
		    nParts = processCount;
		}

		if(nParts == 1){
		    return graph.getVertices();
		}
		
		idx_t nVertices = graph.getVertices().size();
		idx_t nWeights  = 1;
		idx_t objval;
		std::vector<idx_t> part(nVertices, 0);
		
		METIS_PartGraphKway(&nVertices, &nWeights,
				    csr.first.data(), csr.second.data(),
				    NULL, NULL, NULL, &nParts, NULL,
				    NULL, NULL,
				    &objval,
				    part.data());

		
		for(unsigned part_i = 0; part_i < part.size(); part_i++){
		    if(part[part_i] == (int)processID){
			myVertices.push_back(graph.getVertices().at(part_i));
		    }
		    
		} 
		
		return myVertices;
	    }

	private:
	    idx_t nParts;

	};

	

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
	      
		for(Vertex v: graph.getVertices()){
		    unsigned randomID = rand() % processCount;
		    if(randomID == processID){
			myVertices.push_back(v);
		    }

		}
	      

		return myVertices;
	    }
	  
	private:
	    int seed;

	};
      

      
	struct Roundrobin {
    
	    template<typename T_Graph>
	    std::vector<typename T_Graph::Vertex> operator()(const unsigned processID, const unsigned processCount, T_Graph &graph){
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

	};

	struct Consecutive {

	    template<typename T_Graph>
	    std::vector<typename T_Graph::Vertex> operator()(const unsigned processID, const unsigned processCount, T_Graph &graph){

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

	};

    } /* mapping */
    
} /* graybat */
