// Communication
#include <Graph.hpp>                           /* Graph */
#include <CommunicationAbstractionLayer.hpp>   /* CommunicationAbstractionLayer */
#include <MPI.hpp>                             /* CommunicationPolicy::MPI*/
#include <GraphBasedVirtualOverlayNetwork.hpp> /* GraphBasedVirtualOverlayNetwork */

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
#include <functional> /* std::minus, std::plus*/

#include <chrono>     /* std::chrono::high_resolution_clock */

// C Header
#include <cmath>      /* sqrt, pow */
#include <cstdlib>    /* atoi */
#include <assert.h>   /* assert */
#include <cstdlib>    /* rand */


// MPI
#include <mpi.h>
template <typename T_Data>
int reduceMPI(unsigned nElements, std::vector<double>& times){
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
  std::vector<T_Data> dataSend(nElements, 1); 
  std::vector<T_Data> dataRecv(nElements, 0);


  for(unsigned i = 0; i < times.size(); ++i){

      using namespace std::chrono;
      high_resolution_clock::time_point t1 = high_resolution_clock::now();

      MPI_Reduce(dataSend.data(), dataRecv.data(), dataSend.size(), 
       		 MPI_INT, MPI_SUM, 
		 root, MPI_COMM_WORLD);

      high_resolution_clock::time_point t2 = high_resolution_clock::now();
      duration<double> timeSpan = duration_cast<duration<double>>(t2 - t1);
      times[i] = timeSpan.count();
  }

  // if(rank == root){
  //     for(T_Data d : dataRecv){
  // 	  assert(d == size);
  //     }
  // }
  
  MPI_Barrier(MPI_COMM_WORLD);
  MPI_Finalize();

  if(rank == 0){
      return 1;
  }
  return 0;

}


template <typename T_Data>
int reduceCAL(const unsigned nElements, std::vector<double>& times) {
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
    std::vector<T_Data> dataSend(nElements, 1);
    std::vector<T_Data> dataRecv(nElements, 0);
    std::vector<unsigned> recvCount;

    VAddr root = 0;

    for(unsigned i = 0; i < times.size(); ++i){

	using namespace std::chrono;
	high_resolution_clock::time_point t1 = high_resolution_clock::now();
    
	cal.reduce(root, cal.getGlobalContext(), std::plus<T_Data>(), dataSend, dataRecv);

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
int reduceGVON(const unsigned nElements, std::vector<double>& times) {
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
    // Inantiate communication objects
    MpiCAL cal;
    GVON gvon(cal);

    // Create Graph
    std::vector<Vertex> graphVertices;
    std::vector<EdgeDescriptor> edges = Topology::star<NBodyGraph>(cal.getGlobalContext().size(), graphVertices);
    NBodyGraph graph (edges, graphVertices); 


    // // Distribute work evenly
    VAddr myVAddr      = cal.getGlobalContext().getVAddr();
    unsigned nAddr     = cal.getGlobalContext().size();
    std::vector<Vertex> myGraphVertices = Distribute::consecutive(myVAddr, nAddr, graph);

    assert(myGraphVertices.size() == 1);
    
    // Announce vertices
    gvon.announce(graph, myGraphVertices); 

    /***************************************************************************
     * Start Test
     ****************************************************************************/

    //std::vector<T_Data> dataSend(nElements); 
    std::vector<T_Data> dataSend(nElements, 1);
    std::vector<T_Data> dataRecv(nElements, 0);

    Vertex root = graph.getVertices().at(0);

    for(unsigned i = 0; i < times.size(); ++i){

	using namespace std::chrono;
	high_resolution_clock::time_point t1 = high_resolution_clock::now();
    
	for(Vertex v : myGraphVertices){
	    gvon.reduceNew(root, v, graph, std::plus<T_Data>(), dataSend, dataRecv);

	}

	high_resolution_clock::time_point t2 = high_resolution_clock::now();
	duration<double> timeSpan = duration_cast<duration<double>>(t2 - t1);
	times[i] = timeSpan.count();
    }

    // if(myVAddr == gvon.locateVertex(graph, root)){
    // 	for(T_Data d : dataRecv){
    // 	    //std::cout << d << std::endl;
    // 	    assert(d == cal.getGlobalContext().size());

    // 	}
    // }
  
    if(myVAddr == 0){
	return 1;
    }
    return 0;

}



int main(int argc, char** argv){

    if(argc < 5){
	std::cout << "Usage ./Reduce [nElements] [nTimesteps] [0,1,2]" << std::endl;
	return 0;
    }

    // Benchmark parameter
    typedef int Data;
    unsigned nElements  = atoi(argv[2]);
    unsigned nTimesteps = atoi(argv[3]);
    unsigned mode       = atoi(argv[4]);


    bool printTime = 0;
    std::vector<double> runtimes(nTimesteps, 0.0);

    switch(mode){
    case 0:
	printTime = reduceMPI<Data>(nElements, runtimes);
	break;
    case 1: 
	printTime = reduceCAL<Data>(nElements, runtimes);
       break;
    case 2: 
	printTime = reduceGVON<Data>(nElements, runtimes);
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
