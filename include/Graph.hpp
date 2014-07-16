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
    typedef unsigned                             GraphID;

private: 
    template <typename T> 
    using Container = typename GraphPolicy::template Container<T>;

    typedef typename GraphPolicy::Vertex                    GraphPolicyVertex;
    typedef typename GraphPolicy::Edge                      GraphPolicyEdge;
    typedef typename Container<GraphPolicyEdge>::iterator   EdgeIter;
    typedef typename Container<GraphPolicyVertex>::iterator VertexIter;
    typedef std::tuple<Vertex, Vertex, Edge>                EdgeDescriptor;
  typedef typename Vertex::ID                               VertexID;


  Graph(Graph<GraphPolicy>& superGraph, typename GraphPolicy::Graph& subGraph, unsigned id) : 
    GraphPolicy(subGraph),
    id(id),
    superGraph(superGraph){

  }

public:

  GraphID id;
  Graph<GraphPolicy>& superGraph;
  std::vector<Graph<GraphPolicy>> subGraphs;
 
    Graph(std::vector<EdgeDescriptor> edges, std::vector<Vertex> vertices) :
	GraphPolicy(toGraphPolicyEdges(edges), vertices),
	id(0),
	superGraph(*this){
	
    }


  ~Graph(){

  }

  
    
    std::vector<Vertex> getVertices(){
	Container<GraphPolicyVertex> v = GraphPolicy::getVertices();
	return getVerticesProperties(v);
    }

    std::vector<Vertex> getAdjacentVertices(const Vertex vertex){
    	std::vector<Vertex> adjacentVertices;
    	for(Edge e: GraphPolicy::getOutEdges(vertex.id)){
    	    GraphPolicyVertex target = GraphPolicy::getEdgeTarget(e);
    	    Vertex s = GraphPolicy::getVertexProperty(target);
    	    adjacentVertices.push_back(s);
    	}

    	return adjacentVertices;
    }

    std::vector<std::pair<Vertex, Edge> > getOutEdges(const Vertex srcVertex){

    	std::vector<std::pair<Vertex, Edge> > outEdges;
    	for(GraphPolicyEdge e : GraphPolicy::getOutEdges(srcVertex.id)){
    	    GraphPolicyVertex target = GraphPolicy::getEdgeTarget(e);
	    Vertex vertex = GraphPolicy::getVertexProperty(target);
    	    Edge   edge   = GraphPolicy::getEdgeProperty(e);
    	    outEdges.push_back(std::make_pair(vertex, edge));
    	}
    	return outEdges;
    }

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

    void print(){
	std::vector<Vertex> vertices = getVertices();

	for(Vertex v : vertices){
	    std::vector<std::pair<Vertex, Edge> > outEdges = getOutEdges(v);
	    for(std::pair<Vertex, Edge> e : outEdges){
	      std::cout << "Graph [" << id << "] "<<"Edge [" << e.second.id << "] : (" << v.id << ") ==> (" << e.first.id << ")" << std::endl; 
	    }

	}

    }



  Graph<GraphPolicy>& createSubGraph(std::vector<Vertex> vertices){
    	std::vector<GraphPolicyVertex> graphPolicyVertices;
    	 for(unsigned v_i = 0; v_i < vertices.size(); ++v_i){
	   graphPolicyVertices.push_back(GraphPolicyVertex(vertices[v_i].id));
    	 }

	 typename GraphPolicy::Graph& subGraph = GraphPolicy::createSubGraph(graphPolicyVertices); 

	 subGraphs.push_back(Graph<GraphPolicy>(*this, subGraph, id + 1));
	 return subGraphs.back();


    }

  bool contains(Vertex testVertex){
    std::vector<Vertex> vertices = getVertices();
    for(Vertex containedVertex: vertices){
      if(containedVertex.id == testVertex.id){
	return true;
      }

    }
    return false;

  }

  bool hasSuperGraph(){
    if(id == superGraph.id){
      return false;
    }
    else {
      return true;
    }
  }

  VertexID getLocalID(Vertex vertex){
    return GraphPolicy::getLocalID(vertex.id);
  }
	      
private:
    std::vector<Vertex> getVerticesProperties(Container<GraphPolicyVertex> vertices){
	std::vector<Vertex> properties;
	for(GraphPolicyVertex v : vertices){
	    properties.push_back(GraphPolicy::getVertexProperty(v));
	}
	return properties;
    }
	      
    std::vector<std::tuple <GraphPolicyVertex, GraphPolicyVertex, Edge> > toGraphPolicyEdges(std::vector<EdgeDescriptor> edges){
	std::vector<std::tuple<GraphPolicyVertex, GraphPolicyVertex, Edge> > bglEdges;
	for(auto e : edges){
	    bglEdges.push_back(std::make_tuple(std::get<0>(e).id, std::get<1>(e).id, std::get<2>(e)));
	}
	return bglEdges;
    }

};
