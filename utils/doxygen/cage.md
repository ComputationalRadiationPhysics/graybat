Communication and Graph Environment
===================================

The communication and graph environment (cage) provides a graph-based
virtual overlay network which is implemented by the policy based
design. Taking this term to pieces, the cage is an interface which
provides communication methods on basis of an existing communication
library, where the possible paths of communication are described by a
graph.

The behavior of the cage need to be defined by the
[communication policy](utils/doxygen/communicationPolicy.md) and the
[graph policy](utils/doxygen/graphPolicy.md). These policies need to
be provided as template arguments to the cage class. The following
listing shows the configuration of the cage with predefined
communication and graph policies.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cc}
// STL
#include <iostream>

// GrayBat
#include <graybat.hpp>
#include <pattern/GridDiagonal.hpp>
#include <mapping/Consecutive.hpp>

// Predefined communication policy based on boost.MPI
typedef graybat::communicationPolicy::BMPI CommunicationPolicy;

// Predefined graph policy based on the boost graph library
typedef graybat::graphPolicy::BGL<> GraphPolicy;

// Define Cage through policies
typedef graybat::Cage<CommunicatonPolicy, GraphPolicy> Cage;
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Additionally, a graph functor which describes the
[communication pattern](utils/doxygen/communicationPattern.md) need to
be provided.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cc}
// Instanciate cage with a communication pattern 
Cage cage(graybat::pattern::GridDiagonal(100,100))
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


## Mapping Operations ##

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cc}
typedef Cage::Vertex Vertex;

// Distributes vertices of the graph based on a mapping functor
cage.distribute(graybat::mapping::Consecutive());

// Iterate over all vertices that I host
for(Vertex v: cage.hostedVertices){
	std::cout << v.id << std::endl;
}

// Define vertex with id 0 as root vertex
Vertex rootVertex = cage.getVertex(0);

// Do I host the root vertex ?
if(cage.peerHostVertex()){
	std::cout << "Huraaaaaay, I am the boss" << std::endl
}
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


## Graph Operations ##

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cc}
typedef Cage::Edge Edge;


// Iterate over hosted vertices
for(Vertex v: cage.hostedVertices){
	// Retrieve outgoing edges of a vertex
	for(auto outEdge : cage.getOutEdges(v)){
    	Vertex destVertex = outEdge.first;
		Edge   destEdge   = outEdge.second;
		}
	// Retrieve incoming vertices of a vertex
	for(auto inEdge : cage.getInEdges(v)){
    	Vertex srcVertex = inEdge.first;
		Edge   srcEdge   = inEdge.second;
		}
	// Retrieve adjacent vertices
	for(auto adjVertex : cage.getAdjacentVertices(v)){
	}

}
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


## Point to Point Communication Operations ##

