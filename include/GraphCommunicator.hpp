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

    // TODO
    // Return Event because its non blocking !
    template <typename T>
    void reduce(const Vertex rootVertex, const Vertex srcVertex, Graph graph, const std::vector<T> sendData, T& recvData){
	static T reduceTmp;
	CommID rootCommID = nameService.mapVertex(rootVertex);
	CommID srcCommID  = nameService.mapVertex(srcVertex);
	std::vector<Vertex> vertices = nameService.mapCommID(srcCommID);

	Context context = nameService.mapGraph(graph);

	for(T d : sendData){
	    reduceTmp += d;
	}

	reduceCount++;
	if(reduceCount == vertices.size()){
	    communicator.reduce(rootCommID, context, BinaryOperations::SUM, std::vector<unsigned>(1 , reduceTmp), recvData);
	    reduceTmp = 0;
	    reduceCount = 0;
	}

	

    }

    // TODO
    // Return Event because its non blocking !
    template <typename T>
    void allGather(const Vertex srcVertex, Graph& graph, const T sendData, std::vector<T>& recvData){
	static std::vector<T> gatherTmp;
	CommID srcCommID  = nameService.mapVertex(srcVertex);
	std::vector<Vertex> vertices = nameService.mapCommID(srcCommID);
	Context context = nameService.mapGraph(graph);

	gatherTmp.push_back(sendData);

	if(gatherTmp.size() == vertices.size()){
	    communicator.allGather2(context, gatherTmp, recvData);
	    gatherTmp.clear();
		
	}

	for(T t : recvData){
	    std::cout << t << std::endl;
	}

	// for(T d : sendData){
	//     reduceTmp += d;
	// }

	// reduceCount++;
	// if(reduceCount == vertices.size()){
	//     communicator.reduce(rootCommID, context, BinaryOperations::SUM, std::vector<unsigned>(1 , reduceTmp), recvData);
	//     reduceTmp = 0;
	//     reduceCount = 0;
	// }

	

    }

    unsigned reduceCount;
    
};