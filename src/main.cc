#include <types.hpp>
#include <Graph.hpp>
#include <BGL.hpp>
#include <Communicator.hpp>
#include <MPI.hpp>

#include <iostream> /* cout */
#include <tuple>    /* pair */
#include <vector>   /* vector   */
#include <mpi.h>    /* MPI_INIT */


/*******************************************************************************
 *
 * CONFIGURATION
 *
 ******************************************************************************/
// Graph
// Property is struct with at least UUID uuid and typename UUDI
typedef GraphPolicy::NoProperty NoProperty; 
typedef GraphPolicy::BGL<NoProperty, NoProperty> BGL;
typedef Graph<BGL>                                     BGLGraph;
typedef typename BGLGraph::Vertex                      Vertex;
typedef typename BGLGraph::Edge                        Edge;
typedef std::tuple<Vertex, Vertex, Edge>               EdgeDescriptor;

// Communicator
// Vertex ist struct with at least UUID uuid member
typedef CommunicationPolicy::MPI                   Mpi;
typedef Communicator<Mpi, Vertex>                  MPICommunicator;
typedef MPICommunicator::Channel<char>             CharChannel;
typedef MPICommunicator::CollectiveChannel<char>   CharCollectiveChannel;
typedef typename MPICommunicator::Context          Context;
typedef typename MPICommunicator::BinaryOperations BinaryOperations;


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

std::vector<EdgeDescriptor> generateFullyConnected(const std::vector<Vertex> vertices){
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

std::vector<EdgeDescriptor> generateStarTopology(const std::vector<Vertex> vertices){
    std::cout << "Create star with " << vertices.size() << " cells" << std::endl;

    unsigned edgeCount = 0;    
    std::vector<EdgeDescriptor> edges;

    for(unsigned i = 0; i < vertices.size(); ++i){
	if(i != 0){
	    edges.push_back(std::make_tuple(vertices[0], vertices[i], Edge(edgeCount++)));
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
int main(int argc, char **argv){
    // Init MPI
    MPI_Init(&argc, &argv);

    // Create graph
    const unsigned numVertices        = 4;
    std::vector<Vertex> vertices      = generateVertices(numVertices);
    std::vector<EdgeDescriptor> edges = generateStarTopology(vertices);
    BGLGraph myGraph (edges, vertices);

    // MPI 
    MPICommunicator mpiCommunicator;
    Context initialContext = mpiCommunicator.getInitialContext();
    int cid = initialContext.uuid();

    // Get vertex to work on
    Vertex myVertex = myGraph.getVertices().at(cid);

    // Announce own vertices
    std::vector<Vertex> myVertices;
    myVertices.push_back(myVertex);

    mpiCommunicator.announce(myVertices, initialContext);

    // Create Context
    // std::vector<Vertex> contextVertices;
    // contextVertices.push_back(myGraph.getVertices().at(2));
    // contextVertices.push_back(myGraph.getVertices().at(3));
    // Context newContext = mpiCommunicator.getContext(contextVertices, initialContext);

    // if(newContext.valid()){
    // 	std::cout << "old context uuid: " << initialContext.uuid() << " new context uuid:  " << newContext.uuid() << std::endl;
    // }


    // Simple broadcast example
    const size_t size = 512;
    if(cid == 0){
    	char data[size] = "Hello World";
    	CharCollectiveChannel broadcastChannel(data, data, size, myGraph.getVertices().at(0), initialContext);
    	mpiCommunicator.broadcast(broadcastChannel);

	CharChannel testChannel(myGraph.getVertices().at(0),
				myGraph.getVertices().at(1),
				data, 
				size,
				0,
				initialContext);

	mpiCommunicator.send(testChannel);


    }
    else {
    	char data[size];
    	CharCollectiveChannel broadcastChannel(data, data, size, myGraph.getVertices().at(0), initialContext);
    	mpiCommunicator.broadcast(broadcastChannel);
    	std::cout << broadcastChannel.recvData << std::endl;

	CharChannel testChannel(myGraph.getVertices().at(0),
				myGraph.getVertices().at(1),
				data, 
				size,
				0,
				initialContext);

	mpiCommunicator.recv(testChannel);

    
    }

}

