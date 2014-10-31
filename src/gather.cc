// Communication
#include <Graph.hpp>                         /* Graph */
#include <CommunicationAbstractionLayer.hpp> /* CommunicationAbstractionLayer */
#include <MPI.hpp>                           /* CommunicationPolicy::MPI*/
#include <GraphBasedVirtualOverlayNetwork.hpp>         /* GraphBasedVirtualOverlayNetwork */

// Helpers
#include <distribution.hpp> /* roundRobin */
#include <topology.hpp>     /* meshDiagonal */
#include <evals.hpp>        /* variance, median, avg */

// STL
#include <iostream>   /* std::cout */
#include <tuple>      /* std::pair */
#include <vector>     /* std::vector */
#include <array>      /* std::array */
#include <algorithm>  /* std::copy */
#include <functional> /* std::minus */

#include <chrono>     /* std::chrono::high_resolution_clock */

// C Header
#include <cmath>      /* sqrt, pow */
#include <cstdlib>    /* atoi */
#include <assert.h>   /* assert */
#include <cstdlib>    /* rand */


// MPI
#include <mpi.h>

template <typename T_Data>
int gatherCAL(const unsigned nPeers, const unsigned nElements, std::vector<double>& times) {
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

    //T_Data dataSend; 
    std::vector<T_Data> dataSend(1);
    std::vector<T_Data> dataRecv(nPeers * nElements);
    std::vector<unsigned> recvCount;

    VAddr root = 0;

    for(unsigned i = 0; i < times.size(); ++i){

	using namespace std::chrono;
	high_resolution_clock::time_point t1 = high_resolution_clock::now();
    
	cal.gather2(root, cal.getGlobalContext(), dataSend, dataRecv, recvCount);

	high_resolution_clock::time_point t2 = high_resolution_clock::now();
	duration<double> timeSpan = duration_cast<duration<double>>(t2 - t1);
	times[i] = timeSpan.count();
    }


    if(myVAddr == 0){
	return 1;
    }
    return 0;
}

template <typename T_Data>
int gatherGVON(const unsigned nPeers, unsigned nElements, std::vector<double>& times) {
    /***************************************************************************
     * Configuration
     ****************************************************************************/
    // Graph
    typedef Graph<SimpleProperty, SimpleProperty> NBodyGraph;
    typedef typename NBodyGraph::Vertex           Vertex;
    //typedef typename NBodyGraph::Edge             Edge;
    typedef typename NBodyGraph::EdgeDescriptor   EdgeDescriptor;

    // Cal
    typedef CommunicationPolicy::MPI            Mpi;
    typedef CommunicationAbstractionLayer<Mpi>  MpiCAL;
    typedef typename MpiCAL::VAddr              VAddr;

    // GVON
    typedef GraphBasedVirtualOverlayNetwork<NBodyGraph, MpiCAL>  GVON;


    /***************************************************************************
     * Init Communication
     ****************************************************************************/
    // Create Graph
    std::vector<Vertex> graphVertices;
    std::vector<EdgeDescriptor> edges = Topology::star<NBodyGraph>(nPeers, graphVertices);
    NBodyGraph graph (edges, graphVertices); 

    // Inantiate communication objects
    MpiCAL cal;
    GVON gvon(cal);

    // // Distribute work evenly
    VAddr myVAddr      = cal.getGlobalContext().getVAddr();
    unsigned nAddr     = cal.getGlobalContext().size();
    std::vector<Vertex> myGraphVertices = Distribute::roundrobin(myVAddr, nAddr, graph);

    // Announce vertices
    gvon.announce(graph, myGraphVertices); 

    /***************************************************************************
     * Start Test
     ****************************************************************************/

    //std::vector<T_Data> dataSend(nElements); 
    T_Data dataSend = 0; 
    std::vector<T_Data> dataRecv(nPeers * nElements);

    Vertex root = graph.getVertices().at(0);

    for(unsigned i = 0; i < times.size(); ++i){

	using namespace std::chrono;
	high_resolution_clock::time_point t1 = high_resolution_clock::now();
    
	for(Vertex v : myGraphVertices){
	    gvon.gatherNew(root, v, graph, dataSend, dataRecv);

	}

	high_resolution_clock::time_point t2 = high_resolution_clock::now();
	duration<double> timeSpan = duration_cast<duration<double>>(t2 - t1);
	times[i] = timeSpan.count();
    }
  
    if(myVAddr == 0){
	return 1;
    }
    return 0;

}

template <typename T_Data>
int gatherMPI(const unsigned nPeers, unsigned nElements, std::vector<double>& times){
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
  int root = 0;
  MPI_Comm_size(MPI_COMM_WORLD, &size);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);


  // Start Communication
  std::vector<T_Data> dataSend(nElements); 
  std::vector<T_Data> dataRecv(nElements * nPeers);


  for(unsigned i = 0; i < times.size(); ++i){

      using namespace std::chrono;
      high_resolution_clock::time_point t1 = high_resolution_clock::now();

      int rcounts[size];
      int rdispls[size];

      // Create recv buffer with sendsize information of other ranks
      MPI_Allgather(&nElements, 1, MPI_UNSIGNED, 
		    &rcounts, 1, MPI_UNSIGNED, 
		    MPI_COMM_WORLD);


      unsigned offset  = 0;
      for (int i=0; i < size; ++i) { 
	  rdispls[i] = offset; 
	  offset += rcounts[i];
      } 

      // Receive data with varying size
      MPI_Gatherv(dataSend.data(), dataSend.size(), MPI_INT, 
		  dataRecv.data(), rcounts, rdispls, MPI_INT, 
		  root, MPI_COMM_WORLD);

      high_resolution_clock::time_point t2 = high_resolution_clock::now();
      duration<double> timeSpan = duration_cast<duration<double>>(t2 - t1);
      times[i] = timeSpan.count();
  }
  
  MPI_Barrier(MPI_COMM_WORLD);
  MPI_Finalize();

  if(rank == 0){
      return 1;
  }
  return 0;

}


int main(int argc, char** argv){

    if(argc < 5){
	std::cout << "Usage ./SyntheticCollective [nPeers] [nElements] [nTimesteps] [0,1,2]" << std::endl;
	return 0;
    }

    // Benchmark parameter
    typedef int Data;
    unsigned nPeers      = atoi(argv[1]);
    unsigned nElements  = atoi(argv[2]);
    unsigned nTimesteps = atoi(argv[3]);
    unsigned mode       = atoi(argv[4]);


    bool printTime = 0;
    std::vector<double> runtimes(nTimesteps, 0.0);

    switch(mode){
    case 0:
	printTime = gatherMPI<Data>(nPeers, nElements, runtimes);
	break;
    case 1: 
	printTime = gatherCAL<Data>(nPeers, nElements, runtimes);
       break;
    case 2: 
	printTime = gatherGVON<Data>(nPeers, nElements, runtimes);
       break;

    default:
	break;
    };
    
    double avgTime = avg(runtimes);
    double varTime = variance(runtimes, avgTime);
    double devTime = sqrt(varTime);
    double medTime = median(runtimes);

    if(printTime){
	//std::cerr << "Time[s]: " << avgTime << " Variance: " << varTime << " Deviation: " << devTime << " Median: " << medTime << std::endl;

	// average, variance, deviation, median
	std::cerr << avgTime << " " << varTime << " " << devTime << " " << medTime << std::endl;
    }


    return 0;

}
