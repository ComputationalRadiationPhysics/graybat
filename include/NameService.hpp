#pragma once
#include <map>       /* map */
#include <set>       /* set */
#include <exception> /* std::out_of_range */
#include <sstream>   /* std::stringstream */
#include <assert.h>  /* assert */

#include <binaryOperation.hpp> /* op::maximum */
#include <dout.hpp>            /* dout::Dout::getInstance() */

/************************************************************************//**
 * @class NameService
 *
 * @brief A central instance to locate the host Communicator
 *        of vertices.
 *
 * @remark A Communicator can host several vertices.
 * @todo   Think of vertices hosted by several Communicators
 *         for fault tolerance purpose
 * @todo remove T_Graph template
 *
 *
 ***************************************************************************/
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

    // Maps
    std::map<GraphID, std::map<VertexID, CommID> > commMap;
    std::map<GraphID, Context> contextMap; /** Each graph is mapped to a context of the Communicator*/
    std::map<GraphID, std::map<CommID, std::vector<Vertex>> > vertexMap;

    // References
    //Graph& graph;
    Communicator& communicator;

    // NameService(Graph& graph, Communicator& communicator) : 
    // 	graph(graph),
    // 	communicator(communicator){

    // }

    NameService(Communicator& communicator) : 
    	communicator(communicator){

    }

    /**
     * @brief Announces *vertices* of a *graph* to the network, so that other Communicators
     *        know that these *vertices* are hosted by this Communicator.
     *
     * The general workflow includes two steps:
     *  1. Each Communicator, that hosts vertices of the *graph* announces its *vertices*
     *     * Each Communicator will send its hosted vertices and update its vertices location
     *     * The host Communicators will create a new context for *graph*
     *  2. Vertices can now be located by locateVertex()
     *  3. use GraphCommunicator to communicate between vertices
     *
     * @remark This is a collective Operation on which either all host Communicators
     *         of the supergraph of *graph* have to take part or when *graph* has no
     *         supergraph then all Communicatos from the globalContext (which should
     *         be all Communicators in the network).
     *
     * @todo What happens if *vertices* is empty ?
     * @todo What happens when there already exist an context for *graph* ?
     * @todo What happens when not all *vertices* of a *graph* were announced ?
     *
     * @param[in] graph  Its vertices will be announced
     * @param[in] vertices A set of vertices, that will be hosted by this Communicator
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
		 oldContext = communicator.getGlobalContext();

	     }

	 }
	 else {
	     //std::cout << "Has already context" << std::endl;
	 }

	assert(oldContext.valid());

	// Create new context for communicators which host vertices
	std::vector<unsigned> hasVertices(1, !vertices.empty());
	std::vector<unsigned> recvHasVertices(oldContext.size(), 0);
	communicator.allGather(oldContext, hasVertices, recvHasVertices);

	std::vector<CommID> commIDsWithVertices;
	for(unsigned i = 0; i < recvHasVertices.size(); ++i){
	    if(recvHasVertices[i] == 1){
		commIDsWithVertices.push_back(i);
	    }
	}

	Context newContext = communicator.createContext(commIDsWithVertices, oldContext); 
	
	// Each Communicator announces the vertices it hosts
	if(newContext.valid()){
	    
	    contextMap[graph.id] = newContext;

	    std::vector<unsigned> myVerticesCount(1,vertices.size());
	    std::vector<unsigned> maxVerticesCount(1,  0);
	    communicator.allReduce(newContext, op::maximum<unsigned>(), myVerticesCount, maxVerticesCount);

	    std::vector<std::vector<Vertex> > newVertexMap (newContext.size(), std::vector<Vertex>());
	    for(unsigned i = 0; i < maxVerticesCount[0]; ++i){
		std::vector<int> vertexID(1, -1);
	    	std::vector<int> recvData(newContext.size(), 0);

	    	if(i < vertices.size()){
	    	    vertexID[0] = graph.getLocalID(vertices.at(i));
	    	}

	    	communicator.allGather(newContext, vertexID, recvData);
		
		   
	    	for(unsigned commID = 0; commID < newVertexMap.size(); ++commID){
	    	    if(recvData[commID] != -1){
	    		VertexID vertexID = (VertexID) recvData[commID];
	    		Vertex v = graph.getVertices().at(vertexID);
	    		commMap[graph.id][v.id] = commID; 
	    		newVertexMap[commID].push_back(v);
		    
	    	    }

	    	}
      
	    	for(unsigned commID = 0; commID < newVertexMap.size(); ++commID){
	    	    vertexMap[graph.id][commID] = newVertexMap[commID];

	    	}

	    }

	}
	
    }

    //template <class T_Graph2>
    struct TestVertex{
	typedef T_Graph Graph;
	typedef typename T_Graph::Vertex Vertex;

	TestVertex(Communicator communicator, Graph graph, Vertex vertex) : 
	    communicator(communicator),
	    graph(graph),
	    vertex(vertex){
	    
	}

	void send(){
	    std::cout << vertex.id << std::endl;
	}

	Communicator &communicator;
	Graph        &graph;
	Vertex       vertex;

    };

    template <class T_Graph2>
    TestVertex testAnnounce(T_Graph2& graph, const std::vector<typename T_Graph2::Vertex> vertices){
	typedef T_Graph2                  Graph;
	//typedef typename T_Graph2::Vertex Vertex;

	//TestVertex<Graph> testVertex(communicator, graph);
	TestVertex testVertex(communicator, graph, vertices.at(0));


    }



  
    /**
     * @brief Returns the CommID of the host Communicator of *vertex* in the *graph*
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
    CommID locateVertex(Graph& graph, Vertex vertex){
	return commMap[graph.id][vertex.id];

    }

    /**
     * @brief Opposite operation of locateVertex(). It returns the
     *        vertices that are hosted by the Communicator with
     *        *commID*
     */
    std::vector<Vertex> getHostedVertices(Graph& graph, CommID commID){
	return vertexMap[graph.id][commID];

    }

    /**
     * @brief Returns the context of the *graph*. All host
     *        Communicators of the *graph* are part of the
     *        returned context.
     *
     */
    Context getGraphContext(Graph& graph){
	return contextMap[graph.id];

    }

    /**
     * @brief Returns a set of all host Communicator CommIDs of the *graph*
     *
     */
    std::vector<CommID> getGraphHostCommIDs(Graph& graph){
	std::vector<Vertex> vertices = graph.getVertices();

	std::set<CommID> commIDs;
	for(Vertex vertex : vertices){
	    commIDs.insert(locateVertex(graph, vertex));
	}

	return std::vector<CommID>(commIDs.begin(), commIDs.end());

    }

private:

    /**
     * @brief Creates a context from the given *subgraph* inherited from
     *        the context of the given graph.
     *
     * @param[in] graph is the supergraph of subGraph
     * @param[in] subGraph is a subgraph of graph
     *
     */
    void createGraphContext(Graph& graph, Graph& subGraph){
	std::vector<CommID> commIDs = getGraphHostCommIDs(subGraph);

	Context oldContext = getGraphContext(graph);
	Context newContext = communicator.createContext(commIDs, oldContext);
	if(newContext.valid()){
	    contextMap[subGraph.id] = newContext;
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
	std::vector<CommID> commIDs = getGraphHostCommIDs(graph);

	Context oldContext = communicator.getGlobalContext();
	Context newContext = communicator.createContext(commIDs, oldContext);
	
	contextMap[graph.id] = newContext;
    
    }



};
