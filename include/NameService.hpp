#pragma once
#include <map> /* map */
#include <set> /* set */

/**
 * Nameservice provides the connection between the logical level of a
 * graph and the physical level of the communication layer.  When sending
 * data between vertices of a graph the main question is where a specific
 * vertex is located in the underlying communication network.
 *
 * Before communication can take place, each process has to announce the
 * vertices it manages. After that graphs or subgraphs have
 * to be announced to create their communication context.
 */
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
  std::map<GraphID, Context> contextMap;
  std::map<GraphID, std::map<CommID, std::vector<Vertex>> > vertexMap;

    // References
    Graph& graph;
    Communicator& communicator;

    NameService(Graph& graph, Communicator& communicator) : 
	graph(graph),
	communicator(communicator){

    }

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
	
      for(unsigned i = 0; i < maxVerticesCount[0]; ++i){
	std::vector<int> sendData(1, -1);
	std::vector<int> recvData(oldContext.size(), 0);

	if(i < vertices.size()){
	  //std::cout << "graphID:" << graph.id << " commiD:" << oldContext.getCommID() << " size:" << vertices.size() << std::endl;
	  sendData[0] = graph.getLocalID(vertices.at(i));
	}
	else{
	  sendData[0] = -1;
	}

	communicator.allGather(oldContext, sendData, recvData);

	std::vector<std::vector<Vertex> > vertexMapTmp (oldContext.size(), std::vector<Vertex>());
	for(unsigned commID = 0; commID < vertexMapTmp.size(); ++commID){
	  if(recvData[commID] != -1){
	    VertexID vertexID = (VertexID) recvData[commID];
	    Vertex v = graph.getVertices().at(vertexID);
	    commMap[graph.id][v.id] = commID; // <======= FAIL

	    // if(graph.id == 1){
	    //   std::cout << "graphID:" << graph.id << " myCommID:" << oldContext.getCommID() << " commID:" << mapVertex(graph, v) << " Vertex:" << v.id <<std::endl;
	    // }
	    vertexMapTmp[commID].push_back(v);
		    
	  }

	}
      
	for(unsigned commID = 0; commID < vertexMapTmp.size(); ++commID){
	  vertexMap[graph.id][commID] = vertexMapTmp[commID];

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
  
  CommID mapVertex(Graph& graph, Vertex vertex){
    return commMap[graph.id][vertex.id];

  }
    
  std::vector<Vertex> mapCommID(Graph& graph, CommID commID){
    return vertexMap[graph.id][commID];

  }

  Context mapGraph(Graph& graph){
    return contextMap[graph.id];

  }

private:

/**
   * Creates a context for the given subgraph inherited from
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
      //std::cout << "graphID:" << subGraph.id << " CommID:" << mapVertex(subGraph, vertex) << " vertex:" << vertex.id << std::endl;
      commIDs.insert(mapVertex(subGraph, vertex));
    }

    Context oldContext = mapGraph(graph);
    Context newContext = communicator.createContext(std::vector<CommID>(commIDs.begin(), commIDs.end()), oldContext);
    if(newContext.valid()){
      //std::cout << "valid oldContext:" << oldContext.getCommID() << std::endl;
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
      commID.insert(mapVertex(graph, vertex));
    }
	
    Context oldContext = communicator.getGlobalContext();
    Context newContext = communicator.createContext(std::vector<CommID>(commID.begin(), commID.end()), oldContext);
	
    contextMap[graph.id] = newContext;
    
  }


};
