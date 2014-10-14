// Communication
#include <Graph.hpp>                         /* Graph */
#include <CommunicationAbstractionLayer.hpp> /* CommunicationAbstractionLayer */
#include <MPI.hpp>                           /* CommunicationPolicy::MPI*/
#include <VirtualOverlayNetwork.hpp>         /* VirtualOverlayNetwork */

// Helpers
#include <distribution.hpp> /* roundRobin */
#include <topology.hpp>     /* meshDiagonal */

// STL
#include <iostream>   /* std::cout */
#include <tuple>      /* std::pair */
#include <vector>     /* std::vector */
#include <cmath>      /* sqrt */


void printGolDomain(const std::vector<unsigned> domain, const unsigned width, const unsigned height, const unsigned generation){
    for(unsigned i = 0; i < domain.size(); ++i){
	if((i % (width)) == 0){
	    std::cerr << std::endl;
	}

	if(domain.at(i)){
	    std::cerr << "#";
	}
	else {
	    std::cerr << " ";
	}

    }
    std::cerr << "Generation: " << generation << std::endl;
    for(unsigned i = 0; i < height+1; ++i){
	std::cerr << "\033[F";
    }

}

template <class T_Cell>
void updateState(std::vector<T_Cell> &cells){
    for(T_Cell &v : cells){
	
	switch(v.aliveNeighbors){

	case 0:
	case 1:
	    v.isAlive = 0;
	    break;

	case 2:
	    v.isAlive = v.isAlive;
	    break;
	    
	case 3: 
	    v.isAlive = 1;
	    break;

	default: 
	    v.isAlive = 0;
	    break;

	}

    }

}

void gol(const unsigned N) {
    /***************************************************************************
     * Configuration
     ****************************************************************************/
    struct Cell {
	typedef unsigned ID;
	Cell() : id(0), isAlive(0), aliveNeighbors(0){}
	Cell(ID id) : id(id), isAlive(0), aliveNeighbors(0){
	    unsigned random = rand() % 10000;
	    if(random < 3125){
		isAlive = 1;
	    }

	}
	
	unsigned id;
	bool isAlive;
	unsigned aliveNeighbors;

    };

    // Graph
    typedef Graph<Cell, SimpleProperty>               LifeGraph;
    typedef typename LifeGraph::Vertex                Vertex;
    typedef typename LifeGraph::Edge                  Edge;
    typedef typename LifeGraph::EdgeDescriptor        EdgeDescriptor;

    // Cal
    typedef CommunicationPolicy::MPI                  Mpi;
    typedef CommunicationAbstractionLayer<Mpi>        MpiCAL;
    typedef typename MpiCAL::VAddr                    VAddr;
    typedef typename MpiCAL::Event                    Event;

    // GVON
    typedef VirtualOverlayNetwork<LifeGraph, MpiCAL>  GVON;


    /***************************************************************************
     * Init Communication
     ****************************************************************************/
    // Create Graph
    std::cout << "Generate communication topology" << std::endl;
    const unsigned height = unsigned(sqrt(N));
    const unsigned width  = unsigned(sqrt(N));
    std::vector<Vertex> graphVertices;
    std::vector<EdgeDescriptor> edges = Topology::gridDiagonal<LifeGraph>(height, width, graphVertices);
    LifeGraph graph (edges, graphVertices); 

    // Inantiate communication objects
    MpiCAL cal;
    GVON gvon(cal);

    // Distribute work evenly
    VAddr myVAddr      = cal.getGlobalContext().getVAddr();
    unsigned commCount = cal.getGlobalContext().size();
    std::vector<Vertex> myGraphVertices = Distribute::roundRobin(myVAddr, commCount, graph);

    // Announce vertices
    gvon.announce(graph, myGraphVertices); 


    /***************************************************************************
     * Start Simulation
     ****************************************************************************/
    unsigned generation = 0;
    std::vector<Event> events;   
    std::vector<unsigned> golDomain(graph.getVertices().size(), 0); 

    // Simulate life forever
    std::cout << "Start simulation" << std::endl;
    while(true){

	// Print life field
	if(myVAddr == 0){
	    printGolDomain(golDomain, width, height, generation);
	}

	// Send status to neighbor cells
	for(Vertex v : myGraphVertices){
	    for(std::pair<Vertex, Edge> edge : graph.getOutEdges(v)){
		//std::vector<unsigned> isAlive(1, v.isAlive);
		events.push_back(gvon.asyncSend(graph, edge.first, edge.second, v.isAlive));
	    }
	}

	// Recv status from neighbor cells
	for(Vertex &v : myGraphVertices){
	    for(std::pair<Vertex, Edge> edge : graph.getInEdges(v)){
		std::vector<unsigned> isAlive(1, 0);
		gvon.recv(graph, edge.first, edge.second, isAlive);
		if(isAlive[0]){ 
		    v.aliveNeighbors++;
		}

	    }
	}

	// Wait to finish events
	for(unsigned i = 0; i < events.size(); ++i){
	    events.back().wait();
	    events.pop_back();
	}

	// Calculate state for next generation
	updateState(myGraphVertices);

	// Send alive information to host of vertex 0
	for(Vertex &v: myGraphVertices){
	    v.aliveNeighbors = 0;
	    gvon.gather(graph.getVertices().at(0), v, graph, unsigned(v.isAlive), golDomain);
	}

	generation++;
    }
    
}

int main(){

    const unsigned N = 900;

    gol(N);

    return 0;
}
