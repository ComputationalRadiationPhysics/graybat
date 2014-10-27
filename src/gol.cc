// Communication
#include <Graph.hpp>                         /* Graph */
#include <CommunicationAbstractionLayer.hpp> /* CommunicationAbstractionLayer */
#include <MPI.hpp>                           /* CommunicationPolicy::MPI*/
#include <VirtualOverlayNetwork.hpp>         /* VirtualOverlayNetwork */

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
	
    std::array<unsigned,1> isAlive;
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

std::vector<unsigned> generateNeighborRanks(const unsigned rank, const unsigned width, const unsigned size){

    std::vector<unsigned> neighbors;

    // UP
    if(rank >= width){
	neighbors.push_back(rank - width);
    }

    // UP LEFT
    if(rank >= width and (rank % width) != 0){
	neighbors.push_back(rank - width - 1);
    }

    // UP RIGHT
    if(rank >= width and (rank % width) != (width - 1)){
	neighbors.push_back(rank - width + 1);
    }

    // DOWN
    if(rank < (size - width)){
	neighbors.push_back(rank + width);
    }

    // DOWN LEFT
    if(rank < (size - width) and (rank % width) != 0){
	neighbors.push_back(rank + width - 1);
    }

    // DOWN RIGHT
    if(rank < (size - width) and (rank % width) != (width - 1)){
	neighbors.push_back(rank + width + 1);
    }

    // RIGHT
    if((rank % width) != (width - 1)){
	neighbors.push_back(rank + 1);
    }

    // LEFT
    if((rank % width) != 0){
	neighbors.push_back(rank - 1);
    }

    return neighbors;
}


int golMPI(const unsigned nCells, std::vector<double> &times){
    // Init MPI
    int mpiError = MPI_Init(NULL,NULL);
    if(mpiError != MPI_SUCCESS){
	std::cout << "Error starting MPI program." << std::endl;
	MPI_Abort(MPI_COMM_WORLD,mpiError);
	return 0;
    }

    // Get size and rank
    int rank;
    int size;
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    assert(nCells == (unsigned)size);

    const unsigned width  = unsigned(sqrt(nCells));
    size = width * width;
    unsigned ops = 0;
    if(rank < size) {

	std::array<unsigned, 1> isAlive;
	Cell myCell(rank);
	std::vector<unsigned> neighbors = generateNeighborRanks(rank, width, size);
	std::vector<MPI_Request> requests;


	for(unsigned timestep = 0; timestep < times.size(); ++timestep){
	    using namespace std::chrono;
	    high_resolution_clock::time_point t1 = high_resolution_clock::now();

	    // Send state to neighbor cells
	    for(unsigned neighbor : neighbors){
		MPI_Request request;
		MPI_Issend(myCell.isAlive.data(), myCell.isAlive.size(), MPI_UNSIGNED, neighbor, 0, MPI_COMM_WORLD, &request);
		requests.push_back(request);
		ops++;
	    }
	
	
	    // Recv state from neighbor cells
	    for(unsigned neighbor : neighbors){
		MPI_Status status;

		MPI_Recv(isAlive.data(), isAlive.size(), MPI_UNSIGNED, neighbor, 0, MPI_COMM_WORLD, &status);
		ops++;

		if(isAlive[0]) myCell.aliveNeighbors++;

	    } 
	
	    // Wait to finish events
	    for(unsigned i = 0; i < requests.size(); ++i){
	    	MPI_Status status;
	    	MPI_Wait(&(requests.back()), &status);
		requests.pop_back();
	    }
	
	    // Calculate state for next generation
	    updateState(myCell);

	    high_resolution_clock::time_point t2 = high_resolution_clock::now();
	    duration<double> timeSpan = duration_cast<duration<double>>(t2 - t1);
	    times[timestep] = timeSpan.count();

	}

    }
    MPI_Barrier(MPI_COMM_WORLD);
    MPI_Finalize();

    if(rank == 0){
	std::cout << "Opeartions: " << ops << std::endl;
	return 1;
    }
    return 0;

}


int gol(const unsigned nCells, std::vector<double> &times) {
    /***************************************************************************
     * Configuration
     ****************************************************************************/
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
    const unsigned height = unsigned(sqrt(nCells));
    const unsigned width  = unsigned(sqrt(nCells));
    std::vector<Vertex> graphVertices;
    std::vector<EdgeDescriptor> edges = Topology::gridDiagonal<LifeGraph>(height, width, graphVertices);
    LifeGraph graph (edges, graphVertices); 

    // Inantiate communication objects
    MpiCAL cal;
    GVON gvon(cal);

    // Distribute work evenly
    VAddr myVAddr      = cal.getGlobalContext().getVAddr();
    unsigned nAddr = cal.getGlobalContext().size();
    std::vector<Vertex> hostedVertices = Distribute::roundRobin(myVAddr, nAddr, graph);

    // Announce vertices
    gvon.announce(graph, hostedVertices); 


    /***************************************************************************
     * Start Simulation
     ****************************************************************************/
    //unsigned generation = 0;
    std::vector<Event> events;   
    std::vector<unsigned> golDomain(graph.getVertices().size(), 0); 

    //unsigned generation = 0;

    unsigned ops = 0;

    // Simulate life forever
    for(unsigned timestep = 0; timestep < times.size(); ++timestep){
	using namespace std::chrono;
	high_resolution_clock::time_point t1 = high_resolution_clock::now();


	// Print life field
	// if(myVAddr == 0){
	//     printGolDomain(golDomain, width, height, generation);
	// }

	// Send state to neighbor cells
	for(Vertex v : hostedVertices){
	    for(std::pair<Vertex, Edge> edge : graph.getOutEdges(v)){
		events.push_back(gvon.asyncSend(graph, edge.first, edge.second, v.isAlive));
		ops++;
	    }
	}

	// Recv state from neighbor cells
	for(Vertex &v : hostedVertices){
	    for(std::pair<Vertex, Edge> edge : graph.getInEdges(v)){
		gvon.recv(graph, edge.first, edge.second, edge.first.isAlive);
		ops++;
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
	// for(Vertex &v: hostedVertices){
	//     v.aliveNeighbors = 0;
	//     gvon.gather(graph.getVertices().at(0), v, graph, v.isAlive[0], golDomain);
	// }

	//generation++;

	high_resolution_clock::time_point t2 = high_resolution_clock::now();
	duration<double> timeSpan = duration_cast<duration<double>>(t2 - t1);
	times[timestep] = timeSpan.count();
    }
    
    

    if(myVAddr == 0){
	std::cout << "Operations: " << ops << std::endl;
	return 1;
    }
    return 0;

}

int main(int argc, char** argv){

    if(argc < 4){
	std::cout << "Usage ./GoL [nCells] [nTimeSteps] [0,1]" << std::endl;
	return 0;
    }

    // Benchmark parameter
    unsigned nCells     = atoi(argv[1]);
    unsigned nTimesteps = atoi(argv[2]);
    unsigned mode       = atoi(argv[3]);
    

    bool printTime = 0;
    std::vector<double> runtimes(nTimesteps, 0.0);

    switch(mode){

    case 0:
	printTime = golMPI(nCells, runtimes);
	break;

    case 1:
	printTime = gol(nCells, runtimes);
	break;
       
    default:
	break;

    };

    double avgTime = avg(runtimes);
    double varTime = variance(runtimes, avgTime);
    double devTime = sqrt(varTime);
    double medTime = median(runtimes);

    if(printTime){
	//std::cout << "Time[s]: " << avgTime << " Variance: " << varTime << " Deviation: " << devTime << " Median: " << medTime << std::endl;

	// average, variance, deviation, median
	std::cerr << avgTime << " " << varTime << " " << devTime << " " << medTime << std::endl;
    }

    return 0;
}
