// Communication
#include <graybat.h>
// Helpers
#include <distribution.hpp> /* consecutive, roundRobin */
#include <topology.hpp>     /* meshDiagonal */

// STL
#include <iostream>   /* std::cout */
#include <tuple>      /* std::pair */
#include <vector>     /* std::vector */
#include <array>      /* std::array */
#include <cmath>      /* sqrt */
#include <cstdlib>    /* atoi */
#include <assert.h>   /* assert */

struct Cell : public SimpleProperty{
    Cell() : SimpleProperty(0), isAlive{{0}}, aliveNeighbors(0){}
    Cell(ID id) : SimpleProperty(id), isAlive{{0}}, aliveNeighbors(0){
      unsigned random = rand() % 10000;
      if(random < 3125){
	isAlive[0] = 1;
      }

    }
	
    std::array<unsigned, 1> isAlive;
    unsigned aliveNeighbors;

};

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
    for(T_Cell &cell : cells){
	updateState(cell);

    }

}

template <class T_Cell>
void updateState(T_Cell &cell){
    switch(cell.aliveNeighbors){

    case 0:
    case 1:
	cell.isAlive[0] = 0;
	break;

    case 2:
	cell.isAlive[0] = cell.isAlive[0];
	break;
	    
    case 3: 
	cell.isAlive[0] = 1;
	break;

    default: 
	cell.isAlive[0] = 0;
	break;

    };

}

int gol(const unsigned nCells, const unsigned nTimeSteps ) {
    /***************************************************************************
     * Configuration
     ****************************************************************************/
    // Graph
    typedef Graph<Cell, SimpleProperty>            LifeGraph;
    typedef typename LifeGraph::Vertex             Vertex;
    typedef typename LifeGraph::Edge               Edge;
    typedef typename LifeGraph::EdgeDescriptor     EdgeDescriptor;

    // Cal
    typedef CommunicationPolicy::MPI               Mpi;
    typedef CommunicationAbstractionLayer<Mpi>     MpiCAL;
    typedef typename MpiCAL::Event                 Event;

    // GVON
    typedef GraphBasedVirtualOverlayNetwork<LifeGraph, MpiCAL>  GVON;

    /***************************************************************************
     * Init Communication
     ****************************************************************************/
    // Create Graph
    const unsigned height = sqrt(nCells);
    const unsigned width  = height;
    std::vector<Vertex> graphVertices;
    std::vector<EdgeDescriptor> edges = Topology::gridDiagonal<LifeGraph>(height, width, graphVertices);
    LifeGraph graph (edges, graphVertices);

    // Inantiate communication objects
    MpiCAL cal;
    GVON gvon(cal);

    // Distribute work evenly
    std::vector<Vertex> hostedVertices = distribute::consecutive(cal.getGlobalContext(), graph);

    // Announce vertices
    gvon.announce(graph, hostedVertices); 

    /***************************************************************************
     * Start Simulation
     ****************************************************************************/
    std::vector<Event> events;   
    std::vector<unsigned> golDomain(graph.getVertices().size(), 0); 

    const Vertex root = graph.getVertices().at(0);

    // Simulate life forever
    for(unsigned timestep = 0; timestep < nTimeSteps; ++timestep){

	// Print life field by owner of vertex 0
	if(gvon.peerHostsVertex(root, graph)){
	  printGolDomain(golDomain, width, height, timestep);
	}
	
	// Send state to neighbor cells
	for(Vertex &v : hostedVertices){
	    for(std::pair<Vertex, Edge> edge : graph.getOutEdges(v)){
		events.push_back(gvon.asyncSend(graph, edge.first, edge.second, v.isAlive));
	    }
	}

	// Recv state from neighbor cells
	for(Vertex &v : hostedVertices){
	     for(std::pair<Vertex, Edge> edge : graph.getInEdges(v)){
		 gvon.recv(graph, edge.first, edge.second, edge.first.isAlive);
		 if(edge.first.isAlive[0]) v.aliveNeighbors++;
	     }
	 }

	 // Wait to finish events
	 for(unsigned i = 0; i < events.size(); ++i){
	     events.back().wait();
	     events.pop_back();
	 }

	 // Calculate state for next generation
	 updateState(hostedVertices);

	 // Gather state by vertex with id = 0
	 for(Vertex &v: hostedVertices){
	     v.aliveNeighbors = 0;
	     gvon.gather(root, v, graph, v.isAlive, golDomain, true);
	 }


    }
    
    return 0;

}

int main(int argc, char** argv){

    if(argc < 3){
	std::cout << "Usage ./GoL [nCells] [nTimeteps]" << std::endl;
	return 0;
    }

    const unsigned nCells    = atoi(argv[1]);
    const unsigned nTimeSteps = atoi(argv[2]);


    gol(nCells, nTimeSteps);


    return 0;
}
