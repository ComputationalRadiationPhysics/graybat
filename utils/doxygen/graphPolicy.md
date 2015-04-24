Graph Policy
============

[cage]:utils/doxygen/cage.md

The graph policy is a class which implements the graph interface of
its host class ([cage]). A graph is defined by its vertex and
edge property through template arguments. The following defintion
of a graph defines a graph were a city property represents a vertex
and a road represents an edge:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cc}
typedef graybat::graphPolicy::BGL<City, Road> CityGraph;
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The basic member of a property is an unique identifier within the
graph.  This id is used by the [cage] to map vertices to peers.  The
following property is a very simple and basic proptert for edges and
vertex. Therefore, it is called SimpleProperty:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cc}
struct SimpleProperty{
    typedef unsigned ID;
    SimpleProperty() : SimpleProperty(0){}
    SimpleProperty(ID id) : id(id){}

    ID id;
};

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The SimpleProperty can 
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cc}
struct ComplexProperty::SimpleProperty{
	// Property specific data
};

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~



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


