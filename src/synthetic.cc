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
#include <array>      /* std::array */
#include <algorithm>  /* std::copy */
#include <cmath>      /* sqrt, pow */
#include <cstdlib>    /* atoi */
#include <assert.h>   /* assert */
#include <cstdlib>    /* rand */
#include <chrono>     /* std::chrono::high_resolution_clock */

// MPI
#include <mpi.h>


int sendCAL(const unsigned N) {
    /***************************************************************************
     * Configuration
     ****************************************************************************/

    // Cal
    typedef CommunicationPolicy::MPI            Mpi;
    typedef CommunicationAbstractionLayer<Mpi>  MpiCAL;
    typedef typename MpiCAL::VAddr              VAddr;
    typedef typename MpiCAL::Context            Context;

    /***************************************************************************
     * Init Communication
     ****************************************************************************/
    // Initiate communication objects
    MpiCAL cal;
    VAddr myVAddr = cal.getGlobalContext().getVAddr();
    Context context = cal.getGlobalContext();

    /***************************************************************************
     * Start Test
     ****************************************************************************/

    std::array<int, 1> dataSend {{1}};
    std::array<int, 1> dataRecv {{0}};

    using namespace std::chrono;
    high_resolution_clock::time_point t1 = high_resolution_clock::now();
    
    for(unsigned timestep = 0; timestep < N; ++timestep){

	switch(myVAddr) {
	case 0:
	    cal.send(1, 0, context, dataSend);
	    break;
	case 1:
	    cal.recv(0, 0, context, dataRecv);
	    break;
	    
	default:
	    break;
	};

    }

    high_resolution_clock::time_point t2 = high_resolution_clock::now();
    duration<double> time_span = duration_cast<duration<double>>(t2 - t1);
    if(myVAddr == 0) std::cout << "CAL Time[s]: " << time_span.count() << std::endl;

    return 0;
}


int sendGVON(const unsigned N) {
    /***************************************************************************
     * Configuration
     ****************************************************************************/
    // Graph
    typedef Graph<SimpleProperty, SimpleProperty> NBodyGraph;
    typedef typename NBodyGraph::Vertex           Vertex;
    typedef typename NBodyGraph::Edge             Edge;
    typedef typename NBodyGraph::EdgeDescriptor   EdgeDescriptor;

    // Cal
    typedef CommunicationPolicy::MPI            Mpi;
    typedef CommunicationAbstractionLayer<Mpi>  MpiCAL;
    typedef typename MpiCAL::VAddr              VAddr;

    // GVON
    typedef VirtualOverlayNetwork<NBodyGraph, MpiCAL>  GVON;


    /***************************************************************************
     * Init Communication
     ****************************************************************************/
    // Create Graph
    std::vector<Vertex> graphVertices;
    std::vector<EdgeDescriptor> edges = Topology::star<NBodyGraph>(2, graphVertices);
    NBodyGraph graph (edges, graphVertices); 

    // Inantiate communication objects
    MpiCAL cal;
    GVON gvon(cal);

    // // Distribute work evenly
    VAddr myVAddr      = cal.getGlobalContext().getVAddr();
    unsigned nAddr     = cal.getGlobalContext().size();
    std::vector<Vertex> myGraphVertices = Distribute::roundRobin(myVAddr, nAddr, graph);

    // Announce vertices
    gvon.announce(graph, myGraphVertices); 

    /***************************************************************************
     * Start Test
     ****************************************************************************/

    std::array<int, 1> dataSend {{1}};
    std::array<int, 1> dataRecv {{0}};

    using namespace std::chrono;
    high_resolution_clock::time_point t1 = high_resolution_clock::now();
    
    for(unsigned timestep = 0; timestep < N; ++timestep){
	for(Vertex v : myGraphVertices){
	    for(std::pair<Vertex, Edge> edge : graph.getOutEdges(v)){
		gvon.send(graph, edge.first, edge.second, dataSend);
	    }
	}

	for(Vertex v : myGraphVertices){
	    for(std::pair<Vertex, Edge> edge : graph.getInEdges(v)){
		gvon.recv(graph, edge.first, edge.second, dataRecv);
	    }

	}

    }

    high_resolution_clock::time_point t2 = high_resolution_clock::now();
    duration<double> time_span = duration_cast<duration<double>>(t2 - t1);
    if(myVAddr == 0) std::cout << "GVON Time[s]: " << time_span.count() << std::endl;

    return 0;
}


int sendMPI(const unsigned N){
    // Init MPI
    int mpiError = MPI_Init(NULL,NULL);
    if(mpiError != MPI_SUCCESS){
	std::cout << "Error starting MPI program." << std::endl;
	MPI_Abort(MPI_COMM_WORLD,mpiError);
	return 1;
    }

  // Get size and rank
  int rank;
  int size;
  MPI_Status status;
  MPI_Comm_size(MPI_COMM_WORLD, &size);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);


  // Start Communication
  std::array<int, 1> dataSend {{1}};
  std::array<int, 1> dataRecv {{0}};

  using namespace std::chrono;
  high_resolution_clock::time_point t1 = high_resolution_clock::now();


  for(unsigned timestep = 0; timestep < N; ++timestep){
      switch(rank) {

      case 0 :
	  MPI_Send(dataSend.data(), dataSend.size(), MPI_INT, 1, 0, MPI_COMM_WORLD);
	  break;

      case 1:
	  MPI_Recv(dataRecv.data(), dataRecv.size(), MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
	  break;

      default:
	  break;

      };
  }

  
  high_resolution_clock::time_point t2 = high_resolution_clock::now();
  duration<double> time_span = duration_cast<duration<double>>(t2 - t1);
  if(rank == 0) std::cout << "MPI Time[s]: " << time_span.count() << std::endl;

  MPI_Barrier(MPI_COMM_WORLD);
  MPI_Finalize();

  return 0;

}


int main(int argc, char** argv){

    if(argc < 1){
	std::cout << "Usage ./Synthetic [N] [0,1,2]" << std::endl;

    }

    unsigned mode = atoi(argv[2]);
    unsigned N = atoi(argv[1]);

    switch(mode){
    case 0:
	sendMPI(N);
	break;
    case 1: 
       sendCAL(N);
       break;
    case 2: 
       sendGVON(N);
       break;

    default:
	break;
    };

    return 0;

}
