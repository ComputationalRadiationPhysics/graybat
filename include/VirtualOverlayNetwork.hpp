#pragma once
#include <map>       /* map */
#include <set>       /* set */
#include <exception> /* std::out_of_range */
#include <sstream>   /* std::stringstream */
#include <assert.h>  /* assert */

#include <atomic>  // std::atomic
#include <mutex>   // std::mutex


#include <binaryOperation.hpp> /* op::maximum */
#include <dout.hpp>            /* dout::Dout::getInstance() */

/************************************************************************//**
 * @class VirtualOverlayNetwork
 *
 * @brief A central instance to locate the host 
 *        of vertices.
 *
 * @remark A peer can host several vertices.
 * @todo   Think of vertices hosted by several peers
 *         for fault tolerance purpose
 * @todo remove T_Graph template
 *
 *
 ***************************************************************************/
template <typename T_Graph, typename T_CAL>
struct VirtualOverlayNetwork {
    typedef T_CAL                            CAL;
    typedef T_Graph                          Graph;
    typedef typename Graph::Vertex           Vertex;
    typedef typename Graph::Edge             Edge;
    typedef typename Graph::GraphID          GraphID;
    typedef typename Vertex::ID              VertexID;
    typedef typename CAL::VAddr    VAddr;
    typedef typename CAL::Event     Event;
    typedef typename CAL::Context   Context;
    typedef typename CAL::ContextID ContextID;

    // References
    CAL& cal;

    /***************************************************************************
     *
     * MAPPING OPERATIONS
     *
     ***************************************************************************/
    VirtualOverlayNetwork(CAL& cal) : 
    	cal(cal){

    }


    /**
     * @brief Announces *vertices* of a *graph* to the network, so that other peers
     *        know that these *vertices* are hosted by this peer.
     *
     * The general workflow includes two steps:
     *  1. Each peer, that hosts vertices of the *graph* announces its *vertices*
     *     * Each peer will send its hosted vertices and update its vertices location
     *     * The host peers will create a new context for *graph*
     *  2. Vertices can now be located by locateVertex()
     *  3. use Graphpeer to communicate between vertices
     *
     * @remark This is a collective Operation on which either all host peers
     *         of the supergraph of *graph* have to take part or when *graph* has no
     *         supergraph then all Communicatos from the globalContext (which should
     *         be all peers in the network).
     *
     * @todo What happens if *vertices* is empty ?
     * @todo What happens when there already exist an context for *graph* ?
     * @todo What happens when not all *vertices* of a *graph* were announced ?
     * @todo Reduce communication from 2 steps (allReduce + allGather) to one
     *       step (allGather2), could reduce communication.
     *
     * @param[in] graph  Its vertices will be announced
     * @param[in] vertices A set of vertices, that will be hosted by this peer
     *
     */
    void announce(Graph& graph, const std::vector<Vertex> vertices){
	// Get old context from graph
	Context oldContext = getGraphContext(graph);

	 if(!oldContext.valid()){
	     if(graph.hasSuperGraph()){
		 //std::cout << "hasSuperGraph" << std::endl;
		 oldContext = getGraphContext(graph.superGraph);

	     }
	     else {
		 //std::cout << "global context" << std::endl;
		 oldContext = cal.getGlobalContext();

	     }

	 }
	 else {
	     //std::cout << "Has already context" << std::endl;
	 }

	assert(oldContext.valid());

	// Create new context for peers which host vertices
	std::vector<unsigned> hasVertices(1, vertices.size());
	std::vector<unsigned> recvHasVertices(oldContext.size(), 0);
	cal.allGather(oldContext, hasVertices, recvHasVertices);

	std::vector<VAddr> vAddrsWithVertices;
	for(unsigned i = 0; i < recvHasVertices.size(); ++i){
	    if(recvHasVertices[i] > 0){
		vAddrsWithVertices.push_back(i);
	    }
	}

	Context newContext = cal.createContext(vAddrsWithVertices, oldContext); 

	// Get IDs of hosted vertices
	// std::vector<unsigned> myVertexIDs;
	// for(Vertex v: vertices){
	//     myVertexIDs.push_back(v.id);
	// }
	
	// Each peer announces the vertices it hosts
	if(newContext.valid()){
	    
	    graphMap[graph.id] = newContext;

	    // std::vector<unsigned> recvCount(newContext.size());
	    // std::vector<unsigned> vertexIDs(graph.getVertices().size(), 0);
	    // cal.allGather2(newContext, myVertexIDs, vertexIDs, recvCount);

	    // unsigned offset = 0;
	    // for(unsigned vAddr = 0; vAddr < newVertexMaps.size(); ++vAddr){
	    // 	std::vector<Vertex> vertexMap;
	    // 	unsigned nhostedVertices = recvCount[vAddr];
		
		
	    // }


	    // Retrieve maximum number of vertices per peer
	    std::vector<unsigned> myVerticesCount(1,vertices.size());
	    std::vector<unsigned> maxVerticesCount(1,  0);
	    cal.allReduce(newContext, op::maximum<unsigned>(), myVerticesCount, maxVerticesCount);

	    // Gather maxVerticesCount times vertex ids
	    std::vector<std::vector<Vertex> > newVertexMaps (newContext.size(), std::vector<Vertex>());
	    for(unsigned i = 0; i < maxVerticesCount[0]; ++i){
		std::vector<int> vertexID(1, -1);
	    	std::vector<int> recvData(newContext.size(), 0);

	    	if(i < vertices.size()){
	    	    vertexID[0] = graph.getLocalID(vertices.at(i));
	    	}

	    	cal.allGather(newContext, vertexID, recvData);
		
		   
	    	for(unsigned vAddr = 0; vAddr < newVertexMaps.size(); ++vAddr){
	    	    if(recvData[vAddr] != -1){
	    		VertexID vertexID = (VertexID) recvData[vAddr];
	    		Vertex v = graph.getVertices().at(vertexID);
	    		vertexMap[graph.id][v.id] = vAddr; 
	    		newVertexMaps[vAddr].push_back(v);
		    
	    	    }

	    	}
      
	    	for(unsigned vAddr = 0; vAddr < newVertexMaps.size(); ++vAddr){
	    	    peerMap[graph.id][vAddr] = newVertexMaps[vAddr];

	    	}

	    }

	}
	
    }

  
    /**
     * @brief Returns the VAddr of the host peer of *vertex* in the *graph*
     *
     * @bug When the location of *vertex* is not known then
     *      the programm crashes by an exception. 
     *      This exception should be handled for better
     *      debugging behaviour.
     *
     * @param[in] graph Contains *vertex*.
     * @param[in] vertex Will be located.
     *
     */
    VAddr locateVertex(Graph& graph, Vertex vertex){
	return vertexMap[graph.id][vertex.id];

    }

    /**
     * @brief Opposite operation of locateVertex(). It returns the
     *        vertices that are hosted by the peer with
     *        *vAddr*
     */
    std::vector<Vertex> getHostedVertices(Graph& graph, VAddr vAddr){
	return peerMap[graph.id][vAddr];

    }

    /**
     * @brief Returns the context of the *graph*. All host
     *        peers of the *graph* are part of the
     *        returned context.
     *
     */
    Context getGraphContext(Graph& graph){
	return graphMap[graph.id];

    }



    /***************************************************************************
     *
     * COMMUNICATION OPERATIONS BASED ON THE GRAPH
     *
     ***************************************************************************/


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
	VAddr destVAddr = locateVertex(graph, destVertex);
	Context context   = getGraphContext(graph);
	Event e = cal.asyncSend(destVAddr, edge.id, context, data);
	e.wait();
    }

    /**
     * @brief Asynchron transmission of *data* to the *destVertex* on *edge*.
     *
     * @todo Documentation how data should be formated !!!
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
	VAddr destVAddr = locateVertex(graph, destVertex);
	Context context   = getGraphContext(graph);
	return cal.asyncSend(destVAddr, edge.id, context, data);
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
	VAddr srcVAddr = locateVertex(graph, srcVertex);
	Context context  = getGraphContext(graph);
	Event e = cal.asyncRecv(srcVAddr, edge.id, context, data);
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
	VAddr srcVAddr = locateVertex(graph, srcVertex);
	Context context  = getGraphContext(graph);
	return cal.asyncRecv(srcVAddr, edge.id, context, data);

    }

    /**************************************************************************
     *
     * COLLECTIVE GRAPH OPERATIONS
     *
     **************************************************************************/ 

    template <typename T>
    struct Reduce {
	unsigned count;
	T reduce;
	bool imRoot;
	T* rootRecvData;
	
    };

    /**
     * @brief Collective reduction of sendData (here only sum). *rootVertex* will
     *        receive the reduced value. Data of vertices from the same host
     *        peer will be reduced locally first. The reduction of
     *        a graph has to be finished til new reduction can be performed.
     *
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
    template <typename T, typename Op>
    void reduce(const Vertex rootVertex, const Vertex srcVertex, Graph& graph, Op op, const std::vector<T> sendData, T& recvData){
	static std::map<std::string, Reduce<T>> reduces;

	VAddr rootVAddr = locateVertex(graph, rootVertex);
	VAddr srcVAddr  = locateVertex(graph, srcVertex);
	Context context   = getGraphContext(graph);
	std::vector<Vertex> vertices = getHostedVertices(graph, srcVAddr); 

	std::string reduceID = generateID(graph, srcVertex);

	for(T data : sendData){
	    reduces[reduceID].reduce = op(reduces[reduceID].reduce, data);
	}

	// Remember pointer of recvData from rootVertex
	if(rootVertex.id == srcVertex.id){
	    reduces[reduceID].rootRecvData = &recvData;
	    reduces[reduceID].imRoot = true;
	}

	reduces[reduceID].count++;

	// Finally start reduction
	if(reduces[reduceID].count == vertices.size()){
	    T recvDataCollctive;

	    cal.reduce(rootVAddr, context, op, std::vector<T>(1 , reduces[reduceID].reduce), recvDataCollctive);

	    if(reduces[reduceID].imRoot){
		*(reduces[reduceID].rootRecvData) = recvDataCollctive;
	    }
	    
	    reduces.erase(reduceID);
	    vertexCount.erase(graph.id);

	}


    }


    template <typename T>
    struct Collective {
	std::vector<T> send;
	bool isRoot;
	std::vector<T>* rootRecvData;
	
    };

    /**
     * @brief Collective operation that collects data from all vertices of the *graph* and sends this *recvData*
     *        the *rootVertex*.The gather of a graph has to be finished til new gather can be performed.
     *
     * @param[in]  rootVertex Vetex that will receive the collected Data.
     * @param[in]  srcVertex  One of the vertices that send his data *sendData*.
     * @param[in]  graph      Vertices of this *graph* take part on the collective operation.
     * @param[in]  sendData   Data that each *srcVertex* sends (each Vertex send one element).
     * @param[out] recvData   Data that *rootVertex* will receive from each srcVertex in local vertex.id order.
     */
    template <typename T>
    void gather(const Vertex rootVertex, const Vertex srcVertex, Graph& graph, const T sendData, std::vector<T>& recvData){
	static std::map<std::string, Collective<T>> gathers;

	VAddr srcVAddr  = locateVertex(graph, srcVertex);
	VAddr rootVAddr = locateVertex(graph, rootVertex);
	Context context   = getGraphContext(graph);
	std::vector<Vertex> vertices = getHostedVertices(graph, srcVAddr);



	std::string gatherID = generateID(graph, srcVertex);

	// Temporily collect data to gather
	gathers[gatherID].send.push_back(sendData);

	if(rootVertex.id == srcVertex.id){
	    gathers[gatherID].rootRecvData = &recvData;
	    gathers[gatherID].isRoot = true;
	}

	if(gathers[gatherID].send.size() == vertices.size()){
	    std::vector<unsigned> recvCount;
	    std::vector<T> recvDataCollective;
	    cal.gather2(rootVAddr, context, gathers[gatherID].send, recvDataCollective, recvCount);

	    // Reorder received elements in vertex order
	    std::vector<T> recvReordered(recvDataCollective.size(), 0);
	    unsigned vAddr = 0;
	    for(unsigned recv_i = 0; recv_i < recvDataCollective.size(); ){
		std::vector<Vertex> hostedVertices = getHostedVertices(graph, vAddr);
		for(Vertex v: hostedVertices){
		    T recvElement = recvDataCollective.at(recv_i);
		    recvReordered.at(v.id) = recvElement;
		    recv_i++;
		}
		vAddr++;
	    
	    }
	
	    // Write received data to root cal pointer
	    if(gathers[gatherID].isRoot){
		*(gathers[gatherID].rootRecvData) = recvReordered;
	    }

	    gathers.erase(gatherID);
	    vertexCount.erase(graph.id);

	}

    }

    void synchronize(Graph &graph){
	Context context = getGraphContext(graph);
	cal.synchronize(context);

    }

    
private:

    /***************************************************************************
     *
     * MAPS
     *
     ***************************************************************************/
    // Maps vertices to its hosts
    std::map<GraphID, std::map<VertexID, VAddr> > vertexMap;
    
    // Each graph is mapped to a context of the peer
    std::map<GraphID, Context> graphMap; 

    // List of vertices a peer hosts of a graph
    std::map<GraphID, std::map<VAddr, std::vector<Vertex>> > peerMap;

    // Count of vertices that are in a collective operation in the moment
    std::map<GraphID, std::map<VertexID, unsigned>> vertexCount;


    /***************************************************************************
     *
     * AUXILIARY FUNCTIONS
     *
     ***************************************************************************/

    /**
     * @brief Returns a set of all host peer VAddrs of the *graph*
     *
     */
    std::vector<VAddr> getGraphHostVAddrs(Graph& graph){
	std::vector<Vertex> vertices = graph.getVertices();

	std::set<VAddr> vAddrs;
	for(Vertex vertex : vertices){
	    vAddrs.insert(locateVertex(graph, vertex));
	}

	return std::vector<VAddr>(vAddrs.begin(), vAddrs.end());

    }

    std::string generateID(Graph &graph, Vertex vertex){
	std::stringstream idSS; 
	if(vertexCount[graph.id][vertex.id] > 0){
	    throw std::logic_error("Can not perform collective operation on same vertex in same graph simultaneous");
	}
	else {
	    idSS << graph.id << vertexCount[graph.id][vertex.id]++;
	    return idSS.str();
	}

    }


    /**
     * @brief Creates a context from the given *subgraph* inherited from
     *        the context of the given graph.
     *
     * @param[in] graph is the supergraph of subGraph
     * @param[in] subGraph is a subgraph of graph
     *
     */
    void createGraphContext(Graph& graph, Graph& subGraph){
	std::vector<VAddr> vAddrs = getGraphHostVAddrs(subGraph);

	Context oldContext = getGraphContext(graph);
	Context newContext = cal.createContext(vAddrs, oldContext);
	if(newContext.valid()){
	    graphMap[subGraph.id] = newContext;
	}

    }

    /**
     * @brief Creates a context for the the given graph inherited from
     *        the global context. After this step, vertices within
     *        this graph can do communication.
     *
     * @param[in] graph for which a new context from global context will be created.
     */
    void createGraphContext(Graph& graph){
	std::vector<VAddr> vAddrs = getGraphHostVAddrs(graph);

	Context oldContext = cal.getGlobalContext();
	Context newContext = cal.createContext(vAddrs, oldContext);
	
	graphMap[graph.id] = newContext;
    
    }



};
