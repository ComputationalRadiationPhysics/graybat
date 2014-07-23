#pragma once

#include <sstream>
#include <atomic>  // std::atomic
#include <mutex>   // std::mutex

/************************************************************************//**
* @class GraphCommunicator
*
* @brief Provides point to point and collective communication schemas
*        on graph base. Communicator is used as communication backend
*        and NameService provide location information of the vertices
*        of the Graph.
*
***************************************************************************/
template <typename T_Graph, typename T_Communicator, typename T_NameService>
struct GraphCommunicator {

    typedef T_Graph                 Graph;
    typedef typename Graph::Vertex  Vertex;
    typedef typename Vertex::ID     VertexID;
    typedef typename Graph::Edge    Edge;
    typedef typename Graph::GraphID GraphID;

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
    
    /**
     * @brief Synchron transmission of *data* to the *destVertex* on *edge*.
     *
     * @param[in] graph The graph in which the communication takes place.
     * @param[in] destVertex Vertex that will receive the *data*.
     * @param[in] edge Edge over which the *data* will be transmitted.
     * @param[in] data Data that will be send.
     *
     */
    template <typename T>
    void send(Graph &graph, const Vertex destVertex, const Edge edge, const T& data){
      CommID destCommID = nameService.locateVertex(graph, destVertex);
      Context context = nameService.getGraphContext(graph);
      Event e = communicator.asyncSend(destCommID, edge.id, context, data);
      e.wait();
    }

    /**
     * @brief Asynchron transmission of *data* to the *destVertex* on *edge*.
     *
     * @param[in] graph The graph in which the communication takes place.
     * @param[in] destVertex Vertex that will receive the *data*.
     * @param[in] edge Edge over which the *data* will be transmitted.
     * @param[in] data Data that will be send.
     *
     * @return Event Can be waited (Event::wait()) for or checked for (Event::ready())
     *
     */
    template <typename T>
    Event asyncSend(Graph& graph, const Vertex destVertex, const Edge edge, const T& data){
      CommID destCommID = nameService.locateVertex(graph, destVertex);
      Context context = nameService.getGraphContext(graph);
       return communicator.asyncSend(destCommID, edge.id, context, data);
    }

    /**
     * @brief Synchron receive of *data* from the *srcVertex* on *edge*.
     *
     * @param[in]  graph The graph in which the communication takes place.
     * @param[in]  srcVertex Vertex that send the *data*
     * @param[in]  edge Edge over which the *data* will be transmitted.
     * @param[out] data Data that will be received
     *
     */
    template <typename T>
    void recv(Graph& graph, const Vertex srcVertex, const Edge edge, const T& data){
      CommID srcCommID = nameService.locateVertex(graph, srcVertex);
      Context context = nameService.getGraphContext(graph);
      Event e = communicator.asyncRecv(srcCommID, edge.id, context, data);
      e.wait();

    }

    /**
     * @brief Asynchron receive of *data* from the *srcVertex* on *edge*.
     *
     * @param[in]  graph The graph in which the communication takes place.
     * @param[in]  srcVertex Vertex that send the *data*
     * @param[in]  edge Edge over which the *data* will be transmitted.
     * @param[out] data Data that will be received
     *
     * @return Event Can be waited (Event::wait()) for or checked for (Event::ready())
     *
     */
    template <typename T>
    Event asyncRecv(Graph& graph, const Vertex srcVertex, const Edge edge, const T& data){
      CommID srcCommID = nameService.locateVertex(graph, srcVertex);
      Context context = nameService.getGraphContext(graph);
      return communicator.asyncRecv(srcCommID, edge.id, context, data);

    }

    /**************************************************************************
     *
     * COLLECTIVE GRAPH OPERATIONS
     *
     **************************************************************************/ 

    std::map<GraphID, std::map<VertexID, unsigned>> vertexCount;

    template <typename T>
    struct Reduce {
	std::atomic<unsigned> count;
	std::atomic<T> reduce;
	bool imRoot;
	T* rootRecvData;
	
	
    };

    /**
     * @brief Collective reduction of sendData (here only sum). *rootVertex* will
     *        receive the reduced value. Data of vertices from the same host
     *        Communicator will be reduced locally first.
     *
     * @todo Add the possibility to choose own binary operation
     * @todo Do I have to return an Event because its partly non blocking ?
     * @todo By collecting data first you could get rid of Reduce.count !
     *
     * @remark This function should be thread safe (not tested)!
     *
     * @param[in]  rootVertex Vertex that will receive the reduced value.
     * @param[in]  srcVertex  Vertex that provides *sendData* to be reduced.
     * @param[in]  graph      Vertices of this *graph* take part in the reduction.
     * @param[in]  sendData   Data that will be reduced.
     * @param[out] recvData   Reduced value, that will be received by *rootVertex*
     *
     */

    std::mutex mtx;


    template <typename T>
    void reduce(const Vertex rootVertex, const Vertex srcVertex, Graph& graph, const std::vector<T> sendData, T& recvData){
	static std::map<std::string, Reduce<T>> reduces;

	CommID rootCommID = nameService.locateVertex(graph, rootVertex);
	CommID srcCommID  = nameService.locateVertex(graph, srcVertex);
	Context context = nameService.getGraphContext(graph);
	std::vector<Vertex> vertices = nameService.getHostedVertices(graph, srcCommID); 

	std::string reduceID = generateID(graph, srcVertex);

	for(T data : sendData){
	    reduces[reduceID].reduce.fetch_add(data);
	}



	// Remember pointer of recvData from rootVertex
	if(rootVertex.id == srcVertex.id){
	    reduces[reduceID].rootRecvData = &recvData;
	    reduces[reduceID].imRoot = true;
	}

	mtx.lock();
	reduces[reduceID].count++;
	// Finally start reduction
	if(reduces[reduceID].count == vertices.size()){
	    T recvDataCollctive;
	    communicator.reduce(rootCommID, context, BinaryOperations::SUM, std::vector<T>(1 , reduces[reduceID].reduce), recvDataCollctive);

	    if(reduces[reduceID].imRoot){
		*(reduces[reduceID].rootRecvData) = recvDataCollctive;
	    }
	    
	    reduces.erase(reduceID);

	}
	mtx.unlock();

    }


    template <typename T>
    struct Collective {
	std::vector<T> send;
	bool isRoot;
	T* rootRecvData;
	
    };

    /**
     * @brief Collective operation that collects data from all vertices of the *graph* and sends this *recvData*
     *        the *rootVertex*.
     *
     * @param[in]  rootVertex Vetex that will receive the collected Data.
     * @param[in]  srcVertex  One of the vertices that send his data *sendData*.
     * @param[in]  graph      Vertices of this *graph* take part on the collective operation.
     * @param[in]  sendData   Data that each *srcVertex* sends.
     * @param[out] recvData   Data that *rootVertex* will receive from each srcVertex.
     */
    template <typename T>
    void gather(const Vertex rootVertex, const Vertex srcVertex, Graph& graph, const T sendData, std::vector<T>& recvData){
	static std::map<std::string, Collective<T>> gathers;

	CommID srcCommID  = nameService.locateVertex(graph, srcVertex);
	CommID rootCommID  = nameService.locateVertex(graph, rootVertex);
	std::vector<Vertex> vertices = nameService.getHostedVertices(graph, srcCommID);
	Context context = nameService.getGraphContext(graph);

	std::string gatherID = generateID(graph, srcVertex);
	
	gathers[gatherID].send.push_back(sendData);

	if(rootVertex.id == srcVertex.id){
	    gathers[gatherID].rootRecvData = &recvData;
	    gathers[gatherID].isRoot = true;
	}

	if(gathers[gatherID].send.size() == vertices.size()){
	    std::vector<T> recvDataCollective;
	    communicator.gather2(rootCommID, context, gathers[gatherID].send, recvDataCollective);

	    if(gathers[gatherID].isRoot){
		*(gathers[gatherID].rootRecvData) = recvDataCollective;
	    }

	    gathers.erase(gatherID);
	}

    }

    // TODO
    // Return Event because its non blocking !
    template <typename T>
    void allGather(const Vertex srcVertex, Graph& graph, const T sendData, std::vector<T>& recvData){
	static std::vector<T> sendTmp;
	static std::vector<std::vector<T>*> recvDataPtr;

	CommID srcCommID  = nameService.locateVertex(graph, srcVertex);
	std::vector<Vertex> vertices = nameService.getHostedVertices(graph, srcCommID);
	Context context = nameService.getGraphContext(graph);

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

    	CommID srcCommID  = nameService.locateVertex(graph, srcVertex);
    	CommID rootCommID  = nameService.locateVertex(graph, rootVertex);
    	std::vector<Vertex> vertices = nameService.getHostedVertices(graph, srcCommID);
    	Context context = nameService.getGraphContext(graph);

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

    
private:
    std::string generateID(Graph &graph, Vertex vertex){
	std::stringstream idSS; 
	idSS << graph.id << vertexCount[graph.id][vertex.id]++;
	return idSS.str();

    }

};
