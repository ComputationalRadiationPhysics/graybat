#pragma once
#include <map> /* map */
#include <set> /* set */

/************************************************************************//**
 * @brief A central instance to locate host the Communicator
 *        of vertices.
 *
 * @remark A Communicator can host several vertices.
 * @todo   Think of vertices hosted by several Communicators
 *         for fault tolerance purpose
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
    typedef typename Communicator::BinaryOperations BinaryOperations;

    // Maps
    std::map<GraphID, std::map<VertexID, CommID> > commMap;
    std::map<GraphID, Context> contextMap; /** Each graph is mapped to a context of the Communicator*/
    std::map<GraphID, std::map<CommID, std::vector<Vertex>> > vertexMap;

    // References
    Graph& graph;
    Communicator& communicator;

    NameService(Graph& graph, Communicator& communicator) : 
	graph(graph),
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
     *
     * @param[in] graph  Its vertices will be announced
     * @param[in] vertices A set of vertices, that will be hosted by this Communicator
     *
     */
    void announce(Graph& graph, const std::vector<Vertex> vertices){
	// Get super context
	Context oldContext;
	if(graph.hasSuperGraph()){
	    oldContext = mapGraph(graph.superGraph);
	}
	else {
	    oldContext = communicator.getGlobalContext();
	}

    
	if(oldContext.valid()){
	    // Each process announces which vertices it manages
	    std::array<unsigned, 1> myVerticesCount {{(unsigned) vertices.size()}};
	    std::array<unsigned, 1> maxVerticesCount  {{0}};
	    communicator.allReduce(oldContext, BinaryOperations::MAX, myVerticesCount, maxVerticesCount);
	
	    std::vector<std::vector<Vertex> > newVertexMap (oldContext.size(), std::vector<Vertex>());
	    for(unsigned i = 0; i < maxVerticesCount[0]; ++i){
		std::vector<int> sendData(1, -1);
		std::vector<int> recvData(oldContext.size(), 0);

		if(i < vertices.size()){
		    sendData[0] = graph.getLocalID(vertices.at(i));
		}
		else{
		    sendData[0] = -1;
		}

		communicator.allGather(oldContext, sendData, recvData);


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

	    //Create new sub context for the graph
	    if(graph.hasSuperGraph()){
		createGraphContext(graph.superGraph, graph);

	    }
	    else {
		createGraphContext(graph);

	    }


	}


    }
  
    /**
     * @brief Returns the CommID of the host Communicator of *vertex* of a *graph*
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
     *
     * @bug Exception when commID is not host of vertices in *graph*.
     *      This Exception should be hanled somehow!
     *
     */
    std::vector<Vertex> mapCommID(Graph& graph, CommID commID){
	return vertexMap[graph.id][commID];

    }

    /**
     * @brief Returns the context of the *graph*. All host
     *        Communicators of the *graph* are part of the
     *        returned context.
     *
     */
    Context mapGraph(Graph& graph){
	return contextMap[graph.id];

    }

private:

    /**
     * Creates a context from the given *subgraph* inherited from
     * the context of the given graph.
     *
     * @param[in] graph is the supergraph of subGraph
     * @param[in] subGraph is a subgraph of graph
     *
     */
    void createGraphContext(Graph& graph, Graph& subGraph){
	std::vector<Vertex> vertices = subGraph.getVertices();

	std::set<CommID> commIDs;
	for(Vertex vertex : vertices){
	    commIDs.insert(locateVertex(subGraph, vertex));
	}

	Context oldContext = mapGraph(graph);
	Context newContext = communicator.createContext(std::vector<CommID>(commIDs.begin(), commIDs.end()), oldContext);
	if(newContext.valid()){
	    contextMap[subGraph.id] = newContext;
	}

    }

    /**
     * Creates a context for the the given graph inherited from
     * the global context. After this step, vertices within
     * this graph can do communication.
     *
     * @param[in]
     */
    void createGraphContext(Graph& graph){
	std::vector<Vertex> vertices = graph.getVertices();

	std::set<CommID> commID;
	for(Vertex vertex : vertices){
	    commID.insert(locateVertex(graph, vertex));
	}
	
	Context oldContext = communicator.getGlobalContext();
	Context newContext = communicator.createContext(std::vector<CommID>(commID.begin(), commID.end()), oldContext);
	
	contextMap[graph.id] = newContext;
    
    }


};
