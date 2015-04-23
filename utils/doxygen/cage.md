Communication and Graph Environment
===================================

[cage]: @ref graybat::Cage
[communication policy]:utils/doxygen/communicationPolicy.md
[graph policy]:utils/doxygen/graphPolicy.md
[pattern]:utils/doxygen/communicationPattern.md
[mapping]:utils/doxygen/mapping.md


The communication and graph environment ([cage]) provides a
graph-based virtual overlay network which is implemented by the policy
based design. Taking this term to pieces, the [cage] is an interface
which provides communication methods on basis of an existing
communication library, where the possible paths of communication are
described by a graph.

The behavior of the [cage] need to be defined by a
[communication policy] and a [graph policy]. These policies need to be
provided as template arguments to the [cage] class.
The following listings show examples on how to use and how to configure
the [cage] with predefined [communication policy] and [graph policy].

## Configure the GrayBat Cage ##

1. Include GrayBat umbrella header and predefined functors for
   graph generation and graph mapping.
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cc}
#include <graybat.hpp>
#include <pattern/GridDiagonal.hpp>
#include <mapping/Consecutive.hpp>
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

2. Define communication policy to use (boost.MPI)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cc}
typedef graybat::communicationPolicy::BMPI CommunicationPolicy;
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

3. Define graph policy to use (boost graph library)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cc}
typedef graybat::graphPolicy::BGL<> GraphPolicy;
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

4. Define cage through policies
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cc}
typedef graybat::Cage<CommunicatonPolicy, GraphPolicy> Cage;
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

5. Create Cage instance with graph creation functor that describes the
   communication [pattern]
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cc}
Cage cage(graybat::pattern::GridDiagonal(100,100))
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


## Mapping Operations ##

1. Distributes vertices of the graph based on a [mapping] functor
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cc}
cage.distribute(graybat::mapping::Consecutive());
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

2. The vertices mapped to the own peer are called hosted vertices
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cc}
typedef Cage::Vertex Vertex;

// Iterate over all vertices that I host
for(Vertex v: cage.hostedVertices){
	std::cout << v.id << std::endl;
}
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

## Graph Operations ##

1. Retrieve outgoing edges of hosted vertices
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cc}
typedef Cage::Edge Edge;
for(Vertex v: cage.hostedVertices){
	for(auto outEdge : cage.getOutEdges(v)){
    	Vertex destVertex = outEdge.first;
		Edge   destEdge   = outEdge.second;
	}
}
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

2. Retrieve incoming edges of hosted vertices
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cc}
for(Vertex v: cage.hostedVertices){
	for(auto inEdge : cage.getInEdges(v)){
    	Vertex srcVertex = inEdge.first;
		Edge   srcEdge   = inEdge.second;
	}
}
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


## Point to Point Communication Operations ##

1. Send synchronous and asynchronous data
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cc}
typedef Cage::Edge  Edge;
typedef Cage::Event Event;
for(Vertex v: cage.hostedVertices){
	for(auto outEdge : cage.getOutEdges(v)){
    	Vertex destVertex = outEdge.first;
		Edge   destEdge   = outEdge.second;
		// some data
		std::vector<int> data(100,1);
		// synchronous
		cage.send(destVertex, destEdge, data);
		// asynchronous
		Event e = cage.asyncSend(destVertex, destEdge, data);
		// wait for event
		e.wait();
	}
}
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


1. Receive synchronous and asynchronous data
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cc}
typedef Cage::Edge  Edge;
typedef Cage::Event Event;
for(Vertex v: cage.hostedVertices){
	for(auto inEdge : cage.getInEdges(v)){
    	Vertex srcVertex = inEdge.first;
		Edge   srcEdge   = inEdge.second;
		// some data
		std::vector<int> data(100,1);
		// synchronous
		cage.recv(destVertex, destEdge, data);
		// asynchronous
		Event e = cage.asyncRecv(destVertex, destEdge, data);
		// wait for event
		e.wait();
	}
}
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

## Collective Communication Operations ##

1. Reduce vector of data with binary operator and receive by some root vertex
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cc}
Vertex rootVertex = cage.getVertex(0);

std::vector<int> send(100);
std::vector<int> recv(100);

// Each vertex need to reduce its data
for(Vertex vertex: cage.hostedVertices){
	cage.reduce(rootVertex, vertex, std::plus<int>, send, recv);
}
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

2. Reduce vector of data and receive them by every vertex
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cc}
std::vector<int> send(100);
std::vector<int> recv(100);

// Each vertex need to reduce its data
for(Vertex vertex: cage.hostedVertices){
	cage.allReduce(rootVertex, vertex, std::plus<int>, send, recv);
}
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


3. Root vertex collects data from each vertex
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cc}
Vertex rootVertex = cage.getVertex(0);

std::vector<int> send(10);
std::vector<int> recv(10 * cage.getVertices().size());

// Each vertex need to send its data
for(Vertex vertex: cage.hostedVertices){
	cage.gather(rootVertex, vertex, send, recv);
}
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


4. Data is send to all vertices
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cc}
std::vector<int> send(10);
std::vector<int> recv(10 * cage.getVertices().size());

// Each vertex need to send its data
for(Vertex vertex: cage.hostedVertices){
	cage.allGather(vertex, send, recv);
}
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

5. Synchronize all peers (including their hosted vertices)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cc}
cage.synchronize();
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

