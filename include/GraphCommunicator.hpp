#pragma once

template <typename T_Graph, typename T_Communicator, typename T_NameService>
struct GraphCommunicator {

    typedef T_Graph        Graph;
    typedef typename Graph::Vertex  Vertex;
    typedef typename Graph::Edge    Edge;

    typedef T_Communicator Communicator;
    typedef typename Communicator::Context Context;
    typedef typename Communicator::CommID  CommID;

    typedef T_NameService  NameService;

    GraphCommunicator(Graph& graph, Communicator& communicator, NameService& nameService) : 
	graph(graph),
	communicator(communicator),
	nameService(nameService){



    }

    Graph&        graph;
    Communicator& communicator;
    NameService&  nameService;
    
    template <typename T>
    void send(const Vertex dest, const Edge edge, const T& data){
	CommID destCommID = nameService.mapVertex(dest);
	Context globalContext = communicator.getGlobalContext();
	communicator.send(destCommID, edge.id, globalContext, data);
	std::cout << "send: " << data.data() << std::endl;
    }


    template <typename T>
    void recv(const Vertex src, const Edge edge, const T& data){
	CommID srcCommID = nameService.mapVertex(src);
	Context globalContext = communicator.getGlobalContext();
	communicator.recv(srcCommID, edge.id, globalContext, data);
	std::cout << "recv: " << data.data() << std::endl;

    }






};
