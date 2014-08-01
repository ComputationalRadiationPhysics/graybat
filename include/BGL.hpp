#pragma once
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/subgraph.hpp>
#include <boost/graph/graphviz.hpp>
#include <tuple>
#include <vector>
#include <string>
#include <fstream> /* std::fstream */

#include <types.hpp>

/**
 * @brief Policies for Graph, that implements graph functionality.
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
     * @brief A implementation for GraphPolicy for the hostclass Graph.
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
	template <typename T> using               Container = std::vector<T>;

    public:
	typedef typename Graph::vertex_descriptor Vertex;
	typedef typename Graph::edge_descriptor   Edge;

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
	    //writeGraph("subgraph.dot");
	}
	

	/**
	 * @brief Creates this BGL graph from edges (VertexProperty ==EdgeProperty==> VertexProperty)
	 *        and VertexProperties. 
	 *
	 *        VertexProperty and EdgeProperty need to have at least
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

	/**
	 * @bug cleanup of graph leads to SEG FAULTS
	 *
	 */
	~BGL(){
	    // TODO
	    // For some reason its no good idea to destroy the graph
	    // if((*graph).is_root()){
	    //delete graph;
	    // }
	}

	/**
	 * @brief Returns a container of all vertices of this graph. This
	 *        container supports at least the iterator mechanism.
	 *
	 * @remark Instead of returning the Container of vertices, a pair
	 *         of first and end iterator would be more efficient. But
	 *         this would also lead to different problems, because
	 *         BGL provides different iterators for different functions.
	 *
	 */
	Container<Vertex> getVertices(){
	    AllVertexIter vi, vi_end;
	    std::tie(vi, vi_end) =  boost::vertices((*graph));
	    return Container<Vertex>(vi, vi_end);
	}

	/**
	 * @brief Returns the property of *vertex*.
	 *
	 */
	VertexProperty getVertexProperty(Vertex vertex){
	    return (*graph)[vertex];
	}
  
	/**
	 * @brief Return the property of *edge*.
	 *
	 */
	EdgeProperty getEdgeProperty(Edge edge){
	    return (*graph)[edge];
	}

	/**
	 * @brief Returns all incoming edges of *vertex*.
	 *
	 */
	Container<Edge> getInEdges(Vertex vertex){
	    InEdgeIter ei, ei_end;
	    std::tie(ei, ei_end) = boost::in_edges((*graph).global_to_local(vertex), (*graph));
	    return Container<Edge> (ei, ei_end);
	}

	/**
	 * @brief Returns all outgoing edges of *vertex*.
	 *
	 */
	Container<Edge> getOutEdges(Vertex vertex){
	    OutEdgeIter ei, ei_end;
	    std::tie(ei, ei_end) = boost::out_edges((*graph).global_to_local(vertex), (*graph));
	    return Container<Edge> (ei, ei_end);
	}

	/**
	 * @brief Return the vertex to which *edge* points to.
	 *
	 */
	Vertex getEdgeTarget(Edge edge){
	    return boost::target(edge, (*graph));
	}

	/**
	 * @brief Return the vertex to which *edge* points from.
	 *
	 */
	Vertex getEdgeSource(Edge edge){
	    return boost::source(edge, (*graph));
	}

	/**
	 * @brief Creates a subgraph from a list of *vertices*.
	 *
	 * This vertices should be part of this graph.
	 *
	 */
	Graph& createSubGraph(const Container<Vertex> vertices){
	    return (*graph).create_subgraph(vertices.begin(), vertices.end());
	}

	/**
	 * @brief Returns the local id of *vertex* in this graph.
	 *
	 * If this graph has no supergraph (graph.is_root()==true) then local ids are the same as global ids.
	 */
	Vertex getLocalID(Vertex vertex){
	    return (*graph).global_to_local(vertex);
	}

	/**
	 * @brief Test of graph visualization with the BGL and graphviz.
	 *        A file graph.dot will be written to the root directory
	 *        and can be transformed to a png etc. by :
	 *        dot graph.dot -Tpng > graph.png
	 */
	template<typename VertexWriter, typename EdgeWriter, typename GraphWriter>
	void writeGraph(VertexWriter vertexWriter, EdgeWriter edgeWriter, GraphWriter graphWriter, std::string fileName){
	    std::fstream fileStream(fileName, std::fstream::out);

	    boost::write_graphviz(fileStream, 
				  (*graph), 
				  vertexWriter,
				  edgeWriter,
				  graphWriter);
	}


    private:

	// template<class Graph>
	// struct vertexIDWriter{
	//     vertexIDWriter(Graph &graph) : graph(graph) {}
	//     void operator()(std::ostream& out, const Vertex& v) const {
	// 	out << "[label=\"" << graph.getVertexProperty(v).id << "\"]";
	//     }
	// private:
	//     Graph& graph;
	// };


	// template<class Graph>
	// struct edgeIDWriter{
	//     edgeIDWriter(Graph &graph) : graph(graph) {}
	//     void operator()(std::ostream& out, const Edge& e) const {
	// 	out << "[label=\"" << graph.getEdgeProperty(e).id << "\"]";
	//     }
	// private:
	//     Graph& graph;
	// };

	// struct graphWriter {
	//     void operator()(std::ostream& out) const {
	// 	out << "ratio=1." << std::endl;
	// 	out << "graph [bgcolor=white]" << std::endl;
	// 	out << "node [shape=circle color=black]" << std::endl;
	// 	out << "edge [color=black]" << std::endl;
	//     }
	// };




	void setVertexProperty(Vertex vertex, VertexProperty value){
	    (*graph)[vertex] = value;
	}

	void setEdgeProperty(Edge edge, EdgeProperty value){
	    (*graph)[edge] = value;
	}



    };


} // namespace GraphPolicy
