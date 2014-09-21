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

#include <types.hpp>


/************************************************************************//**
 * @class Graph
 *									   
 * @brief A class to describe directed graphs.
 *
 * Hostclass for a GraphPolicy, that implements
 * the graph functionality.     
 *
 ***************************************************************************/
template <class T_VertexProperty, class T_EdgeProperty>
class Graph {

public:
    // Public typedefs
    typedef T_VertexProperty                     Vertex;
    typedef typename Vertex::ID                  VertexID;
    typedef T_EdgeProperty                       Edge;
    typedef std::tuple<Vertex, Vertex, Edge>     EdgeDescriptor;
    typedef unsigned                             GraphID;


    // typedef typename GraphPolicy::Vertex         GraphPolicyVertex;
    // typedef typename GraphPolicy::Edge           GraphPolicyEdge;
    // struct NewVertex : public Vertex {
    // 	NewVertex(Vertex v) : Vertex(v) {
    // 	}
    // 	unsigned id;
    // 	Graph &graph;
    // };

private:

    // BGL typdefs
    typedef boost::adjacency_list<boost::vecS, 
				  boost::vecS, 
				  boost::bidirectionalS, 
				  boost::property<boost::vertex_index_t, size_t, Vertex>,
				  boost::property<boost::edge_index_t, size_t, Edge>> GraphType;

    typedef boost::subgraph<GraphType> BGLGraph;
    typedef typename BGLGraph::vertex_descriptor BGLVertex;
    typedef typename BGLGraph::edge_descriptor   BGLEdge;

    typedef typename boost::graph_traits<BGLGraph>::in_edge_iterator   InEdgeIter;
    typedef typename boost::graph_traits<BGLGraph>::out_edge_iterator  OutEdgeIter;
    typedef typename boost::graph_traits<BGLGraph>::adjacency_iterator AdjacentVertexIter;
    typedef typename boost::graph_traits<BGLGraph>::vertex_iterator    AllVertexIter;


    // Member
    BGLGraph* graph;
    GraphID id;
    Graph<Vertex, Edge>& superGraph;
    std::vector<Graph<Vertex, Edge>> subGraphs;

public: 

    /**
     * @brief The graph has to be described by *edges* (source Vertex ==Edge==> target Vertex) and
     *        the *vertices* of this graph.
     *
     */
    Graph(std::vector<EdgeDescriptor> edges, std::vector<Vertex> vertices) :
	id(0),
	superGraph(*this){

	graph = new BGLGraph(vertices.size());
	    
	for(EdgeDescriptor edgeDescriptor: edges){
	    Vertex srcVertex    = std::get<0>(edgeDescriptor);
	    Vertex targetVertex = std::get<1>(edgeDescriptor);
	    Edge edge           = std::get<2>(edgeDescriptor);
	    BGLEdge edgeID = boost::add_edge(srcVertex.id, targetVertex.id, (*graph)).first;
	    setEdgeProperty(edgeID, edge);
	    
	}

	// Bind vertex_descriptor and VertexProperty;
	for(unsigned i = 0; i < boost::num_vertices((*graph)); ++i){
	    setVertexProperty(boost::vertex(vertices.at(i).id, (*graph)), vertices.at(i));
	}
	
    }


    ~Graph(){

    }

  
    /**
     * @brief Returns all vertices of the graph
     * 
     */
    std::vector<Vertex> getVertices(){
	AllVertexIter vi, vi_end;
	std::tie(vi, vi_end) =  boost::vertices((*graph));
	return std::vector<Vertex>(vi, vi_end);

    }



    /**
     * @brief Returns all vertices, that are adjacent (connected) to *vertex*
     *
     */
    std::vector<Vertex> getAdjacentVertices(const Vertex vertex){
    	std::vector<Vertex> adjacentVertices;
    	for(std::pair<Vertex, Edge> e: getOutEdges(vertex)){
    	    adjacentVertices.push_back(e.first);
    	}

    	return adjacentVertices;
    }

    /**
     * @brief Returns all outgoing edges of *srcVertex* paired with its target vertex.
     *
     */
    std::vector<std::pair<Vertex, Edge> > getOutEdges(const Vertex srcVertex){
	OutEdgeIter ei, ei_end;
	std::tie(ei, ei_end) = boost::out_edges((*graph).global_to_local(srcVertex.id), (*graph));
	std::vector<BGLEdge> bglOutEdges(ei, ei_end);

    	std::vector<std::pair<Vertex, Edge> > outEdges;
    	for(BGLEdge e : bglOutEdges){
    	    BGLVertex target = getEdgeTarget(e);
	    Vertex vertex    = getVertex(target);
    	    Edge   edge      = getEdge(e);
    	    outEdges.push_back(std::make_pair(vertex, edge));
    	}
    	return outEdges;
    }

    /**
     * @brief Returns all incoming edges to *targetVertex* paired with its source vertex.
     *
     */
    std::vector<std::pair<Vertex, Edge> > getInEdges(const Vertex targetVertex){
	InEdgeIter ei, ei_end;
	std::tie(ei, ei_end) = boost::in_edges((*graph).global_to_local(targetVertex.id), (*graph));
	std::vector<BGLEdge> bglInEdges(ei, ei_end);

    	std::vector<std::pair<Vertex, Edge> > inEdges;
    	for(BGLEdge e : bglInEdges){
	    BGLVertex source = getEdgeSource(e);
	    Vertex vertex    = getVertex(source);
	    Edge edge        = getEdge(e);
	    inEdges.push_back(std::make_pair(vertex, edge));
    	}
    	return inEdges;
    }

    /**
     * @brief Prints graphs vertices connected by edges in simple representation
     *
     */
    void print(){
	std::vector<Vertex> vertices = getVertices();

	for(Vertex v : vertices){
	    std::vector<std::pair<Vertex, Edge> > outEdges = getOutEdges(v);
	    for(std::pair<Vertex, Edge> e : outEdges){
		std::cout << "Graph [" << id << "] "<<"Edge [" << e.second.id << "] : (" << v.id << ") ==> (" << e.first.id << ")" << std::endl; 
	    }

	}

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
    Graph<Vertex, Edge>& createSubGraph(const std::vector<Vertex> vertices){
    	std::vector<BGLVertex> vertexIDs;
	for(unsigned v_i = 0; v_i < vertices.size(); ++v_i){
	    vertexIDs.push_back(BGLVertex(vertices[v_i].id));
	}

	Graph& subGraph = (*graph).create_subgraph(vertexIDs.begin(), vertexIDs.end());

	subGraphs.push_back(Graph<Vertex, Edge>(*this, subGraph, id + 1));
	return subGraphs.back();


    }

    /**
     * @brief Cheacks wheather *textVertex* is in the set of vertices of this graph.
     * 
     * @return **true**  If testVertex is part of this graph.
     * @return **false** If testVertex is not part of this graph.
     */
    bool contains(Vertex testVertex){
	std::vector<Vertex> vertices = getVertices();
	for(Vertex containedVertex: vertices){
	    if(containedVertex.id == testVertex.id){
		return true;
	    }

	}
	return false;

    }

    /**
     * @brief Checks wheather this graph has an supergraph (is subgraph) 
     *
     * @return **true** If this graph is subgrapph of some supergraph.
     * @return **false** If this graph has no supergraph (is rootgraph).
     */
    bool hasSuperGraph(){
	if(id == superGraph.id){
	    return false;
	}
	else {
	    return true;
	}
    }

    /**
     * @brief Returns the local id of *vertex* in this graph.
     *
     * If this graph has no supergraph (hasSuperGraph()==false) then local ids are the same as global ids.
     */
    VertexID getLocalID(Vertex vertex){
	return (*graph).global_to_local(vertex.id);
    }
	    
    
    // template<typename VertexWriter, typename EdgeWriter, typename GraphWriter>
    // void writeGraph(VertexWriter vertexWriter, EdgeWriter edgeWriter, GraphWriter graphWriter, std::string fileName){
    // 	GraphPolicy::writeGraph(vertexWriter, edgeWriter, graphWriter, fileName);

    // }
private:

    Graph(Graph<Vertex, Edge>& superGraph, Graph& subGraph, unsigned id) : 
	graph(&subGraph),
	id(id),
	superGraph(superGraph){

    }

    std::vector<Vertex> getVerticesProperties(std::vector<BGLVertex> bglVertices){
	std::vector<Vertex> vertices;
	for(BGLVertex v : bglVertices){
	    vertices.push_back(getVertex(v));
	}
	return vertices;
    }

    void setVertexProperty(BGLVertex vertex, Vertex value){
	(*graph)[vertex] = value;
    }

    void setEdgeProperty(BGLEdge edge, Edge value){
	(*graph)[edge] = value;
    }

    /**
     * @brief Returns the property of *vertex*.
     *
     */
    Vertex getVertex(BGLVertex vertex){
	return (*graph)[vertex];
    }
  
    /**
     * @brief Return the property of *edge*.
     *
     */
    Edge getEdge(BGLEdge edge){
	return (*graph)[edge];
    }

    /**
     * @brief Return the vertex to which *edge* points to.
     *
     */
    BGLVertex getEdgeTarget(BGLEdge edge){
	return boost::target(edge, (*graph));
    }

    /**
     * @brief Return the vertex to which *edge* points from.
     *
     */
    BGLVertex getEdgeSource(BGLEdge edge){
	return boost::source(edge, (*graph));
    }
	      
};
