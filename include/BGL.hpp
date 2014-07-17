#pragma once
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/subgraph.hpp>
#include <tuple>
#include <vector>

#include <types.hpp>

/**
 * @brief Brief comment on GraphPolicy.
 *
 */
namespace GraphPolicy {

    /************************************************************************//**
     * @struct NoProperty
     *
     * @brief Dummy property for a GraphPolicy, 
     *        that only contains the required id.
     *
     ***************************************************************************/
    struct NoProperty{
	typedef unsigned ID;
	NoProperty() : id(0){}
	NoProperty(ID id) : id(id){}

	ID id;
    };


    /***********************************************************************//**
     * @class BGL
     *
     * @brief A implementation for GraphPolicy for the hostclass graph.
     *
     * Provides only some functionality of the Boost Graph Library that
     * is needed by the usecase of the implementor. For the inner graph,
     * a subgraph implementation was choosen to support subgraph creation.
     *
     ***************************************************************************/
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
	typedef typename Graph::vertex_descriptor Vertex;
	typedef typename Graph::edge_descriptor   Edge;
	template <typename T> using               Container = std::vector<T>;

    private:
	typedef typename boost::graph_traits<Graph>::in_edge_iterator   InEdgeIter;
	typedef typename boost::graph_traits<Graph>::out_edge_iterator  OutEdgeIter;
	typedef typename boost::graph_traits<Graph>::adjacency_iterator AdjacentVertexIter;
	typedef typename boost::graph_traits<Graph>::vertex_iterator    AllVertexIter;
	typedef typename std::tuple<VertexProperty, VertexProperty, EdgeProperty>       EdgeDescriptor;

    protected:


      Graph* graph;

	/**
	 * @brief Creates this BGL graph from other BGL graph. Used for subgraph creation 
	 *        created by createSubGraph(std::vector<Vertex>).
	 *
	 */
	BGL(Graph& subGraph) : graph(&subGraph){
	     
	}
	

	/**
	 * @brief Creates this BGL graph from edges (VertexProperty ==EdgeProperty==> VertexProperty)
	 *        and VertexProperties. VertexProperty and EdgeProperty need to have at least
	 *        an id member to be unique identifiable. The ids need to range from 0 to n-1,
	 *        how n is the number of vertices in this graph. Vertex.id is mapped 1:1 to the 
	 *        BGL inner representation of vertices, thus a vertex with id i is also the ith
	 *        vertex in the BGL graph.
	 *
	 */
	BGL(std::vector<EdgeDescriptor> edges, 
	    std::vector<VertexProperty> vertexProperties) {
	
	    graph = new Graph(vertexProperties.size());
	    
	    for(EdgeDescriptor edgeDescriptor: edges){
		VertexProperty srcVertex    = std::get<0>(edgeDescriptor);
		VertexProperty targetVertex = std::get<1>(edgeDescriptor);
		EdgeProperty edgeProperty   = std::get<2>(edgeDescriptor);
		Edge edge = boost::add_edge(srcVertex.id, targetVertex.id, (*graph)).first;
		setEdgeProperty(edge, edgeProperty);
	    
	    }

	    // Bind vertex_descriptor and VertexProperty;
	    for(unsigned i = 0; i < boost::num_vertices((*graph)); ++i){
		setVertexProperty(boost::vertex(vertexProperties.at(i).id, (*graph)), vertexProperties.at(i));
	    }

	}

      ~BGL(){
	// TODO
	// For some reason its no good idea to destroy the graph
	  if((*graph).is_root()){
	  delete graph;
	}
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
	    return (*graph).create_subgraph(vertices.begin(), vertices.end());
	}

      Vertex getLocalID(Vertex vertex){
	return (*graph).global_to_local(vertex);
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
