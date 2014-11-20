// Communication
#include <Graph.hpp>                           /* Graph */
#include <CommunicationAbstractionLayer.hpp>   /* CommunicationAbstractionLayer */
#include <MPI.hpp>                             /* CommunicationPolicy::MPI*/
#include <GraphBasedVirtualOverlayNetwork.hpp> /* GraphBasedVirtualOverlayNetwork */

// Helpers
#include <distribution.hpp> /* roundRobin */
#include <topology.hpp>     /* meshDiagonal */
#include <evals.hpp>

// STL
#include <iostream>   /* std::cout */
#include <tuple>      /* std::pair */
#include <vector>     /* std::vector */
#include <array>      /* std::array */
#include <cmath>      /* sqrt */
#include <cstdlib>    /* atoi */
#include <chrono>     /* std::chrono::high_resolution_clock */ 
#include <assert.h>   /* assert */


// MPI
#include <mpi.h>


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

void printGolDomain(const std::vector<unsigned> domain, const unsigned width, const unsigned height, const unsigned generation, const unsigned nPeers){
    // for(unsigned i = 0; i < domain.size(); ++i){
    // 	if((i % (width)) == 0){
    // 	    std::cerr << std::endl;
    // 	}

    // 	if(domain.at(i)){
    // 	    std::cerr << "#";
    // 	}
    // 	else {
    // 	    std::cerr << " ";
    // 	}

    // }
    std::cerr << "Generation: " << generation <<  " nPeers: " << nPeers << std::endl;
    // for(unsigned i = 0; i < height+1; ++i){
    // 	std::cerr << "\033[F";
    // }

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

    // Watchdog Network
    typedef Graph<SimpleProperty, SimpleProperty>  WatchdogGraph;
    typedef typename WatchdogGraph::Vertex         WVertex;
    typedef typename WatchdogGraph::Edge           WEdge;
    typedef typename WatchdogGraph::EdgeDescriptor WEdgeDescriptor;


    // Cal
    typedef CommunicationPolicy::MPI               Mpi;
    typedef CommunicationAbstractionLayer<Mpi>     MpiCAL;
    typedef typename MpiCAL::VAddr                 VAddr;
    typedef typename MpiCAL::Event                 Event;

    // GVON
    typedef GraphBasedVirtualOverlayNetwork<LifeGraph, MpiCAL>  GVON;
    typedef GraphBasedVirtualOverlayNetwork<WatchdogGraph, MpiCAL>  WGVON;


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
    WGVON wGvon(cal);

    // Create Watchdog Graph
    std::vector<WVertex> wGraphVertices;
    std::vector<WEdgeDescriptor> wEdges = Topology::star<WatchdogGraph>(cal.getGlobalContext().size(), wGraphVertices);
    WatchdogGraph wGraph(wEdges, wGraphVertices);


    // Distribute work evenly
    VAddr myVAddr      = cal.getGlobalContext().getVAddr();
    unsigned nVAddr = cal.getGlobalContext().size();
    std::vector<Vertex> hostedVertices = Distribute::roundrobin(myVAddr, nVAddr, graph);
    std::vector<WVertex> dog(1, wGraph.getVertices().at(myVAddr));

    // Announce vertices
    gvon.announce(graph, hostedVertices); 
    wGvon.announce(wGraph, dog);

    /***************************************************************************
     * Start Simulation
     ****************************************************************************/
    std::vector<Event> events;   
    std::vector<unsigned> golDomain(graph.getVertices().size(), 0); 

    const Vertex root = graph.getVertices().at(0);
    unsigned generation = 1;

    // Load balancing variables
    std::vector<unsigned> participants(1, 0);
    std::vector<unsigned> participate(1, !hostedVertices.empty());
    wGvon.allReduce(dog[0], wGraph, std::plus<unsigned>(), participate, participants);
    unsigned direction = 0;

    // Meassurement variables
    using namespace std::chrono;
    unsigned period = 10;
    unsigned timestep_begin = 0;
    unsigned timestep_end = timestep_begin + period;
    high_resolution_clock::time_point t1 = high_resolution_clock::now();;
    
    
    // Simulate life forever
    for(unsigned timestep = 0; timestep < nTimeSteps; ++timestep){

	// Meassurement
	if(myVAddr == 0){
	    if(timestep == timestep_end){
		high_resolution_clock::time_point t2 = high_resolution_clock::now();
		duration<double> timeSpan = duration_cast<duration<double>>(t2 - t1);
		std::cerr << generation << " " << participants[0] << " " << timeSpan.count() << std::endl;
	    
		t1 = high_resolution_clock::now();
		timestep_end += period;
	    }
	}


	
	// Dynamic Load balancing
	{
	    
	    if(participants[0] == 1){
	     	direction = 1;
	    }

	    if(hostedVertices.empty()){
		participate[0] = 0;
	    }
	    
	    if(direction == 0){
		if((myVAddr == participants[0] - 1) && (participants[0] > 1)){
		    if((generation % (period*1)) == 0){
			participate[0] = 0;
		    }
		}
	    }
	    else {
		if(myVAddr == participants[0]){
		    if((generation % (period*1)) == 0){
			participate[0] = 1;
		    }
		}

	    }

	    const unsigned tmpParticipants = participants[0];
	    wGvon.allReduce(dog[0], wGraph, std::plus<unsigned>(), participate, participants);

	    // if(myVAddr == 0){
	    // 	std::cout << participants[0] << std::endl;
	    // }
	    
	    if(tmpParticipants != participants[0]){
		hostedVertices = Distribute::roundrobin(myVAddr, participants[0], graph);
		gvon.announce(graph, hostedVertices);
	    }

	}
	
	// Print life field
	// if(myVAddr == 0){
	//     printGolDomain(golDomain, width, height, generation, participants[0]);
	// }

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

	//  // Gather state by vertex with id = 0
	 for(Vertex &v: hostedVertices){
	     v.aliveNeighbors = 0;
	     gvon.gatherNew(root, v, graph, v.isAlive, golDomain);
	 }

	 generation++;

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
