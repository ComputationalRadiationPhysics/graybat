#pragma once

template <typename T_Graph, typename T_Communicator, typename T_NameService>
struct GraphCommunicator {

    typedef T_Graph                 Graph;
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

    }

    Communicator& communicator;
    NameService&  nameService;
    
  // TODO
  // Replace globalcontext with context of graph
  // ==> context = nameservice.mapGraph(graph)
    template <typename T>
    void send(Graph &graph, const Vertex dest, const Edge edge, const T& data){
      CommID destCommID = nameService.mapVertex(graph, dest);
      Context globalContext = communicator.getGlobalContext();
      communicator.send(destCommID, edge.id, globalContext, data);
    }

    template <typename T>
    Event asyncSend(Graph& graph, const Vertex dest, const Edge edge, const T& data){
      CommID destCommID = nameService.mapVertex(graph, dest);
	Context globalContext = communicator.getGlobalContext();
	return communicator.asyncSend(destCommID, edge.id, globalContext, data);
    }

    template <typename T>
    void recv(Graph& graph, const Vertex src, const Edge edge, const T& data){
      CommID srcCommID = nameService.mapVertex(graph, src);
	Context globalContext = communicator.getGlobalContext();
	communicator.recv(srcCommID, edge.id, globalContext, data);

    }

    template <typename T>
    void asyncRecv(Graph& graph, const Vertex src, const Edge edge, const T& data){
      CommID srcCommID = nameService.mapVertex(graph, src);
	Context globalContext = communicator.getGlobalContext();
	communicator.recv(srcCommID, edge.id, globalContext, data);

    }

    /**************************************************************************
     *
     * COLLECTIVE GRAPH OPERATIONS
     *
     **************************************************************************/ 



    // TODO
    // first collect all values and then reduce them !
    // Return Event because its non blocking !
    template <typename T>
    void reduce(const Vertex rootVertex, const Vertex srcVertex, Graph& graph, const std::vector<T> sendData, T& recvData){
	static unsigned reduceCount;
	static T reduceTmp;
	static T* rootRecvDataPtr;
	static bool imRoot;
	CommID rootCommID = nameService.mapVertex(graph, rootVertex);
	CommID srcCommID  = nameService.mapVertex(graph, srcVertex);
	std::vector<Vertex> vertices = nameService.mapCommID(graph, srcCommID); // <== BUGGY gives not correct vertices of a Communicator

	Context context = nameService.mapGraph(graph);
	//std::cout << "reduce context size:" << context.size() << " graph id:" << graph.id << std::endl;

	for(T d : sendData){
	  //std::cout << reduceTmp << std::endl;
	  reduceTmp += d;
	}

	if(rootVertex.id == srcVertex.id){
	    rootRecvDataPtr = &recvData;
	    imRoot = true;
	}

	reduceCount++;
	//std::cout << "srcCommID: " << srcCommID << " reduceCount: "<< reduceCount << " vertices.size(): " << vertices.size() << std::endl;
	if(reduceCount == vertices.size()){
	    T recvDataCollctive;
	    communicator.reduce(rootCommID, context, BinaryOperations::SUM, std::vector<T>(1 , reduceTmp), recvDataCollctive);

	    if(imRoot){
		*rootRecvDataPtr = recvDataCollctive;
	    }
	    
	    // reset static vars
	    reduceTmp   = 0;
	    reduceCount = 0;
	    imRoot      = false;
	}

	

    }

    template <typename T>
    void gather(const Vertex rootVertex, const Vertex srcVertex, Graph& graph, const T sendData, std::vector<T>& recvData){
	static std::vector<T>  sendTmp;
	static T* rootRecvDataPtr;
	static bool imRoot;

	CommID srcCommID  = nameService.mapVertex(graph, srcVertex);
	CommID rootCommID  = nameService.mapVertex(graph, rootVertex);
	std::vector<Vertex> vertices = nameService.mapCommID(graph, srcCommID);
	Context context = nameService.mapGraph(graph);

	sendTmp.push_back(sendData);

	if(rootVertex.id == srcVertex.id){
	    rootRecvDataPtr = &recvData;
	    imRoot = true;
	}

	if(sendTmp.size() == vertices.size()){
	    std::vector<T> recvDataCollective;
	    communicator.gather2(rootCommID, context, sendTmp, recvDataCollective);

	    if(imRoot){
		*rootRecvDataPtr = recvDataCollective;
	    }

	    // reset static var
	    sendTmp.clear();
	    imRoot = false;


	}

    }

    // TODO
    // Return Event because its non blocking !
    template <typename T>
    void allGather(const Vertex srcVertex, Graph& graph, const T sendData, std::vector<T>& recvData){
	static std::vector<T> sendTmp;
	static std::vector<std::vector<T>*> recvDataPtr;

	CommID srcCommID  = nameService.mapVertex(graph, srcVertex);
	std::vector<Vertex> vertices = nameService.mapCommID(graph, srcCommID);
	Context context = nameService.mapGraph(graph);

	sendTmp.push_back(sendData);
	recvDataPtr.push_back(&recvData);


	if(sendTmp.size() == vertices.size()){
	    std::vector<T> recvDataCollective;
	    communicator.allGather2(context, sendTmp, recvDataCollective);

	    for(unsigned i = 0; i < recvDataPtr.size(); ++i){
		*(recvDataPtr[i]) = recvDataCollective;
	    }

	    sendTmp.clear();
	    recvDataPtr.clear();
	}

    }

    template <typename T>
    void broadcast(const Vertex srcVertex, const Vertex rootVertex, Graph& graph, std::vector<T>& sendData){
    	static unsigned broadcastCount;
    	static std::vector<std::vector<T>*> broadcastPtr;
	static std::vector<T> sendDataPtr;
	static bool imRoot;

    	CommID srcCommID  = nameService.mapVertex(graph, srcVertex);
    	CommID rootCommID  = nameService.mapVertex(graph, rootVertex);
    	std::vector<Vertex> vertices = nameService.mapCommID(graph, srcCommID);
    	Context context = nameService.mapGraph(graph);

    	broadcastPtr.push_back(&sendData);
	if(rootVertex.id == srcVertex.id){
	    sendDataPtr = sendData;
	    imRoot = true;

	}

	broadcastCount++;

    	if(broadcastCount == vertices.size()){
	    if(imRoot){
		communicator.broadcast(rootCommID, context, sendDataPtr);
		for(unsigned i = 0; i < broadcastPtr.size(); ++i){
		    *(broadcastPtr[i]) = sendDataPtr;
		}

	    }
	    else {
		std::vector<T> recvData(1, 0);
		communicator.broadcast(rootCommID, context, recvData);
		for(unsigned i = 0; i < broadcastPtr.size(); ++i){
		    *(broadcastPtr[i]) = recvData;
		}

	    }
	    
	    // reset static vars
	    broadcastCount = 0;
	    broadcastPtr.clear();
	    imRoot = false;

    	}

	
    }

    
};
