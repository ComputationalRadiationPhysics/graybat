#pragma once
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/subgraph.hpp>
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
				      boost::property<boost::vertex_index_t, size_t, VertexProperty>,
				      boost::property<boost::edge_index_t, size_t, EdgeProperty>> GraphTmp;

    protected:
	typedef boost::subgraph<GraphTmp> Graph;

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
	template <typename T> using               Container = std::vector<T>;

	Graph* graph;

	BGL(Graph& subGraph) : graph(&subGraph){

	}

	BGL(std::vector<EdgeDescriptor> edges, 
	    std::vector<VertexProperty> vertexProperties) {
	
	    graph = new Graph(vertexProperties.size());
	    
	    for(auto edge: edges){
		Edge e = boost::add_edge(std::get<0>(edge), std::get<1>(edge), (*graph)).first;
		setEdgeProperty(e, std::get<2>(edge));
	    
	    }

	    // Bind vertex_descriptor and VertexProperty;
	    for(unsigned i = 0; i < boost::num_vertices((*graph)); ++i){
		setVertexProperty(boost::vertex(i, (*graph)), vertexProperties.at(i));
	    }

	}

	~BGL(){
	    // TODO
	    // just delete graph if its root graph
	    // delete graph;
	}

	Container<Vertex> getVertices(){
	    AllVertexIter vi, vi_end;
	    std::tie(vi, vi_end) =  boost::vertices((*graph));
	    return Container<Vertex>(vi, vi_end);
	}

	VertexProperty getVertexProperty(Vertex vertex){
	    return (*graph)[vertex];
	}
  
	EdgeProperty getEdgeProperty(Edge edge){
	    return (*graph)[edge];
	}

	Container<Edge> getInEdges(Vertex vertex){
	    InEdgeIter ei, ei_end;
	    std::tie(ei, ei_end) = boost::in_edges((*graph).global_to_local(vertex), (*graph));
	    return Container<Edge> (ei, ei_end);
	}

	Container<Edge> getOutEdges(Vertex vertex){
	    OutEdgeIter ei, ei_end;
	    std::tie(ei, ei_end) = boost::out_edges((*graph).global_to_local(vertex), (*graph));
	    return Container<Edge> (ei, ei_end);
	}

	Vertex getEdgeTarget(Edge edge){
	    return boost::target(edge, (*graph));
	}

	Vertex getEdgeSource(Edge edge){
	    return boost::source(edge, (*graph));
	}

	Graph& createSubGraph(const std::vector<Vertex> vertices){
	    //subGraph =  graph.create_subgraph(vertices.begin(), vertices.end());
	    return (*graph).create_subgraph(vertices.begin(), vertices.end());
	    // AllVertexIter vi, vi_end;
	    // std::tie(vi, vi_end) =  boost::vertices(subGraph);
	    // Container<Vertex>c (vi, vi_end);

	    // for(Vertex v : c){
	    // 	std::cout << subGraph[v].id << std::endl;
	    // }


	    //std::cout << "BGL " << subGraph[boost::vertex(2, subGraph)].id << std::endl;
	    //return sg;
	}


    private:
	void setVertexProperty(Vertex vertex, VertexProperty value){
	    (*graph)[vertex] = value;
	}

	void setEdgeProperty(Edge edge, EdgeProperty value){
	    (*graph)[edge] = value;
	}



    };


} // namespace GraphPolicy
