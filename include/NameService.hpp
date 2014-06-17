#pragma once
#include <map> /* map */

template <typename T_Graph, typename T_Communicator>
struct NameService {
    typedef T_Communicator                   Communicator;
    typedef T_Graph                          Graph;
    typedef typename Graph::Vertex           Vertex;
    typedef typename Communicator::CommID    CommID;
    typedef typename Vertex::ID              VertexID;
    typedef typename Graph::GraphID          GraphID;
    typedef typename Communicator::Context   Context;
    typedef typename Communicator::ContextID ContextID;
    typedef typename Communicator::BinaryOperations BinaryOperations;

    // Maps
    std::map<VertexID, CommID> commMap;
    std::map<GraphID, Context> contextMap;
    std::map<CommID, std::vector<Vertex> > vertexMap;

    // References
    Graph& graph;
    Communicator& communicator;
    unsigned graphContextCount;

    NameService(Graph& graph, Communicator& communicator) : 
	graph(graph),
	communicator(communicator),
	graphContextCount(0){
	
	contextMap[graph.id] =  communicator.getGlobalContext();

    }


    void announce(const std::vector<Vertex> vertices){
	Context communicatorContext = communicator.getGlobalContext();

    	// Each announces how many nodes it manages
    	std::array<unsigned, 1> myVerticesCount {{(unsigned) vertices.size()}};
    	std::array<unsigned, 1> maxVerticesCount  {{0}};
    	communicator.allReduce(communicatorContext, BinaryOperations::MAX, myVerticesCount, maxVerticesCount);
	 
	
    	for(unsigned i = 0; i < maxVerticesCount[0]; ++i){
    	    const size_t communicatorContextSize = communicatorContext.size();
    	    std::array<int, 1> sendData{{-1}};
    	    std::vector<int> recvData(communicatorContextSize);

    	    if(i < vertices.size()){
    	    	sendData[0] = vertices.at(i).id;
    	    }
    	    else{
    	    	sendData[0] = -1;
    	    }

    	    communicator.allGather(communicatorContext, sendData, recvData);

    	    for(unsigned commID = 0; commID < communicatorContextSize; ++commID){
    	    	if(recvData[commID] != -1){
    		    VertexID vertexID = (VertexID) recvData[commID];
    		    commMap[vertexID] = commID;
		    vertexMap[commID].push_back(graph.getVertices().at(vertexID));
		    
    	    	}
    	    }

    	}

    }

    void announce(Graph& graph, Graph& subGraph){
	std::vector<Vertex> vertices = subGraph.getVertices();

	std::vector<CommID> commIDs;
	for(Vertex vertex : vertices){
	    commIDs.push_back(mapVertex(vertex));
	}
	
	Context oldContext = mapGraph(graph);
	Context newContext = communicator.createContext(commIDs, oldContext);
	
	contextMap[subGraph.id] = newContext;

    }

    CommID mapVertex(Vertex vertex){
	return commMap[vertex.id];

    }
    
    std::vector<Vertex> mapCommID(CommID commID){
	return vertexMap[commID];

    }

    Context mapGraph(Graph graph){
	return contextMap[graph.id];

    }

};
