#pragma once
#include <vector>
#include <iostream>
#include <tuple>



/************************************************************************//**
 * @class Graph
 *									   
 * @brief A class to describe directed graphs.
 *
 * Hostclass for a GraphPolicy, that implements
 * the graph functionality.     
 *
 ***************************************************************************/
template <class T_GraphPolicy>
class Graph : public T_GraphPolicy{
private:
    typedef T_GraphPolicy GraphPolicy;

public:
    typedef typename GraphPolicy::VertexProperty Vertex;
    typedef typename Vertex::ID                  VertexID;
    typedef typename GraphPolicy::EdgeProperty   Edge;
    typedef std::tuple<Vertex, Vertex, Edge>     EdgeDescriptor;
    typedef unsigned                             GraphID;
    typedef typename GraphPolicy::Vertex         GraphPolicyVertex;
    typedef typename GraphPolicy::Edge           GraphPolicyEdge;


private: 
    template <typename T> 
    using Container = typename GraphPolicy::template Container<T>;

    typedef typename Container<GraphPolicyEdge>::iterator   EdgeIter;
    typedef typename Container<GraphPolicyVertex>::iterator VertexIter;

    Graph(Graph<GraphPolicy>& superGraph, typename GraphPolicy::Graph& subGraph, unsigned id) : 
	GraphPolicy(subGraph),
	id(id),
	superGraph(superGraph){

    }

public:

    GraphID id;
    Graph<GraphPolicy>& superGraph;
    std::vector<Graph<GraphPolicy>> subGraphs;
 

    /**
     * @brief The graph has to be described by *edges* (source Vertex ==Edge==> target Vertex) and
     *        the *vertices* of this graph.
     *
     */
    Graph(std::vector<EdgeDescriptor> edges, std::vector<Vertex> vertices) :
	GraphPolicy(edges, vertices),
	id(0),
	superGraph(*this){
	
    }


    ~Graph(){

    }

  
    /**
     * @brief Returns all vertices of the graph
     * 
     */
    std::vector<Vertex> getVertices(){
	Container<GraphPolicyVertex> v = GraphPolicy::getVertices();
	return getVerticesProperties(v);
    }

    /**
     * @brief Returns all vertices, that are adjacent (connected) to *vertex*
     *
     */
    std::vector<Vertex> getAdjacentVertices(const Vertex vertex){
    	std::vector<Vertex> adjacentVertices;
    	for(Edge e: GraphPolicy::getOutEdges(vertex.id)){
    	    GraphPolicyVertex target = GraphPolicy::getEdgeTarget(e);
    	    Vertex s = GraphPolicy::getVertexProperty(target);
    	    adjacentVertices.push_back(s);
    	}

    	return adjacentVertices;
    }

    /**
     * @brief Returns all outgoing edges of *srcVertex* paired with its target vertex.
     *
     */
    std::vector<std::pair<Vertex, Edge> > getOutEdges(const Vertex srcVertex){

    	std::vector<std::pair<Vertex, Edge> > outEdges;
    	for(GraphPolicyEdge e : GraphPolicy::getOutEdges(srcVertex.id)){
    	    GraphPolicyVertex target = GraphPolicy::getEdgeTarget(e);
	    Vertex vertex = GraphPolicy::getVertexProperty(target);
    	    Edge   edge   = GraphPolicy::getEdgeProperty(e);
    	    outEdges.push_back(std::make_pair(vertex, edge));
    	}
    	return outEdges;
	//return std::vector<std::pair<Vertex, Edge> > (1, std::make_pair(Vertex(0), Edge(0)));
    }

    /**
     * @brief Returns all incoming edges to *targetVertex* paired with its source vertex.
     *
     */
    std::vector<std::pair<Vertex, Edge> > getInEdges(const Vertex targetVertex){
    	std::vector<std::pair<Vertex, Edge> > inEdges;
    	for(GraphPolicyEdge e : GraphPolicy::getInEdges(targetVertex.id)){
	    GraphPolicyVertex source = GraphPolicy::getEdgeSource(e);
	    Vertex vertex            = GraphPolicy::getVertexProperty(source);
	    Edge edge = GraphPolicy::getEdgeProperty(e);
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
    Graph<GraphPolicy>& createSubGraph(const std::vector<Vertex> vertices){
    	std::vector<GraphPolicyVertex> graphPolicyVertices;
	for(unsigned v_i = 0; v_i < vertices.size(); ++v_i){
	    graphPolicyVertices.push_back(GraphPolicyVertex(vertices[v_i].id));
	}

	typename GraphPolicy::Graph& subGraph = GraphPolicy::createSubGraph(graphPolicyVertices); 

	subGraphs.push_back(Graph<GraphPolicy>(*this, subGraph, id + 1));
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
	return GraphPolicy::getLocalID(vertex.id);
    }
	    
    
    template<typename VertexWriter, typename EdgeWriter, typename GraphWriter>
    void writeGraph(VertexWriter vertexWriter, EdgeWriter edgeWriter, GraphWriter graphWriter, std::string fileName){
	GraphPolicy::writeGraph(vertexWriter, edgeWriter, graphWriter, fileName);

    }
private:
    std::vector<Vertex> getVerticesProperties(Container<GraphPolicyVertex> vertices){
	std::vector<Vertex> properties;
	for(GraphPolicyVertex v : vertices){
	    properties.push_back(GraphPolicy::getVertexProperty(v));
	}
	return properties;
    }
	      
};
