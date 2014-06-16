#include <types.hpp>
#include <Graph.hpp>
#include <BGL.hpp>
#include <Communicator.hpp>
#include <GraphCommunicator.hpp>
#include <MPI.hpp>
#include <NameService.hpp>

#include <iostream> /* cout */
#include <tuple>    /* pair */
#include <vector>   /* vector   */
#include <array>    /* array */
#include <math.h>   /* ceil */
#include <time.h>   /* time */


#define FALSE 0
#define TRUE  1

/*******************************************************************************
 *
 * CONFIGURATION
 *
 ******************************************************************************/
// Graph
// Property is struct with at least ID id and typename UUDI
typedef GraphPolicy::NoProperty NoProperty; 
typedef GraphPolicy::BGL<NoProperty, NoProperty> BGL;
typedef Graph<BGL>                                     BGLGraph;
typedef typename BGLGraph::Vertex                      Vertex;
typedef typename BGLGraph::Edge                        Edge;
typedef std::tuple<Vertex, Vertex, Edge>               EdgeDescriptor;

// Communicator
// Vertex / Edge is a struct with at least ID id public member
typedef CommunicationPolicy::MPI                   Mpi;
typedef Communicator<Mpi>                          MPICommunicator;
typedef typename MPICommunicator::Context          Context;
typedef typename MPICommunicator::BinaryOperations BinaryOperations;
typedef typename MPICommunicator::Event            Event;

typedef NameService<BGLGraph, MPICommunicator>     NS;
typedef GraphCommunicator<BGLGraph, MPICommunicator, NS> GC;

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
	    if(vertices[i].id == vertices[j].id){
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
	    if(hammingDistance(v1.id, v2.id) == 1){
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
	unsigned i    = v.id;

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

template<typename T_Communicator, typename T_Graph>
void nearestNeighborExchange(T_Communicator &communicator, T_Graph &graph, std::vector<typename T_Graph::Vertex> myVertices){
    typedef typename T_Graph::Vertex Vertex;
    typedef typename T_Graph::Edge   Edge;
    typedef std::array<unsigned, 1>  Buffer;

    // Async send vertices data
    for(Vertex myVertex : myVertices){
    	std::vector<std::pair<Vertex, Edge> > outEdges = graph.getOutEdges(myVertex);
    	Buffer outBuffer{{myVertex.id}};

    	// Send data to out edges
	for(std::pair<Vertex, Edge> outEdge : outEdges){
    	    Vertex dest = outEdge.first;
    	    Edge   e    = outEdge.second;
    	    communicator.asyncSend(dest, e, outBuffer);
	}

    }

    // Sync recv vertices data
    for(Vertex myVertex : myVertices){
    	std::vector<std::pair<Vertex, Edge> > inEdges  = graph.getInEdges(myVertex);
    	std::vector<Buffer>  inBuffers (inEdges.size(), Buffer{{0}});


    	// Recv data from in edges
	for(unsigned i = 0 ; i < inBuffers.size(); ++i){
    	    Vertex src = inEdges[i].first;
    	    Edge   e   = inEdges[i].second;
    	    communicator.recv(src, e, inBuffers[i]);
    	}
	
    	unsigned recvSum = 0;
    	for(Buffer b : inBuffers){
    	    recvSum += b[0];
    	}
    	std::cout << "Vertex: " << myVertex.id << " NeighborIDSum: " << recvSum <<  std::endl;
	
    }

}

template<typename T_Communicator, typename T_Graph>
void reduceVertexIDs(T_Communicator &communicator, T_Graph &graph, std::vector<typename T_Graph::Vertex> myVertices){
    typedef typename T_Graph::Vertex Vertex;
    typedef typename T_Graph::Edge   Edge;

    Vertex rootVertex = graph.getVertices().at(0);
    unsigned recvData;

    for(Vertex vertex : myVertices){
	std::vector<unsigned> sendData(1, vertex.id);
	communicator.reduce(rootVertex, sendData, recvData);
    }
    
    for(Vertex vertex : myVertices){
	if(vertex.id == rootVertex.id){
	    std::cout << "Reduce: " << recvData << std::endl;

	}

    }
    
    

}

// unsigned randomComm(MPICommunicator &mpiCommunicator){
//     Context context    = mpiCommunicator.getGlobalContext();
//     size_t contextSize = context.size();
//     unsigned masterID  = context.getCommID();

//     srand (time(NULL) + masterID);
//     int random = rand();
    
//     std::vector<int> sendData(1, random);
//     std::vector<int> recvData(contextSize, 0);

//     mpiCommunicator.allGather(context, sendData, recvData);

//     for(unsigned i = 0; i < recvData.size(); ++i){
// 	if(recvData[i] > random){
// 	    masterID = i;
// 	    random = recvData[i];
// 	}
//     }

//     return masterID;
// }

// std::vector<Vertex> occupyRandomVertex(MPICommunicator &mpiCommunicator, BGLGraph &myGraph, std::vector<Vertex> myVertices, unsigned masterID){
//     Context context = mpiCommunicator.getGlobalContext();
//     unsigned cid    = context.getCommID();
//     std::array<unsigned, 1> randomVertex{{0}};
//     std::array<char, 1> iHaveVertex{{FALSE}};
//     std::vector<char> whoHasVertex(context.size(), FALSE);

//     if(cid == masterID){
//     	randomVertex[0] = rand() % myGraph.getVertices().size();
// 	mpiCommunicator.broadcast(masterID, context, randomVertex);
// 	mpiCommunicator.gather(masterID, context, iHaveVertex, whoHasVertex);
	
//     	for(unsigned i = 0; i < whoHasVertex.size(); ++i){
//     	    if(whoHasVertex[i] == TRUE){
//     		Vertex v = myGraph.getVertices().at(randomVertex[0]);
//     		myVertices.push_back(v);
//     		std::cout << "Master " << masterID << " occupied Vertex with id " << v.id << std::endl;
//     		break;
//     	    }
//     	}

//     }
//     else {
// 	mpiCommunicator.broadcast(masterID, context, randomVertex);
  
//     	for(unsigned i = 0; i < myVertices.size(); ++i){
//     	    if(myVertices[i].id == randomVertex[0]){
//     		iHaveVertex[0] = TRUE;
//     		myVertices.erase(myVertices.begin() + i);
// 		break;
//     	    }
//     	}
// 	mpiCommunicator.gather(masterID, context, iHaveVertex, whoHasVertex);
//     }

//     return myVertices;
// }


/*******************************************************************************
 *
 * VERTEX DISTRIBUTION
 *
 *******************************************************************************/
template<typename T_Graph>
std::vector<Vertex> distributeVerticesEvenly(const unsigned processID, const unsigned processCount, T_Graph &graph){
    typedef typename T_Graph::Vertex Vertex;
    // Distribute and announce vertices
    unsigned vertexCount   = graph.getVertices().size();
    unsigned maxVertex     = ceil((float)vertexCount / processCount);

    std::vector<Vertex> myVertices;
    for(unsigned i = 0; i < maxVertex; ++i){
	unsigned vertex_i = processID + (i * processCount);
	if(vertex_i >= vertexCount){
	    break;
	}
	else {
	    myVertices.push_back(graph.getVertices().at(vertex_i));
	}
	
    }
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
    std::vector<Vertex> myVertices;
    //std::vector<EdgeDescriptor> edges = generateFullyConnectedTopology(10, vertices);
    //std::vector<EdgeDescriptor> edges = generateStarTopology(10, vertices);
    //std::vector<EdgeDescriptor> edges = generateHyperCubeTopology(8, vertices);
    std::vector<EdgeDescriptor> edges = generate2DMeshTopology(1, 100, vertices);
    BGLGraph myGraph (edges, vertices);


    /***************************************************************************
     * Create communicator
     ****************************************************************************/
    MPICommunicator myCommunicator;
    NS nameService(myGraph, myCommunicator);
    GC myGraphCommunicator(myCommunicator, nameService);


    /***************************************************************************
     * Example create new context
     ****************************************************************************/

    // std::vector<Vertex> contextVertices;
    // contextVertices.push_back(myGraph.getVertices().at(2));
    // contextVertices.push_back(myGraph.getVertices().at(3));

    // myGraph.createSubGraph(contextVertices);
    
    // Context newContext = myCommunicator.getContext(contextVertices, context);
    // if(newContext.valid()){
    // 	myCommunicator.announce(myVertices, newContext);
    // 	std::cout << "old context id: " << context.getCommID() << " new context id:  " << newContext.getCommID() << std::endl;
    // }

    /***************************************************************************
     * Examples communication 
     ****************************************************************************/
    unsigned myProcessID  = myCommunicator.getGlobalContext().getCommID();
    unsigned processCount = myCommunicator.getGlobalContext().size();

    myVertices = distributeVerticesEvenly(myProcessID, processCount, myGraph);
    nameService.announce(myVertices);


    //nearestNeighborExchange(myGraphCommunicator, myGraph, myVertices);
    reduceVertexIDs(myGraphCommunicator, myGraph, myVertices);



    // unsigned masterID = 0;

    // myVertices = distributeVerticesEvenly(myCommunicator, myGraph);

    // masterID = randomComm(myCommunicator);

    // myVertices = occupyRandomVertex(myCommunicator, myGraph, myVertices, masterID);
    // myVertices = occupyRandomVertex(myCommunicator, myGraph, myVertices, masterID);
    

    // nearestNeighborExchange(myCommunicator, myGraph, myVertices);

}

