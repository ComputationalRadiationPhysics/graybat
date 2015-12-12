#pragma once

//STL
#include <vector>
#include <iostream>
#include <tuple>
#include <tuple>
#include <string>
#include <fstream> /* std::fstream */
#include <utility> /* std::pair, std::make_pair */
#include <memory>  /* std::unique_ptr */

// BOOSt
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/subgraph.hpp>
#include <boost/graph/graphviz.hpp>
#include <boost/bimap.hpp>


namespace graybat {
    
    namespace graphPolicy {
	
	struct SimpleProperty{

	};

  	/************************************************************************//**
         * @class BGL
	 *									   
	 * @brief A class to describe directed graphs.
	 *
	 * GraphPolicy on basis of the boost graph library.
	 *
	 ***************************************************************************/
	template <class T_VertexProperty = SimpleProperty, class T_EdgeProperty = SimpleProperty>
	class BGL {

        private:            
	    using GraphType =  boost::adjacency_list<boost::vecS, 
                                                     boost::vecS, 
                                                     boost::bidirectionalS, 
                                                     boost::property<boost::vertex_index_t, size_t, std::pair<graybat::graphPolicy::VertexID, T_VertexProperty> >,
                                                     boost::property<boost::edge_index_t, size_t, std::pair<graybat::graphPolicy::EdgeID, T_EdgeProperty> > >;

	    using BGLGraph    = boost::subgraph<GraphType>;
	    using BglVertexID = typename BGLGraph::vertex_descriptor;
	    using BglEdgeID   = typename BGLGraph::edge_descriptor;            
            using VertexID    = graybat::graphPolicy::VertexID;
            using EdgeID      = graybat::graphPolicy::EdgeID; 
            using GraphID     = graybat::graphPolicy::GraphID;
            
        public:

	    using VertexProperty     = T_VertexProperty;
	    using EdgeProperty       = T_EdgeProperty;
            using VertexDescription  = graybat::graphPolicy::VertexDescription<BGL>;
            using EdgeDescription    = graybat::graphPolicy::EdgeDescription<BGL>;
            using GraphDescription   = graybat::graphPolicy::GraphDescription<BGL>;
	    using InEdgeIter         = typename boost::graph_traits<BGLGraph>::in_edge_iterator;
	    using OutEdgeIter        = typename boost::graph_traits<BGLGraph>::out_edge_iterator;
	    using AdjacentVertexIter = typename boost::graph_traits<BGLGraph>::adjacency_iterator;
	    using AllVertexIter      = typename boost::graph_traits<BGLGraph>::vertex_iterator;
            
        private:
	    // Member
            std::shared_ptr<BGLGraph> graph;
	    std::vector<BGL<VertexProperty, EdgeProperty>> subGraphs;
            boost::bimap<EdgeID, BglEdgeID> edgeIdBiMap;

	public: 
	    GraphID id;

	    /**
	     * @brief The graph has to be described by *edges* 
	     * (source Vertex ==> target Vertex) and
	     * the *vertices* of this graph.
	     *
	     */
	    BGL(GraphDescription graphDesc) :
		id(0){

		std::vector<VertexDescription> vertices = graphDesc.first;
		std::vector<EdgeDescription> edges      = graphDesc.second;

		graph = std::make_shared<BGLGraph>(vertices.size());

		unsigned edgeCount = 0;


                for(EdgeID edgeId = 0; edgeId < edges.size(); ++edgeId){ 
		    BglVertexID srcVertex    = std::get<0>(edges[edgeId].first);
		    BglVertexID targetVertex = std::get<1>(edges[edgeId].first);
		    BglEdgeID bglEdgeId = boost::add_edge(srcVertex, targetVertex, (*graph)).first;
                    edgeIdBiMap.insert(typename boost::bimap<EdgeID, BglEdgeID>::value_type(edgeId, bglEdgeId));
		    setEdgeProperty(bglEdgeId, std::make_pair(edgeCount++, edges[edgeId].second));
		}

		// Bind vertex_descriptor and VertexProperty;
                for(VertexDescription &v : vertices){
                    setVertexProperty(v.first, std::make_pair(v.first, v.second));
                }
		
	    }
            
            /*******************************************************************
             * GRAPH OPERATIONS
             ******************************************************************/
            
	    /**
	     * @brief Returns all vertices of the graph
	     * 
	     */
	    std::pair<AllVertexIter, AllVertexIter> getVertices(){
		return boost::vertices((*graph));

	    }

	    /**
	     * @brief Returns the edge between source and target vertex.
	     * 
	     */
	    std::pair<EdgeID, bool> getEdge(const VertexID source, const VertexID target){
                std::pair<BglEdgeID, bool>  bglEdgeId = boost::edge(source, target, *graph);
		return std::make_pair(edgeIdBiMap.right.at(bglEdgeId.first), bglEdgeId.second);
	    }

	    /**
	     * @brief Returns all vertices, that are adjacent (connected) to *vertex*
	     *
	     */
	    std::pair<AdjacentVertexIter, AdjacentVertexIter>  getAdjacentVertices(const VertexID id){
		return boost::adjacent_vertices(id, *graph);
	    }

	    /**
	     * @brief Returns all outgoing edgpes of *srcVertex* paired with its target vertex.
	     *
	     */
	    std::pair<OutEdgeIter, OutEdgeIter> getOutEdges(const VertexID id){
		return boost::out_edges((*graph).global_to_local(id), (*graph));
	    }

	    /**
	     * @brief Returns all incoming edges to *targetVertex* paired with its source vertex.
	     *
	     */
	    std::pair<InEdgeIter, InEdgeIter> getInEdges(const VertexID id){
		return boost::in_edges((*graph).global_to_local(id), (*graph));
	    }

	    /**
	     * @brief Returns the local id of *vertex* in this graph.
	     *
	     * If this graph has no supergraph (hasSuperGraph()==false) then local ids are the same as global ids.
	     */
	    BglVertexID getLocalID(VertexProperty vertex){
		return (*graph).global_to_local(vertex.id);
	    }
	    
	    void setVertexProperty(BglVertexID vertex, VertexProperty value){
		std::pair<VertexID, VertexProperty> propPair = (*graph)[vertex];
		(*graph)[vertex] = std::make_pair<propPair.first, value>;
	    }

	    void setVertexProperty(BglVertexID vertex, std::pair<VertexID, VertexProperty> propPair){
		(*graph)[vertex] = propPair;
	    }

	    
	    void setEdgeProperty(BglEdgeID edge, EdgeProperty value){
		std::pair<EdgeID, EdgeProperty> propPair = (*graph)[edge];
		(*graph)[edge] = std::make_pair<propPair.first, value>;
	    }

	    void setEdgeProperty(BglEdgeID edge, std::pair<EdgeID, EdgeProperty> propPair){
		(*graph)[edge] = propPair;
	    }	    

	    /**
	     * @brief Returns the property of *vertex*.
	     *
	     */
	    std::pair<VertexID, VertexProperty>& getVertexProperty(const VertexID vertex){
		return (*graph)[vertex];
	    }
	    
	    
	    /**
	     * @brief Return the property of *edge*.
	     *
	     */
	    std::pair<EdgeID, EdgeProperty>& getEdgeProperty(const BglEdgeID edge){
		return (*graph)[edge];
	    }

            std::pair<EdgeID, EdgeProperty>& getEdgeProperty(const EdgeID edge){
		return getEdgeProperty(edgeIdBiMap.left.at(edge));
	    }

	    /**
	     * @brief Return the vertex to which *edge* points to.
	     *
	     */
	    BglVertexID getEdgeTarget(const BglEdgeID edge){
		return boost::target(edge, (*graph));
	    }

	    VertexID getEdgeTarget(const EdgeID edge){
                return getEdgeTarget(edgeIdBiMap.left.at(edge));
	    }
            
	    /**
	     * @brief Return the vertex to which *edge* points from.
	     *
	     */
	    BglVertexID getEdgeSource(const BglEdgeID edge){
		return boost::source(edge, (*graph));
	    }

	    VertexID getEdgeSource(const EdgeID edge){
                return getEdgeSource(edgeIdBiMap.left.at(edge));
	    }
            
	      
	};

    } // namespace graphPolicy

} // namespace graybat
