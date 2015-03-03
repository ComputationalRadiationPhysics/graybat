// Communication
#include <graybat.hpp>

// STL
#include <iostream>   /* std::cout */
#include <vector>     /* std::vector */
#include <array>      /* std::array */
#include <cmath>      /* sqrt */
#include <cstdlib>    /* atoi */


struct Cell : public graybat::graphPolicy::SimpleProperty{
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

    // CommunicationPolicy
    typedef graybat::communicationPolicy::BMPI CP;
    
    // GraphPolicy
    typedef graybat::graphPolicy::BGL<Cell>   GP;
    
    // Cave
    typedef graybat::Cave<CP, GP>   MyCave;
    typedef typename MyCave::Event  Event;
    typedef typename MyCave::Vertex Vertex;
    typedef typename MyCave::Edge   Edge;

    /***************************************************************************
     * Initialize Communication
     ****************************************************************************/
    //Create Graph
    const unsigned height = sqrt(nCells);
    const unsigned width  = height;

    // Create GoL Graph
    MyCave cave(graybat::pattern::GridDiagonal(height, width));
    
    // Distribute vertices
    cave.distribute(graybat::mapping::Random(1234));

    
    /***************************************************************************
     * Run Simulation
     ****************************************************************************/
     std::vector<Event> events;   
     std::vector<unsigned> golDomain(cave.getVertices().size(), 0); 
     const Vertex root = cave.getVertex(0);

    // // Simulate life 
     for(unsigned timestep = 0; timestep < nTimeSteps; ++timestep){

    	// Print life field by owner of vertex 0
    	if(cave.peerHostsVertex(root)){
    	    printGolDomain(golDomain, width, height, timestep);
    	}
	
    	// Send state to neighbor cells
    	for(Vertex &v : cave.hostedVertices){
    	    for(auto link : cave.getOutEdges(v)){
    		Vertex destVertex = link.first;
    		Edge   destEdge   = link.second;
    		events.push_back(cave.asyncSend(destVertex, destEdge, v.isAlive));
    	    }
    	}

     	// Recv state from neighbor cells
     	for(Vertex &v : cave.hostedVertices){
    	    for(auto link : cave.getInEdges(v)){
    		Vertex srcVertex = link.first;
    		Edge   srcEdge   = link.second;
    		cave.recv(srcVertex, srcEdge, srcVertex.isAlive);
    		if(srcVertex.isAlive[0]) v.aliveNeighbors++;
    	    }
    	}

    	// Wait to finish events
    	for(unsigned i = 0; i < events.size(); ++i){
    	    events.back().wait();
    	    events.pop_back();
    	}

    	// Calculate state for next generation
    	updateState(cave.hostedVertices);

     	// Gather state by vertex with id = 0
    	for(Vertex &v: cave.hostedVertices){
    	    v.aliveNeighbors = 0;
    	    cave.gather(root, v, v.isAlive, golDomain, true);
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
