#include <types.hpp>
#include <Graph.hpp>
#include <BGL.hpp>
#include <Communicator.hpp>
#include <GraphCommunicator.hpp>
#include <MPI.hpp>
#include <NameService.hpp>
#include <dout.hpp>

#include <iostream>   /* std::cout */
#include <tuple>      /* std::pair */
#include <vector>     /* std::vector   */
#include <array>      /* std::array */
#include <math.h>     /* std::ceil */
#include <time.h>     /* std::time */
#include <functional> /* std::plus */
#include <unistd.h>   /* usleep */


/*******************************************************************************
 *
 * GRAPH AUXILARY
 *
 *******************************************************************************/
template<typename T_Vertex>
std::vector<T_Vertex> generateVertices(const size_t numVertices){
    std::vector<T_Vertex> vertices;
    for(unsigned i = 0; i < numVertices; ++i){
	vertices.push_back(T_Vertex(i));
    }
    return vertices;
}

// std::vector<EdgeDescriptor> generateFullyConnectedTopology(const unsigned verticesCount, std::vector<Vertex> &vertices){
//     vertices = generateVertices<Vertex>(verticesCount);
//     std::cout << "Create fully connected with " << vertices.size() << " cells" << std::endl;

//     unsigned edgeCount = 0;    
//     std::vector<EdgeDescriptor> edges;

//     for(unsigned i = 0; i < vertices.size(); ++i){
// 	for(unsigned j = 0; j < vertices.size(); ++j){
// 	    if(vertices[i].id == vertices[j].id){
// 		continue;
// 	    } 
// 	    else {
// 		edges.push_back(std::make_tuple(vertices[i], vertices[j], Edge(edgeCount++)));
// 	    }
// 	}

//     }

//     return edges;
// }

// std::vector<EdgeDescriptor> generateStarTopology(const unsigned verticesCount, std::vector<Vertex> &vertices){
//     vertices = generateVertices<Vertex>(verticesCount);
    
//     unsigned edgeCount = 0;    
//     std::vector<EdgeDescriptor> edges;

//     for(unsigned i = 0; i < vertices.size(); ++i){
// 	if(i != 0){
// 	    edges.push_back(std::make_tuple(vertices[i], vertices[0], Edge(edgeCount++)));
// 	}
		
//     }

//     return edges;
// }

unsigned hammingDistance(unsigned a, unsigned b){
    unsigned abXor = a xor b;
    return (unsigned) __builtin_popcount(abXor);
}


// std::vector<EdgeDescriptor> generateHyperCubeTopology(const unsigned dimension, std::vector<Vertex> &vertices){
//     assert(dimension >= 1);
//     std::vector<EdgeDescriptor> edges;

//     unsigned verticesCount = pow(2, dimension);
//     unsigned edgeCount = 0;
//     vertices  = generateVertices<Vertex>(verticesCount);

//     for(Vertex v1 : vertices){
// 	for(Vertex v2 : vertices){
// 	    if(hammingDistance(v1.id, v2.id) == 1){
// 		edges.push_back(std::make_tuple(v1, v2, Edge(edgeCount++)));
// 	    }

// 	}
//     }
    
//     return edges;
// }

// TODO
// Make n-dimensional
// Make connected borders
template<typename T_Graph>
std::vector<typename T_Graph::EdgeDescriptor> generate2DMeshTopology(const unsigned height, const unsigned width, std::vector<typename T_Graph::Vertex> &vertices){
    typedef typename T_Graph::Vertex Vertex;
    typedef typename T_Graph::Edge Edge;
    typedef typename T_Graph::EdgeDescriptor EdgeDescriptor;
    const unsigned verticesCount = height * width;
    vertices = generateVertices<Vertex>(verticesCount);
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

template<typename T_Graph>
std::vector<typename T_Graph::EdgeDescriptor> generate2DMeshDiagonalTopology(const unsigned height, const unsigned width, std::vector<typename T_Graph::Vertex> &vertices){
    typedef typename T_Graph::Vertex Vertex;
    typedef typename T_Graph::Edge Edge;
    typedef typename T_Graph::EdgeDescriptor EdgeDescriptor;
    const unsigned verticesCount = height * width;
    vertices = generateVertices<Vertex>(verticesCount);
    std::vector<EdgeDescriptor> edges;

    unsigned edgeCount = 0;

    for(Vertex v: vertices){
	unsigned i    = v.id;

	    // UP
	    if(i >= width){
		unsigned up   = i - width;
		edges.push_back(std::make_tuple(vertices[i], vertices[up], Edge(edgeCount++)));
	    }

	    // UP LEFT
	    if(i >= width and (i % width) != 0){
		unsigned up_left   = i - width - 1;
		edges.push_back(std::make_tuple(vertices[i], vertices[up_left], Edge(edgeCount++)));
	    }

	    // UP RIGHT
	    if(i >= width and (i % width) != (width - 1)){
		unsigned up_right   = i - width + 1;
		edges.push_back(std::make_tuple(vertices[i], vertices[up_right], Edge(edgeCount++)));
	    }

	    // DOWN
	    if(i < (verticesCount - width)){
		unsigned down = i + width;
		edges.push_back(std::make_tuple(vertices[i], vertices[down], Edge(edgeCount++)));
	    }

	    // DOWN LEFT
	    if(i < (verticesCount - width) and (i % width) != 0){
		unsigned down_left = i + width - 1;
		edges.push_back(std::make_tuple(vertices[i], vertices[down_left], Edge(edgeCount++)));
	    }

	    // DOWN RIGHT
	    if(i < (verticesCount - width) and (i % width) != (width - 1)){
		unsigned down_right = i + width + 1;
		edges.push_back(std::make_tuple(vertices[i], vertices[down_right], Edge(edgeCount++)));
	    }

	    // RIGHT
	    if((i % width) != (width - 1)){
		int right = i + 1;
		edges.push_back(std::make_tuple(vertices[i], vertices[right], Edge(edgeCount++)));
	    }

	    // LEFT
	    if((i % width) != 0){
		int left = i - 1;
		edges.push_back(std::make_tuple(vertices[i], vertices[left], Edge(edgeCount++)));
	    }

	    

    }
    
    for(unsigned i = 0; i < width; ++i){
	
    }
    

    return edges;
}

template <typename T_Graph>
void printVertexDistribution(const std::vector<typename T_Graph::Vertex>& vertices,const T_Graph& graph, const unsigned commID){
    typedef typename T_Graph::Vertex Vertex;
    std::cout << "[" <<  commID << "] " << "Graph: " << graph.id << " " << "Vertices: ";
    for(Vertex v : vertices){
	std::cout << v.id << " ";
    }
    std::cout << std::endl;

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

    	//Send data to out edges
    	for(std::pair<Vertex, Edge> outEdge : outEdges){
    	    Vertex dest = outEdge.first;
    	    Edge   e    = outEdge.second;
    	    communicator.asyncSend(graph, dest, e, outBuffer);// <== BUGGY leads to segfault on some configuration
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
	    communicator.recv(graph, src, e, inBuffers[i]); // <== BUGGY leads to segfault on some configuration
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

    Vertex rootVertex = graph.getVertices().at(0);
    unsigned recvData = 0;
    std::vector<unsigned> sendData(1,0);

    for(Vertex vertex : myVertices){
    	sendData[0] = vertex.id;
    	communicator.reduce(rootVertex, vertex, graph, std::plus<unsigned>(), sendData, recvData);
    }
    
    for(Vertex vertex : myVertices){
    	if(vertex.id == rootVertex.id){
	    std::cout << "Reduce graph " << graph.id << ": " << recvData << std::endl;

    	}

    }
    
}

// Collective
// template<typename T_Communicator>
// unsigned randomNumber(T_Communicator &communicator, const Context &context){
//     srand (time(NULL) + context.getCommID());
//     std::vector<unsigned> ownRandom(1, rand());
//     unsigned random = 0;

//     if(context.valid()){
// 	std::vector<unsigned> recvData(context.size(), 0);

// 	communicator.allGather(context, ownRandom, recvData);

// 	for(unsigned i = 0; i < recvData.size(); ++i){
// 	    random += recvData[i];
// 	}
//     }
//     return random;
// }

// Collective
// template<typename T_Communicator, typename T_NameService, typename T_Graph>
// typename T_Communicator::CommID randomHostCommID(T_Communicator &communicator, T_NameService &nameService, T_Graph &graph){

//     std::vector<CommID> commIDs = nameService.getGraphHostCommIDs(graph);
//     Context context = nameService.getGraphContext(graph);
//     unsigned random = randomNumber(communicator, context);
//     return commIDs.at(random % commIDs.size());
// }

// template<typename T_Communicator, typename T_Graph>
// typename T_Graph::Vertex randomVertex( T_Communicator& communicator, T_Graph& graph, const std::vector<typename T_Graph::Vertex> vertices ){
//     typedef T_Graph                Graph;
//     typedef typename Graph::Vertex Vertex;
//     typedef typename Vertex::ID    VertexID;
    
//     VertexID vertexID  = 0;
//     unsigned myRandom = 0;
//     std::vector<unsigned> recvData;

//     for(Vertex vertex : vertices){
// 	srand(time(NULL) + vertex.id);
// 	myRandom = rand();
// 	communicator.allGather(vertex, graph, myRandom, recvData);

//     }

//     unsigned greatestRandom = 0;

//     for(VertexID i = 0; i < recvData.size(); ++i){
// 	if(recvData[i] > greatestRandom){
// 	    vertexID = i;
// 	    greatestRandom = recvData[i];
// 	}
//     }


//     return graph.getVertices().at(vertexID);  
// }


// template<typename T_Communicator, typename T_Graph, typename T_NameService>
// void occupyRandomVertex(T_Communicator& communicator, T_Graph& graph, T_NameService& nameService, std::vector<typename T_Graph::Vertex>& myVertices){
//     typedef T_Graph Graph;
//     typedef typename Graph::Vertex          Vertex;
//     typedef typename Vertex::ID             VertexID;
//     typedef typename T_Communicator::CommID CommID;

//     CommID masterCommID = randomHostCommID(communicator, nameService, graph);
//     Context context = nameService.getGraphContext(graph);

//     if(nameService.getHostedVertices(graph, masterCommID).empty()){
// 	std::cout << "The masterCommID does not host any vertices of this graph" << std::endl;
// 	return;
//     }

//     if(context.valid()){
//      	CommID commID    = context.getCommID();
//      	std::array<VertexID, 1> randomVertex{{0}};

//      	bool haveVertex = false;
//      	if(commID == masterCommID){
// 	    srand (time(NULL));
//      	    randomVertex[0] = rand() % graph.getVertices().size();

// 	    communicator.broadcast(masterCommID, context, randomVertex);

// 	    VertexID vertexID = randomVertex[0];
// 	    Vertex vertex     = graph.getVertices().at(vertexID);

//      	    for(Vertex v : myVertices){
//      	    	if(v.id == vertex.id){
//      	    	    std::cout << "[" << masterCommID << "] " << "Allready have random vertex: " << vertex.id << std::endl;
//      	    	    haveVertex = true;
//      	    	}
//      	    }
	
//      	    if(!haveVertex){
//      	    	std::cout << "[" << masterCommID << "] " << "Occupy vertex: " << vertex.id << std::endl;
//      	    	myVertices.push_back(vertex);
//      	    }

//      	}
//      	else {
// 	    communicator.broadcast(masterCommID, context, randomVertex);
//      	    VertexID vertexID = randomVertex[0];
//      	    Vertex vertex     = graph.getVertices().at(vertexID);

//      	     for(auto vertexIter = myVertices.begin(); vertexIter != myVertices.end();){
//      	     	if(vertexIter->id == vertex.id)
//      	     	    vertexIter = myVertices.erase(vertexIter);
// 		else
// 		    ++vertexIter;

//      	     }

//      	}

//     }

// }



/*******************************************************************************
 *
 * VERTEX DISTRIBUTION
 *
 *******************************************************************************/
template<typename T_Graph>
std::vector<typename T_Graph::Vertex> distributeVerticesEvenly(const unsigned processID, const unsigned processCount, T_Graph &graph){
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
 * VISUALIZATION OF GRAPH
 *
 *******************************************************************************/
std::vector<std::string> colors {"red", "green", "cyan", "skyblue1", "magenta", "beige", "orange", "yellow", "lightgray", "lightcyan", "moccasin"};

template<class Graph, class NameService>
struct vertexIDWriter{
    vertexIDWriter(Graph &graph, NameService &nameService) : graph(graph), nameService(nameService) {}
    void operator()(std::ostream& out, const typename Graph::GraphPolicyVertex& v) const {
	unsigned i = (unsigned) v;
	unsigned commID= nameService.locateVertex(graph, graph.getVertices().at(i));
	out << "[color=black fillcolor=" << colors[commID % colors.size()] << " fontsize=30 style=filled]";
	out << "[label=\"" << graph.getVertices().at(i).id << "\"]";
    }
private:
    Graph& graph;
    NameService& nameService;
};

template<class Graph>
struct edgeIDWriter{
    edgeIDWriter(Graph &graph) : graph(graph) {}
    void operator()(std::ostream& out, const typename Graph::GraphPolicyEdge& e) const {
	out << e;
    }
private:
    Graph& graph;
};

struct graphWriter {
    void operator()(std::ostream& out) const {
	out << "concentrate=true" << std::endl;
	out << "graph [bgcolor=white]" << std::endl;
	out << "node [shape=circle color=black]" << std::endl;
	out << "edge [color=black]" << std::endl;
    }
};


/*******************************************************************************
 *
 * WORK REDISTRIBUTION 
 *
 *******************************************************************************/

void redistribution(){


    /***************************************************************************
     * Configuration
     ****************************************************************************/
    typedef GraphPolicy::NoProperty                  NoProperty; 
    typedef GraphPolicy::BGL<NoProperty, NoProperty> BGL;
    typedef Graph<BGL>                               BGLGraph;
    typedef typename BGLGraph::Vertex                Vertex;
    typedef typename BGLGraph::Edge                  Edge;
    typedef typename BGLGraph::EdgeDescriptor        EdgeDescriptor;

    // Communicator
    typedef CommunicationPolicy::MPI          Mpi;
    typedef Communicator<Mpi>                 MpiCommunicator;
    typedef typename MpiCommunicator::Context Context;
    typedef typename MpiCommunicator::Event   Event;
    typedef typename MpiCommunicator::CommID  CommID;

    typedef NameService<BGLGraph, MpiCommunicator>           NS;
    typedef GraphCommunicator<BGLGraph, MpiCommunicator, NS> GC;


    /***************************************************************************
     * Create graph
     ****************************************************************************/
    std::vector<Vertex> graphVertices;
    //std::vector<EdgeDescriptor> edges = generateFullyConnectedTopology(10, graphVertices);
    //std::vector<EdgeDescriptor> edges = generateStarTopology(10, graphVertices);
    //std::vector<EdgeDescriptor> edges = generateHyperCubeTopology(4, graphVertices);
    std::vector<EdgeDescriptor> edges = generate2DMeshTopology<BGLGraph>(2, 2, graphVertices);
    BGLGraph graph (edges, graphVertices); //graph.print();


    /***************************************************************************
     * Create some subgraph
     ****************************************************************************/
    std::vector<Vertex> subGraphVertices;
    for(unsigned vertex_i = 0; vertex_i < graphVertices.size() / 2; ++vertex_i){
    	subGraphVertices.push_back(graph.getVertices().at(vertex_i));
    }
    //BGLGraph& subGraph = graph.createSubGraph(subGraphVertices); //subGraph.print();


    /***************************************************************************
     * Create communicator
     ****************************************************************************/
     MpiCommunicator communicator; 
     //communicator.synchronize();
     //CommID myCommID  = communicator.getGlobalContext().getCommID();
     //unsigned commCount = communicator.getGlobalContext().size();
     //NS nameService(graph, communicator);
     //GC graphCommunicator(communicator, nameService);


    /***************************************************************************
     * Examples communication schemas
     ****************************************************************************/

    // // Distribute vertices to communicators
    // std::vector<Vertex> myGraphVertices    = distributeVerticesEvenly(myCommID, commCount, graph);
    // std::vector<Vertex> mySubGraphVertices = distributeVerticesEvenly(myCommID, commCount, subGraph);

    // // Output vertex property
    // printVertexDistribution(myGraphVertices, graph, myCommID);
    // printVertexDistribution(mySubGraphVertices, subGraph, myCommID);

    // // Synchronize after output
    // communicator.synchronize();

    // // Announce distribution on network
    // nameService.announce(graph, myGraphVertices);
    // nameService.announce(subGraph, mySubGraphVertices);

    // // Write graph to dot file
    // if(!myGraphVertices.empty()){
    // 	graph.writeGraph(vertexIDWriter<BGLGraph, NS>(graph, nameService), edgeIDWriter<BGLGraph>(graph), graphWriter(), std::string("graph.dot"));
    // }


    // // BUG
    // // does not work !!!
    // if(!mySubGraphVertices.empty()){
    // 	subGraph.writeGraph(vertexIDWriter<BGLGraph, NS>(subGraph, nameService), edgeIDWriter<BGLGraph>(subGraph), graphWriter(), std::string("subgraph.dot"));
    // }


    // //Communication on graph level
    // if(!myGraphVertices.empty()){
    // 	nearestNeighborExchange(graphCommunicator, graph, myGraphVertices); 
    // 	reduceVertexIDs(graphCommunicator, graph, myGraphVertices);

    // }

    // Communication on subgraph level
    // if(!mySubGraphVertices.empty()){
    // 	nearestNeighborExchange(graphCommunicator, subGraph, mySubGraphVertices);
    // 	reduceVertexIDs(graphCommunicator, subGraph, mySubGraphVertices);

    // }

    // /***************************************************************************
    //  * Redistribution of vertex
    //  ****************************************************************************/

    // // TODO
    // // Communicator which has no vertex of subgraph
    // // can´t occupy vertex from this subgraph!
    // // Because this communicator is not part
    // // of the subgraph context!
    // // Need to recreate context first!
    // if(!mySubGraphVertices.empty()){
    // 	occupyRandomVertex(communicator, subGraph, nameService, mySubGraphVertices);
    // 	printVertexDistribution(mySubGraphVertices, subGraph, myCommID);
    // 	nameService.announce(subGraph, mySubGraphVertices);
    // }

    // if(!mySubGraphVertices.empty()){    
    // 	//reduceVertexIDs(myGraphCommunicator, subGraph, mySubGraphVertices);
    // 	nearestNeighborExchange(graphCommunicator, subGraph, mySubGraphVertices);
    // }


}


/*******************************************************************************
 *
 * GAME OF LIFE
 *
 *******************************************************************************/

void life() {
    /**
     * Configuration
     */
    struct Cell {
	typedef unsigned ID;
	Cell() : id(0), isAlive(0){}
	Cell(ID id) : id(id), isAlive(0){
	    unsigned random = rand() % 10000;
	    if(random < 3125){
		isAlive = 1;
	    }

	}
	
	unsigned id;
	bool isAlive;

    };
    
    // Graph
    typedef GraphPolicy::NoProperty            NoProperty;
    typedef GraphPolicy::BGL<Cell, NoProperty> BGL;
    typedef Graph<BGL>                         LifeGraph;
    typedef typename LifeGraph::Vertex         Vertex;
    typedef typename LifeGraph::Edge           Edge;
    typedef typename LifeGraph::EdgeDescriptor EdgeDescriptor;

    // Communicator
    typedef CommunicationPolicy::MPI         Mpi;
    typedef Communicator<Mpi>                MpiCommunicator;
    typedef typename MpiCommunicator::CommID CommID;


    typedef NameService<LifeGraph, MpiCommunicator>           NS;
    typedef GraphCommunicator<LifeGraph, MpiCommunicator, NS> GC;
    typedef typename GC::Event                                Event;

    /**
     * Game Logik
     */
    // Create cells
    const unsigned height = 25;
    const unsigned width  = 40;
    std::vector<Vertex> graphVertices;
    std::vector<EdgeDescriptor> edges = generate2DMeshDiagonalTopology<LifeGraph>(height, width, graphVertices);

    LifeGraph graph (edges, graphVertices); //graph.print();

    MpiCommunicator communicator;
    NS nameService(graph, communicator);
    GC graphCommunicator(communicator, nameService);

    // Distribute work evenly
    CommID myCommID  = communicator.getGlobalContext().getCommID();
    unsigned commCount = communicator.getGlobalContext().size();
    std::vector<Vertex> myGraphVertices = distributeVerticesEvenly(myCommID, commCount, graph);

    // Announce work distribution 
    nameService.announce(graph, myGraphVertices); 
    unsigned generation = 0;


    std::vector<Event> events;   
    std::vector<unsigned> aliveMap(graph.getVertices().size(), 0); 

    // Simulate life forever
    while(true){

	// Print life field
	if(myCommID == 0){

	    for(unsigned i = 0; i < aliveMap.size(); ++i){
		if((i % (width)) == 0){
		    std::cout << std::endl;
		}

		if(aliveMap.at(i)){
		    std::cout << "#";
		}
		else {
		    std::cout << " ";
		}

	    }
	    std::cout << "Generation: " << generation << std::endl;
	    for(unsigned i = 0; i < height+1; ++i){
	      std::cerr << "\033[F";
	    }
	}


	// Send status to neighbor cells
	for(Vertex v : myGraphVertices){
	    std::vector<std::pair<Vertex, Edge> > outEdges = graph.getOutEdges(v); 
	    for(std::pair<Vertex, Edge> edge : outEdges){
		std::vector<unsigned> isAlive(1, v.isAlive);
		events.push_back(graphCommunicator.asyncSend(graph, edge.first, edge.second, isAlive));
	    
	    }

	}


	// Recv status from neighbor cells
	std::vector<unsigned> aliveCount(myGraphVertices.size(), 0);
	unsigned vertex_i = 0;
	for(Vertex v : myGraphVertices){

	    std::vector<std::pair<Vertex, Edge> > inEdges = graph.getInEdges(v);
	
	    for(std::pair<Vertex, Edge> edge : inEdges){
		std::vector<unsigned> isAlive(1, 0);
		graphCommunicator.recv(graph, edge.first, edge.second, isAlive);
		if(isAlive[0]) aliveCount.at(vertex_i)++;

	    }
	    vertex_i++;

	}


	for(unsigned i = 0; i < events.size(); ++i){
	    events.back().wait();
	    events.pop_back();
	}


	// Calculate status for next generation
	vertex_i = 0;
	for(Vertex &v : myGraphVertices){
	
	    switch(aliveCount.at(vertex_i)){

	    case 0:
	    case 1:
		v.isAlive = 0;
		break;

	    case 2:
		v.isAlive = v.isAlive;
		break;
	    
	    case 3: 
		v.isAlive = 1;
		break;

	    default: 
		v.isAlive = 0;
		break;

	    }
	    vertex_i++;
	}

	communicator.synchronize(nameService.getGraphContext(graph));

	// Send alive information to host of vertex 0
	for(Vertex v: myGraphVertices){
	    std::vector<unsigned> isAlive(1, v.isAlive);
	    events.push_back(graphCommunicator.asyncSend(graph, graph.getVertices().at(0), 0, isAlive));
	}

	if(myCommID == 0){
	    for(Vertex v: graph.getVertices()){
		std::vector<unsigned> isAlive(1, 0);
		graphCommunicator.recv(graph, v, 0, isAlive);
		aliveMap.at(v.id) = isAlive[0];
	    }

	}

	for(unsigned i = 0; i < events.size(); ++i){
	    events.back().wait();
	    events.pop_back();
	}

	communicator.synchronize(nameService.getGraphContext(graph));
	generation++;
    }
    
}



/*******************************************************************************
 *
 * MAIN
 *
 *******************************************************************************/
int main(){

    //life();
    redistribution();
    return 0;

}

