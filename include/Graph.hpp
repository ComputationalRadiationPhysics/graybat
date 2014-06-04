#pragma once
#include <vector>
#include <iostream>
#include <tuple>

template <class T_GraphPolicy>
class Graph : public T_GraphPolicy{
private:
    typedef T_GraphPolicy GraphPolicy;

public:
    typedef typename GraphPolicy::VertexProperty Vertex;
    typedef typename GraphPolicy::EdgeProperty   Edge;

private: 
    template <typename T> 
    using Container = typename GraphPolicy::template Container<T>;

    typedef typename GraphPolicy::Vertex                    GraphPolicyVertex;
    typedef typename GraphPolicy::Edge                      GraphPolicyEdge;
    typedef typename Container<GraphPolicyEdge>::iterator   EdgeIter;
    typedef typename Container<GraphPolicyVertex>::iterator VertexIter;
    typedef std::tuple<Vertex, Vertex, Edge>                EdgeDescriptor;


public:
    Graph(std::vector<EdgeDescriptor> edges, std::vector<Vertex> vertices) :
	GraphPolicy(toGraphPolicyEdges(edges), vertices){
	
    }

    std::vector<Vertex> getVertices(){
	Container<GraphPolicyVertex> v = GraphPolicy::getVertices();
	return getVerticesProperties(v);
    }

    std::vector<Vertex> getAdjacentVertices(const Vertex vertex){
    	std::vector<Vertex> adjacentVertices;
    	for(Edge e: GraphPolicy::getOutEdges(vertex.uuid)){
    	    GraphPolicyVertex target = GraphPolicy::getEdgeTarget(e);
    	    Vertex s = GraphPolicy::getVertexProperty(target);
    	    adjacentVertices.push_back(s);
    	}

    	return adjacentVertices;
    }

    std::vector<std::pair<Vertex, Edge> > getOutEdges(const Vertex srcVertex){

    	std::vector<std::pair<Vertex, Edge> > outEdges;
    	for(GraphPolicyEdge e : GraphPolicy::getOutEdges(srcVertex.uuid)){
    	    GraphPolicyVertex target = GraphPolicy::getEdgeTarget(e);
	    Vertex vertex = GraphPolicy::getVertexProperty(target);
    	    Edge   edge   = GraphPolicy::getEdgeProperty(e);
    	    outEdges.push_back(std::make_pair(vertex, edge));
    	}
    	return outEdges;
    }

    std::vector<std::pair<Vertex, Edge> > getInEdges(const Vertex targetVertex){
    	std::vector<std::pair<Vertex, Edge> > inEdges;
    	for(GraphPolicyEdge e : GraphPolicy::getInEdges(targetVertex.uuid)){
    	     GraphPolicyVertex source = GraphPolicy::getEdgeSource(e);
	     Vertex vertex            = GraphPolicy::getVertexProperty(source);
	     Edge edge = GraphPolicy::getEdgeProperty(e);
	     inEdges.push_back(std::make_pair(vertex, edge));
    	}
    	return inEdges;
    }

    void print(){
	std::vector<Vertex> vertices = getVertices();

	for(Vertex v : vertices){
	    std::vector<std::pair<Vertex, Edge> > outEdges = getOutEdges(v);
	    for(std::pair<Vertex, Edge> e : outEdges){
		std::cout << "Edge [" << e.second.uuid << "] : (" << v.uuid << ") ==> (" << e.first.uuid << ")" << std::endl; 
	    }

	}

    }
	      
private:
    std::vector<Vertex> getVerticesProperties(Container<GraphPolicyVertex> v){
	std::vector<Vertex> properties;
	for(VertexIter vi = v.begin(); vi != v.end(); ++vi){
	    properties.push_back(GraphPolicy::getVertexProperty(*vi));
	}
	return properties;
    }
	      
    std::vector<std::tuple <GraphPolicyVertex, GraphPolicyVertex, Edge> > toGraphPolicyEdges(std::vector<EdgeDescriptor> edges){
	std::vector<std::tuple<GraphPolicyVertex, GraphPolicyVertex, Edge> > bglEdges;
	for(auto e : edges){
	    bglEdges.push_back(std::make_tuple(std::get<0>(e).uuid, std::get<1>(e).uuid, std::get<2>(e)));
	}
	return bglEdges;
    }

};
