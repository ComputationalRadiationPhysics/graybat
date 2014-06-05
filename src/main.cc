#include <types.hpp>
#include <Graph.hpp>
#include <BGL.hpp>
#include <Communicator.hpp>
#include <MPI.hpp>

#include <iostream> /* cout */
#include <tuple>    /* pair */
#include <vector>   /* vector   */
#include <array>    /* array */
#include <math.h>   /* ceil */
#include <time.h>   /* time */



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
// Vertex / Edge is a struct with at least UUID uuid public member
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

std::vector<EdgeDescriptor> generateFullyConnectedTopology(const unsigned verticesCount, std::vector<Vertex> &vertices){
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
    
    unsigned edgeCount = 0;    
    std::vector<EdgeDescriptor> edges;

    for(unsigned i = 0; i < vertices.size(); ++i){
	if(i != 0){
	    edges.push_back(std::make_tuple(vertices[i], vertices[0], Edge(edgeCount++)));
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

void nearestNeighborExchange(MPICommunicator &mpiCommunicator, BGLGraph &graph, std::vector<Vertex> myVertices){
    Context initialContext = mpiCommunicator.getInitialContext();
    // Distribute and announce vertices
    unsigned cid           = initialContext.getCommUUID();

    // Handle communication of vertices
    typedef std::array<unsigned, 1> Buffer;
    typedef MPICommunicator::Channel<Buffer> Channel;

    // Async send vertices data
    for(unsigned vertex_i = 0; vertex_i < myVertices.size(); vertex_i++){
    	Vertex myVertex = myVertices.at(vertex_i);
    	std::vector<std::pair<Vertex, Edge> > outEdges = graph.getOutEdges(myVertex);
    	Buffer outBuffer{{myVertex.uuid}};

    	// Send data to out edges
    	for(unsigned i = 0; i < outEdges.size(); ++i){
    	    Vertex dest = outEdges.at(i).first;
    	    Edge   e   = outEdges.at(i).second;
    	    mpiCommunicator.asyncSend(dest, e.uuid, initialContext, outBuffer);
    	}
    }

    // Sync recv vertices data
    for(unsigned vertex_i = 0; vertex_i < myVertices.size(); vertex_i++){
    	Vertex myVertex = myVertices.at(vertex_i);
    	std::vector<std::pair<Vertex, Edge> > inEdges  = graph.getInEdges(myVertex);
    	std::vector<Buffer>  inBuffers (inEdges.size(), Buffer{{0}});

    	// Recv data from in edges
    	for(unsigned i = 0; i < inEdges.size(); ++i){
    	    Vertex src = inEdges.at(i).first;
    	    Edge   e   = inEdges.at(i).second;
    	    mpiCommunicator.recv(src, e.uuid, initialContext, inBuffers[i]);
    	}
	
    	unsigned recvSum = 0;
    	for(Buffer b : inBuffers){
    	    recvSum += b[0];
    	}
    	std::cout << "CommID[" << cid << "] Vertex: " << myVertices[vertex_i].uuid << " NeighborIDSum: " << recvSum <<  std::endl;
	
    }

}

unsigned randomMaster(MPICommunicator &mpiCommunicator){
    Context context    = mpiCommunicator.getInitialContext();
    size_t contextSize = context.size();
    unsigned masterID  = context.getCommUUID();

    srand (time(NULL) + masterID);
    int random = rand();
    //std::cout << "ID " << masterID << " " <<random << std::endl;
    
    typedef std::vector<int> channelType;
    channelType sendData(1, random);
    channelType recvData(contextSize, 0);


    MPICommunicator::CollectiveChannel<channelType, channelType> gatherChannel(sendData, recvData, context);
    mpiCommunicator.allGather(gatherChannel);

    for(unsigned i = 0; i < recvData.size(); ++i){
	if(recvData[i] > random){
	    masterID = i;
	    random = recvData[i];
	}
    }

    return masterID;
}


/*******************************************************************************
 *
 * VERTEX DISTRIBUTION
 *
 *******************************************************************************/
std::vector<Vertex> distributeVerticesEvenly(MPICommunicator &mpiCommunicator, BGLGraph &graph){
    Context initialContext = mpiCommunicator.getInitialContext();

    // Distribute and announce vertices
    size_t contextSize     = initialContext.size();
    unsigned cid           = initialContext.getCommUUID();
    unsigned vertexCount   = graph.getVertices().size();
    unsigned maxVertex     = ceil((float)vertexCount / contextSize);

    std::vector<Vertex> myVertices;
    for(unsigned i = 0; i < maxVertex; ++i){
	unsigned vertex_i = cid + (i * contextSize);
	if(vertex_i >= vertexCount){
	    break;
	}
	else {
	    myVertices.push_back(graph.getVertices().at(vertex_i));
	}
	
    }
    mpiCommunicator.announce(myVertices, initialContext);

    return myVertices;
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
    //std::vector<EdgeDescriptor> edges = generateFullyConnectedTopology(10, vertices);
    //std::vector<EdgeDescriptor> edges = generateStarTopology(10, vertices);
    //std::vector<EdgeDescriptor> edges = generateHyperCubeTopology(8, vertices);
    std::vector<EdgeDescriptor> edges = generate2DMeshTopology(3, 4, vertices);
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
    std::vector<Vertex> myVertices = distributeVerticesEvenly(mpiCommunicator, myGraph);
    // broadcast(mpiCommunicator, myGraph);
    // broadcastP2P(mpiCommunicator, myGraph);
    // sumP2P(mpiCommunicator, myGraph);
    //nearestNeighborExchange(mpiCommunicator, myGraph, myVertices);
    std::cout << "Master: " << randomMaster(mpiCommunicator) << std::endl;
    

}

