#pragma once

template <typename T_Graph, typename T_Communicator, typename T_NameService>
struct GraphCommunicator {

    typedef T_Graph        Graph;
    typedef typename Graph::Vertex  Vertex;
    typedef typename Graph::Edge    Edge;

    typedef T_Communicator Communicator;
    typedef typename Communicator::Context          Context;
    typedef typename Communicator::CommID           CommID;
    typedef typename Communicator::Event            Event;
    typedef typename Communicator::BinaryOperation  BinaryOperation;
    typedef typename Communicator::BinaryOperations BinaryOperations;

    typedef T_NameService  NameService;
    

    GraphCommunicator(Communicator& communicator, NameService& nameService) : 
	communicator(communicator),
	nameService(nameService){

	reduceCount = 0;
	reduceTmp   = 0;

    }

    Communicator& communicator;
    NameService&  nameService;
    
    template <typename T>
    void send(const Vertex dest, const Edge edge, const T& data){
	CommID destCommID = nameService.mapVertex(dest);
	Context globalContext = communicator.getGlobalContext();
	communicator.send(destCommID, edge.id, globalContext, data);
    }

    template <typename T>
    Event asyncSend(const Vertex dest, const Edge edge, const T& data){
	CommID destCommID = nameService.mapVertex(dest);
	Context globalContext = communicator.getGlobalContext();
	return communicator.asyncSend(destCommID, edge.id, globalContext, data);
    }

    template <typename T>
    void recv(const Vertex src, const Edge edge, const T& data){
	CommID srcCommID = nameService.mapVertex(src);
	Context globalContext = communicator.getGlobalContext();
	communicator.recv(srcCommID, edge.id, globalContext, data);

    }

    template <typename T>
    void asyncRecv(const Vertex src, const Edge edge, const T& data){
	CommID srcCommID = nameService.mapVertex(src);
	Context globalContext = communicator.getGlobalContext();
	communicator.recv(srcCommID, edge.id, globalContext, data);

    }

    
    template <typename T>
    void reduce(const Vertex rootVertex, const std::vector<T> sendData, T& recvData){
	CommID rootCommID = nameService.mapVertex(rootVertex);
	std::vector<Vertex> vertices = nameService.mapCommID(rootCommID);


	Context globalContext = communicator.getGlobalContext();
	// TODO
	// subGraph dependand Context
	// Start reduce just when every Vertex in subGraph which
	// is managaed by this communicator has started reduce
	// 
	// Context subGraphContext = nameService.mapGraph(subGraph);

	for(T d : sendData){
	    reduceTmp += d;
	}

	reduceCount++;
	if(reduceCount == vertices.size()){
	    communicator.reduce(rootCommID, globalContext, BinaryOperations::SUM, std::vector<unsigned>(1 , reduceTmp), recvData);
	    std::cout << "Reduce: " << recvData << std::endl;
	    reduceTmp = 0;
	    reduceCount = 0;
	}

    }
    
    unsigned reduceTmp;
    unsigned reduceCount;

};
