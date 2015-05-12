#pragma once
#include <vector>
#include <iostream>
#include <tuple>

#include <boost/graph/graph_traits.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/subgraph.hpp>
#include <boost/graph/graphviz.hpp>
#include <tuple>
#include <string>
#include <fstream> /* std::fstream */


namespace graybat {
    
    namespace graphPolicy {
	
	struct SimpleProperty{
	    typedef unsigned ID;
	    SimpleProperty() : id(0){}
	    SimpleProperty(ID id) : id(id){}

	    ID id;
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

	public:
	    // Public typedefs
	    typedef T_VertexProperty                                                VertexProperty;
	    typedef T_EdgeProperty                                                  Edge;
	    typedef T_EdgeProperty                                                  EdgeProperty;	    
	    typedef std::pair<unsigned, unsigned>                                   EdgeDescription;
	    typedef std::pair<std::vector<unsigned>, std::vector<EdgeDescription> > GraphDescription;

	    typedef unsigned                                                        GraphID;


	    // BGL typdefs
	    typedef boost::adjacency_list<boost::vecS, 
					  boost::vecS, 
					  boost::bidirectionalS, 
					  boost::property<boost::vertex_index_t, size_t, VertexProperty>,
					  boost::property<boost::edge_index_t, size_t, Edge> > GraphType;

	    typedef boost::subgraph<GraphType> BGLGraph;
	    typedef typename BGLGraph::vertex_descriptor VertexID;
	    typedef typename BGLGraph::edge_descriptor   EdgeID;

	    typedef typename boost::graph_traits<BGLGraph>::in_edge_iterator   InEdgeIter;
	    typedef typename boost::graph_traits<BGLGraph>::out_edge_iterator  OutEdgeIter;
	    typedef typename boost::graph_traits<BGLGraph>::adjacency_iterator AdjacentVertexIter;
	    typedef typename boost::graph_traits<BGLGraph>::vertex_iterator    AllVertexIter;
	    


	    // Member
	    BGLGraph* graph;
	    std::vector<BGL<VertexProperty, Edge>> subGraphs;

	public: 
	    GraphID id;
	    //BGL<Vertex, Edge>& superGraph;


	    /**
	     * @brief The graph has to be described by *edges* 
	     * (source Vertex ==> target Vertex) and
	     * the *vertices* of this graph.
	     *
	     */
	    BGL(GraphDescription graphDesc) :
		id(0){

		std::vector<unsigned> vertices     = graphDesc.first;
		std::vector<EdgeDescription> edges = graphDesc.second;

		graph = new BGLGraph(vertices.size());

		unsigned edgeCount = 0;

		for(EdgeDescription edge: edges){
		    VertexID srcVertex    = std::get<0>(edge);
		    VertexID targetVertex = std::get<1>(edge);
		    EdgeID edgeID = boost::add_edge(srcVertex, targetVertex, (*graph)).first;
		    setEdgeProperty(edgeID, Edge(edgeCount++));
		}

		// Bind vertex_descriptor and VertexProperty;
		for(unsigned vertexID = 0; vertexID < vertices.size(); ++vertexID){
		    setVertexProperty(vertexID, VertexProperty(vertexID));
		}
		
	    }	    

	    ~BGL(){

	    }

  
	    /**
	     * @brief Returns all vertices of the graph
	     * 
	     */
	    std::pair<AllVertexIter, AllVertexIter> getVertices(){
		return boost::vertices((*graph));

	    }

	    /**
	     * @brief Returns all vertices, that are adjacent (connected) to *vertex*
	     *
	     */
	    std::pair<AdjacentVertexIter, AdjacentVertexIter>  getAdjacentVertices(const VertexID id){
		return boost::adjacent_vertices(id, *graph);
	    }

	    /**
	     * @brief Returns all outgoing edges of *srcVertex* paired with its target vertex.
	     *
	     */
	    std::pair<OutEdgeIter, OutEdgeIter> getOutEdges(const VertexID id){
		return boost::out_edges((*graph).global_to_local(id), (*graph));
	    }

	    /**
	     * @brief Returns all incoming edges to *targetVertex* paired with its source vertex.
	     *
	     */
	    std::vector<std::pair<VertexProperty, Edge> > getInEdges(const VertexProperty targetVertex){
		InEdgeIter ei, ei_end;
		std::tie(ei, ei_end) = boost::in_edges((*graph).global_to_local(targetVertex.id), (*graph));
		std::vector<EdgeID> bglInEdges(ei, ei_end);

		std::vector<std::pair<VertexProperty, Edge> > inEdges;
		for(EdgeID e : bglInEdges){
		    VertexID source = getEdgeSource(e);
		    VertexProperty vertex    = getVertexProperty(source);
		    Edge edge        = getEdge(e);
		    inEdges.push_back(std::make_pair(vertex, edge));
		}
		return inEdges;
	    }

	    /**
	     * @brief Creates a subgraph of this graph from *vertices* 
	     *        and returns a reference to this newly created subgraph. 
	     *
	     * Connected vertices in this graph are still connected in the subgraph.
	     * The subgraph will be added to the children of this graph and this graph
	     * will be the supergraph of the subgraph. The global id of the subgraph
	     * vertices are always reachable with vertex.id. To obtain the local id
	     * whithin a subgraph it is possible to call getLocalID(Vertex).
	     *
	     * @param[in] vertices A list of vertices that should be in set of the graph vertices
	     *
	     */
	    /*
	    BGL<Vertex, Edge>& createSubGraph(const std::vector<Vertex> vertices){
		std::vector<VertexID> vertexIDs;
		for(unsigned v_i = 0; v_i < vertices.size(); ++v_i){
		    vertexIDs.push_back(VertexID(vertices[v_i].id));
		}

		BGL& subGraph = (*graph).create_subgraph(vertexIDs.begin(), vertexIDs.end());

		subGraphs.push_back(BGL<Vertex, Edge>(*this, subGraph, id + 1));
		return subGraphs.back();


	    }
	    */

	    /**
	     * @brief Cheacks wheather *textVertex* is in the set of vertices of this graph.
	     * 
	     * @return **true**  If testVertex is part of this graph.
	     * @return **false** If testVertex is not part of this graph.
	     */
	    /*
	    bool contains(Vertex testVertex){
		std::vector<Vertex> vertices = getVertices();
		for(Vertex containedVertex: vertices){
		    if(containedVertex.id == testVertex.id){
			return true;
		    }

		}
		return false;

	    }
	    */

	    /**
	     * @brief Checks wheather this graph has an supergraph (is subgraph) 
	     *
	     * @return **true** If this graph is subgrapph of some supergraph.
	     * @return **false** If this graph has no supergraph (is rootgraph).
	     */
	    /*
	    bool hasSuperGraph(){
		if(id == superGraph.id){
		    return false;
		}
		else {
		    return true;
		}
	    }
	    */

	    /**
	     * @brief Returns the local id of *vertex* in this graph.
	     *
	     * If this graph has no supergraph (hasSuperGraph()==false) then local ids are the same as global ids.
	     */
	    VertexID getLocalID(VertexProperty vertex){
		return (*graph).global_to_local(vertex.id);
	    }
	    
   

	    /*
	    BGL(BGL<Vertex, Edge>& superGraph, BGL& subGraph, unsigned id) : 
		graph(&subGraph),
		id(id),
		superGraph(superGraph){

	    }
	    */

	    std::vector<VertexProperty> getVerticesProperties(std::vector<VertexID> bglVertices){
		std::vector<VertexProperty> vertices;
		for(VertexID v : bglVertices){
		    vertices.push_back(getVertexProperty(v));
		}
		return vertices;
	    }

	    void setVertexProperty(VertexID vertex, VertexProperty value){
		(*graph)[vertex] = value;
	    }

	    void setEdgeProperty(EdgeID edge, Edge value){
		(*graph)[edge] = value;
	    }

	    /**
	     * @brief Returns the property of *vertex*.
	     *
	     */
	    // Vertex getVertexProperty(VertexID vertex){
	    // 	return (*graph)[vertex];
	    // }


	    VertexProperty& getVertexProperty(const VertexID vertex){
		return (*graph)[vertex];
	    }
	    

	    
	    /**
	     * @brief Return the property of *edge*.
	     *
	     */
	    Edge& getEdgeProperty(const EdgeID edge){
		return (*graph)[edge];
	    }

	    /**
	     * @brief Return the vertex to which *edge* points to.
	     *
	     */
	    VertexID getEdgeTarget(EdgeID edge){
		return boost::target(edge, (*graph));
	    }

	    /**
	     * @brief Return the vertex to which *edge* points from.
	     *
	     */
	    VertexID getEdgeSource(EdgeID edge){
		return boost::source(edge, (*graph));
	    }
	      
	};

    } // namespace graphPolicy

} // namespace graybat
