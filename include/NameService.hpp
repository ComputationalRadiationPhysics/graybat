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
    std::map<VertexID, CommID> commMap;
    std::map<GraphID, Context> contextMap;
    std::map<CommID, std::vector<Vertex> > vertexMap;

    // References
    Graph& graph;
    Communicator& communicator;

    NameService(Graph& graph, Communicator& communicator) : 
	graph(graph),
	communicator(communicator){

    }


  /**
   *
   *
   */
  void announce(Graph& graph, const std::vector<Vertex> vertices){
    Context communicatorContext = mapGraph(graph);

    // Each announces how many nodes it manages
    std::array<unsigned, 1> myVerticesCount {{(unsigned) vertices.size()}};
    std::array<unsigned, 1> maxVerticesCount  {{0}};
    communicator.allReduce(communicatorContext, BinaryOperations::MAX, myVerticesCount, maxVerticesCount);
	 
	
    for(unsigned i = 0; i < maxVerticesCount[0]; ++i){
      const size_t communicatorContextSize = communicatorContext.size();
      std::vector<int> sendData(1, -1);
      std::vector<int> recvData(communicatorContextSize, 0);

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

  /**
   * Creates a context for the given subgraph inherited from
   * the context of the given graph.
   *
   * @param[in] graph is the supergraph of subGraph
   * @param[in] subGraph is a subgraph of graph
   *
   */
  void announce(Graph& graph, Graph& subGraph){
    std::vector<Vertex> vertices = subGraph.getVertices();

    std::set<CommID> commID;
    for(Vertex vertex : vertices){
      commID.insert(mapVertex(vertex));
    }
	
    Context oldContext = mapGraph(graph);
    Context newContext = communicator.createContext(std::vector<CommID>(commID.begin(), commID.end()), oldContext);
	
    contextMap[subGraph.id] = newContext;

  }

  /**
   * Creates a context for the the given graph inherited from
   * the global context. After this step, vertices within
   * this graph can do communication.
   *
   * @param[in]
   */
  void announce(Graph& graph){
    std::vector<Vertex> vertices = graph.getVertices();

    std::set<CommID> commID;
    for(Vertex vertex : vertices){
      commID.insert(mapVertex(vertex));
    }
	
    Context oldContext = communicator.getGlobalContext();
    Context newContext = communicator.createContext(std::vector<CommID>(commID.begin(), commID.end()), oldContext);
	
    contextMap[graph.id] = newContext;
    
  }

    CommID mapVertex(Vertex vertex){
	return commMap[vertex.id];

    }
    
    std::vector<Vertex> mapCommID(CommID commID){
	return vertexMap[commID];

    }

    Context mapGraph(Graph& graph){
	return contextMap[graph.id];

    }

};
