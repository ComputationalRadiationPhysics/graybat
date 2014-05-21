#include <types.hpp>
#include <Graph.hpp>
#include <BGL.hpp>
#include <Communicator.hpp>
#include <MPI.hpp>

#include <iostream>
#include <tuple>
#include <vector>


/*******************************************************************************
 *
 * CONFIGURATION
 *
 ******************************************************************************/
// Graph and topologie
typedef GraphPolicy::BGL<GridProperty, GridConnection> BGL;
typedef Graph<BGL> GridTopologie;
typedef typename GridTopologie::Vertex   Vertex;
typedef typename GridTopologie::Edge     Edge;
typedef std::tuple<Vertex, Vertex, Edge> EdgeDescriptor;

// Communicator
typedef Communicator<CommunicationPolicy::MPI, Vertex> MPICommunicator;
typedef MPICommunicator::Channel<char> CharChannel;
typedef typename MPICommunicator::Context Context;


/*******************************************************************************
 *
 * AUXILARY
 *
 *******************************************************************************/
std::vector<Vertex> generateVertices(const size_t numVertices){
    std::vector<Vertex> vertices;
    for(unsigned i = 0; i < numVertices; ++i){
	vertices.push_back(Vertex(i));
    }
    return vertices;
}

std::vector<EdgeDescriptor> generateEdges(const std::vector<Vertex> vertices){
    std::cout << "Create grid with " << vertices.size() << " cells" << std::endl;

    unsigned edgeCount = 0;    
    std::vector<EdgeDescriptor> edges;

    for(unsigned i = 0; i < vertices.size(); ++i){
	for(unsigned j = 0; j < vertices.size(); ++j){
	    if(vertices[i].uuid == vertices[j].uuid){
		continue;
	    } 
	    else {
		edges.push_back(std::make_tuple(vertices[i], vertices[j], Edge(edgeCount++)));
	    }
	}

    }

    return edges;
}

void printVertices(std::vector<Vertex> vertices){
    std::cout << "vertices = ";
    for(Vertex v: vertices){
	std::cout << v.uuid << " ";
    }
    std::cout << std::endl;
}

void handle(){
    std::cout << "handle called" << std::endl;

}

/*******************************************************************************
 *
 * MAIN
 *
 *******************************************************************************/
int main(){


    // Create graph
    const unsigned numVertices = 2;
    auto vertices = generateVertices(numVertices);
    auto edges    = generateEdges(vertices);
    GridTopologie topologie (edges, vertices);

    // // Print all vertices
    // std::cout << "All vertices properties :" << std::endl;
    // printVertices(topologie.getVertices());


    // MPI 
    MPICommunicator mpiCommunicator;
    Context fullContext = mpiCommunicator.getContext();
    int cid = mpiCommunicator.getCommunicatorID(fullContext);

    // Get grid to work on
    Vertex myVertex = topologie.getVertices().at(cid);
    mpiCommunicator.announce(myVertex, fullContext);

    // Simple send / recv example
    const size_t size = 512;
    char data[size] = "Hello World";
    if(cid == 0){
	for(std::pair<Vertex, Edge> outEdge : topologie.getOutEdges(myVertex)){
	    Vertex adjVertex = outEdge.first;
	    CharChannel sendChannel(myVertex, adjVertex, size, data, fullContext);
	    mpiCommunicator.send(sendChannel, handle);
	}
    }
    else {
	 for(std::pair<Vertex, Edge> outEdge : topologie.getInEdges(myVertex)){
	    Vertex adjVertex = outEdge.first;
	    CharChannel recvChannel(adjVertex, myVertex, size, data, fullContext);
	    mpiCommunicator.recv(recvChannel, handle);
	 }
    
    }

    //Get adjacent Segments
    // Vertex targetGrid;
    // Edge edge;
    // for(std::pair<Vertex, Edge> outEdge : topologie.getOutEdges(myVertex)){
    // 	const size_t size = 512;
    // 	char data[size] = "Hello World";
    // 	std::tie(targetGrid, edge) = outEdge;
    // 	std::cout << "Connected to vertex " << targetGrid.uuid << " by edge " << edge.uuid << std::endl;

    // 	CharChannel sendChannel(myVertex, targetGrid, size, data, allNodes);
    // 	CharChannel recvChannel(myVertex, targetGrid, size, data, allNodes);
    // 	mpiCommunicator.send(sendChannel, handle);
    // 	mpiCommunicator.recv(recvChannel, handle);

    // }

}
