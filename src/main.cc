#include <types.hpp>
#include <Graph.hpp>
#include <BGL.hpp>
#include <Communicator.hpp>
#include <MPI.hpp>

#include <iostream> /* cout */
#include <tuple>    /* pair */
#include <vector>   /* vector   */
#include <array>    /* array */
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
typedef typename MPICommunicator::Context          Context;
typedef typename MPICommunicator::BinaryOperations BinaryOperations;
typedef typename MPICommunicator::Event            Event;


/*******************************************************************************
 *
 * GRAPH AUXILARY
 *
 *******************************************************************************/
std::vector<Vertex> generateVertices(const size_t numVertices){
    std::vector<Vertex> vertices;
    for(unsigned i = 0; i < numVertices; ++i){
	vertices.push_back(Vertex(i));
    }
    return vertices;
}

std::vector<EdgeDescriptor> generateFullyConnected(const unsigned verticesCount, std::vector<Vertex> &vertices){
    vertices = generateVertices(verticesCount);
    std::cout << "Create fully connected with " << vertices.size() << " cells" << std::endl;

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

std::vector<EdgeDescriptor> generateStarTopology(const unsigned verticesCount, std::vector<Vertex> &vertices){
    vertices = generateVertices(verticesCount);
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

unsigned hammingDistance(unsigned a, unsigned b){
    unsigned abXor = a xor b;
    return (unsigned) __builtin_popcount(abXor);
}


std::vector<EdgeDescriptor> generateHyperCubeTopology(const unsigned dimension, std::vector<Vertex> &vertices){
    assert(dimension >= 1);
    std::vector<EdgeDescriptor> edges;

    unsigned verticesCount = pow(2, dimension);
    unsigned edgeCount = 0;
    vertices  = generateVertices(verticesCount);

    for(Vertex v1 : vertices){
	for(Vertex v2 : vertices){
	    if(hammingDistance(v1.uuid, v2.uuid) == 1){
		edges.push_back(std::make_tuple(v1, v2, Edge(edgeCount++)));
	    }

	}
    }
    
    return edges;
}

// TODO
// Make n-dimensional
// Make connected borders
std::vector<EdgeDescriptor> generate2DMeshTopology(const unsigned height, const unsigned width, std::vector<Vertex> &vertices){
    const unsigned verticesCount = height * width;
    vertices = generateVertices(verticesCount);
    std::vector<EdgeDescriptor> edges;

    unsigned edgeCount = 0;

    for(Vertex v: vertices){
	unsigned i    = v.uuid;

	if(i >= width){
	    unsigned up   = i - width;
	    edges.push_back(std::make_tuple(vertices[i], vertices[up], Edge(edgeCount++)));
	}

	if(i < (verticesCount - width)){
	    unsigned down = i + width;
	    edges.push_back(std::make_tuple(vertices[i], vertices[down], Edge(edgeCount++)));
	}


	if((i % width) != (width - 1)){
	    int right = i + 1;
	    edges.push_back(std::make_tuple(vertices[i], vertices[right], Edge(edgeCount++)));
	}

	if((i % width) != 0){
	    int left = i - 1;
	    edges.push_back(std::make_tuple(vertices[i], vertices[left], Edge(edgeCount++)));
	}
	

    }

    return edges;
}

/*******************************************************************************
 *
 * COMMUNICATION AUXILARY
 *
 *******************************************************************************/

/*
void broadcast(MPICommunicator &mpiCommunicator, BGLGraph &myGraph){
    Context initialContext = mpiCommunicator.getInitialContext();
    unsigned cid = initialContext.getCommUUID();
    mpiCommunicator.synchronize(initialContext);
    if(cid == 0) std::cerr << "C Broadcast example" << std::endl;
    std::string send;
    std::string recv;
    Vertex rootVertex = myGraph.getVertices().at(0);
    MPICommunicator::CollectiveChannel<std::string, std::string> broadcastChannel(send, recv, rootVertex, initialContext);
    if(cid == 0){
	send = "Hello World (Broadcast)";
	mpiCommunicator.broadcast(broadcastChannel);

    }
    else {
	recv = "00000000000000000000000";
	mpiCommunicator.broadcast(broadcastChannel);
	std::cerr << recv << std::endl;
    
    }

}

void broadcastP2P(MPICommunicator &mpiCommunicator, BGLGraph &myGraph){
    Context initialContext = mpiCommunicator.getInitialContext();
    unsigned cid = initialContext.getCommUUID();
    Vertex myVertex = myGraph.getVertices().at(cid);
    mpiCommunicator.synchronize(initialContext);
    if(cid == 0) std::cerr << "C Broadcast example p2p" << std::endl;

    if(cid == 0){
	int verticesCount = myGraph.getVertices().size();
	for(int i = 1; i < verticesCount; ++i){
	    std::string send = "Hello World (Direct)";
	    MPICommunicator::Channel<std::string>  directChannel(myGraph.getVertices().at(i), send, 0, initialContext);
	    mpiCommunicator.send(directChannel);
	}

    }
    else {
	std::string recv = "00000000000000000000";
	MPICommunicator::Channel<std::string>  directChannel(myVertex, recv, 0, initialContext);
	mpiCommunicator.recv(directChannel);
	std::cerr << recv << std::endl;
    }
}

void sumP2P(MPICommunicator &mpiCommunicator, BGLGraph &graph){
    Context initialContext = mpiCommunicator.getInitialContext();
    size_t contextSize = initialContext.size();
    unsigned cid = initialContext.getCommUUID();
    Vertex myVertex = graph.getVertices().at(cid);
    Vertex rootVertex = graph.getVertices().at(0);

    mpiCommunicator.announce(myVertex, initialContext);

    std::array<int, 1> data{{0}};

    int sum = 0;

    if(cid == 0){
	for(unsigned i = 1; i < contextSize; ++i){
	    MPICommunicator::Channel<std::array<int,1> > c (rootVertex, data, 0, initialContext);
	    mpiCommunicator.recv(c);
	    sum += data[0];
	    std::cout << data[0];
	    if(i != (contextSize - 1))
	       std::cout << " + ";
	}
	std::cout << " = " << sum << std::endl;

    }
    else {
	MPICommunicator::Channel<std::array<int,1> > c (rootVertex, data, 0, initialContext);
	data[0] = myVertex.uuid;
	mpiCommunicator.send(c);
    }

}
*/

void nearestNeighborExchange(MPICommunicator &mpiCommunicator, BGLGraph &graph){
    Context initialContext = mpiCommunicator.getInitialContext();
    unsigned cid           = initialContext.getCommUUID();
    Vertex myVertex        = graph.getVertices().at(cid);
    
    mpiCommunicator.announce(myVertex, initialContext);

    std::vector<std::pair<Vertex, Edge> > inEdges  = graph.getInEdges(myVertex);
    std::vector<std::pair<Vertex, Edge> > outEdges = graph.getOutEdges(myVertex);


    typedef std::array<unsigned, 1> Buffer;
    typedef MPICommunicator::Channel<Buffer> Channel;
    
    // Create data buffer
    std::vector<Buffer> inBuffers(inEdges.size(), Buffer{{0}});
    Buffer outBuffer{{myVertex.uuid}};

    // Send data to out edges
    for(unsigned i = 0; i < outEdges.size(); ++i){
	Vertex dest = outEdges.at(i).first;
	Edge   e   = outEdges.at(i).second;
	mpiCommunicator.asyncSend(dest, e.uuid, initialContext, outBuffer);
    }

    // Recv data from in edges
    unsigned inVertexSum = 0;
    for(unsigned i = 0; i < inEdges.size(); ++i){
	Vertex src = inEdges.at(i).first;
	Edge   e   = inEdges.at(i).second;
	mpiCommunicator.recv(src, e.uuid, initialContext, inBuffers[i]);
	inVertexSum += src.uuid;
    }

    // Sum up collected data
    unsigned recvSum = 0;
    for(Buffer b : inBuffers){
	recvSum += b[0];
    }
    
    std::cout << " recvSum: " << recvSum << " inVertexSum: " << inVertexSum<< std::endl;
    assert(recvSum == inVertexSum);

}
	
/*******************************************************************************
 *
 * MAIN
 *
 *******************************************************************************/
int main(){

    /***************************************************************************
     * Create graph
     ****************************************************************************/
    std::vector<Vertex> vertices;
    //std::vector<EdgeDescriptor> edges = generateFullyConnected(10, vertices);
    //std::vector<EdgeDescriptor> edges = generateStarTopology(10, vertices);
    //std::vector<EdgeDescriptor> edges = generateHyperCubeTopology(8, vertices);
    std::vector<EdgeDescriptor> edges = generate2DMeshTopology(3, 3, vertices);
    BGLGraph myGraph (edges, vertices);

    //myGraph.print();

    /***************************************************************************
     * Create communicator
     ****************************************************************************/
    MPICommunicator mpiCommunicator;

    /***************************************************************************
     * Example create new context
     ****************************************************************************/
    // std::vector<Vertex> contextVertices;
    // contextVertices.push_back(myGraph.getVertices().at(2));
    // contextVertices.push_back(myGraph.getVertices().at(3));
    // Context newContext = mpiCommunicator.getContext(contextVertices, initialContext);
    // if(newContext.valid()){
    // 	mpiCommunicator.announce(myVertices, newContext);
    // 	std::cout << "old context uuid: " << initialContext.getCommUUID() << " new context uuid:  " << newContext.getCommUUID() << std::endl;
    // }

    /***************************************************************************
     * Examples communication 
     ****************************************************************************/
    // broadcast(mpiCommunicator, myGraph);
    // broadcastP2P(mpiCommunicator, myGraph);
    // sumP2P(mpiCommunicator, myGraph);
    nearestNeighborExchange(mpiCommunicator, myGraph);
    

}

