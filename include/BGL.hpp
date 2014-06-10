#pragma once
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <tuple>
#include <vector>

#include <types.hpp>

namespace GraphPolicy {

    struct NoProperty{
	typedef unsigned ID;
	NoProperty() : id(0){}
	NoProperty(ID id) : id(id){}

	ID id;
    };

    template <typename T_VertexProperty = NoProperty, typename T_EdgeProperty = NoProperty>
    class BGL {
    protected:
	typedef T_VertexProperty   VertexProperty;
	typedef T_EdgeProperty     EdgeProperty;

    private:
	typedef boost::adjacency_list<boost::vecS, 
				      boost::vecS, 
				      boost::bidirectionalS, 
				      VertexProperty,
				      EdgeProperty> Graph;

	typedef typename boost::graph_traits<Graph>::in_edge_iterator   InEdgeIter;
	typedef typename boost::graph_traits<Graph>::out_edge_iterator  OutEdgeIter;
	typedef typename boost::graph_traits<Graph>::adjacency_iterator AdjacentVertexIter;
	typedef typename boost::graph_traits<Graph>::vertex_iterator    AllVertexIter;


    protected:
	typedef typename Graph::vertex_descriptor Vertex;
	typedef typename Graph::edge_descriptor   Edge;

    private:
	typedef typename std::tuple<Vertex, Vertex, EdgeProperty>       EdgeDescriptor;

    protected:

	Graph graph;
	template <typename T> using               Container = std::vector<T>;
    
	BGL(std::vector<EdgeDescriptor> edges, 
	    std::vector<VertexProperty> vertexProperties){
	
	    graph = Graph(vertexProperties.size());

	    for(auto edge: edges){
		Edge e = boost::add_edge(std::get<0>(edge), std::get<1>(edge), graph).first;
		setEdgeProperty(e, std::get<2>(edge));
	    
	    }

	    // Bind vertex_descriptor and VertexProperty;
	    for(unsigned i = 0; i < boost::num_vertices(graph); ++i){
		setVertexProperty(boost::vertex(i, graph), vertexProperties.at(i));
	    }

	}

	Container<Vertex> getVertices(){
	    AllVertexIter vi, vi_end;
	    std::tie(vi, vi_end) =  boost::vertices(graph);
	    return Container<Vertex>(vi, vi_end);
	}

	VertexProperty getVertexProperty(Vertex vertex){
	    return graph[vertex];
	}
  
	EdgeProperty getEdgeProperty(Edge edge){
	    return graph[edge];
	}

	Container<Edge> getInEdges(Vertex vertex){
	    InEdgeIter ei, ei_end;
	    std::tie(ei, ei_end) = boost::in_edges(vertex, graph);
	    return Container<Edge> (ei, ei_end);
	}

	Container<Edge> getOutEdges(Vertex vertex){
	    OutEdgeIter ei, ei_end;
	    std::tie(ei, ei_end) = boost::out_edges(vertex, graph);
	    return Container<Edge> (ei, ei_end);
	}

	Vertex getEdgeTarget(Edge edge){
	    return boost::target(edge, graph);
	}

	Vertex getEdgeSource(Edge edge){
	    return boost::source(edge, graph);
	}

    private:
	void setVertexProperty(Vertex vertex, VertexProperty value){
	    graph[vertex] = value;
	}

	void setEdgeProperty(Edge edge, EdgeProperty value){
	    graph[edge] = value;
	}



    };


} // namespace GraphPolicy
