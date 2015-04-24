Graph Policy
============

[cage]:utils/doxygen/cage.md

The graph policy is a class which implements the graph interface of
its host class ([cage]).


* pattern
* properties

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cc}
namespace graybat {

	namespace graphPolicy {

		template <class T_VertexProperty, class T_EdgeProperty>
		class GraphPolicyI {
		
			// Type definitions
			typedef T_VertexProperty                                                Vertex;
			typedef T_EdgeProperty                                                  Edge;
			typedef std::pair<unsigned, unsigned>                                   EdgeDescription;
			typedef std::pair<std::vector<unsigned>, std::vector<EdgeDescription> > GraphDescription;
			
			// Construct the graph from graph description
			GraphPolicyI(GraphDescription description) {...}
			
			// Graph identifier
			unsigned id;
			
			/*******************************************************************
			 * GRAPH OPERATIONS
			 ******************************************************************/
			std::vector<Vertex> getVertices() {...}

			std::vector<Vertex> getAdjacentVertices(const Vertex vertex) {...}

			std::vector<std::pair<Vertex, Edge> > getOutEdges(const Vertex vertex) {...}

			std::vector<std::pair<Vertex, Edge> > getInEdges(const Vertex vertex) {...}
		}
	}
}

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
