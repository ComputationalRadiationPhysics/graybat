// Communication
#include <Graph.hpp>                         /* Graph */
#include <CommunicationAbstractionLayer.hpp> /* CommunicationAbstractionLayer */
#include <MPI.hpp>                           /* CommunicationPolicy::MPI*/
#include <VirtualOverlayNetwork.hpp>         /* VirtualOverlayNetwork */

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
int sendCAL(const unsigned N, const unsigned nSend, std::vector<double>& times) {
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

    std::vector<T_Data> dataSend(nSend); 
    std::vector<T_Data> dataRecv(nSend);

    for(unsigned i = 0; i < times.size(); ++i){

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
	duration<double> timeSpan = duration_cast<duration<double>>(t2 - t1);
	times[i] = timeSpan.count();
    }


  if(myVAddr == 0){
      return 1;
  }
  return 0;
}

template <typename T_Data>
int sendGVON(const unsigned N, const unsigned nSend, std::vector<double>& times) {
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

    std::vector<T_Data> dataSend(nSend); 
    std::vector<T_Data> dataRecv(nSend);

    for(unsigned i = 0; i < times.size(); ++i){

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
	duration<double> timeSpan = duration_cast<duration<double>>(t2 - t1);
	times[i] = timeSpan.count();
    }
  
  if(myVAddr == 0){
      return 1;
  }
  return 0;

}

template <typename T_Data>
int sendMPI(const unsigned N, const unsigned nSend, std::vector<double>& times){
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
  MPI_Status status;
  MPI_Comm_size(MPI_COMM_WORLD, &size);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);


  // Start Communication
  std::vector<T_Data> dataSend(nSend); 
  std::vector<T_Data> dataRecv(nSend);


  for(unsigned i = 0; i < times.size(); ++i){

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

    if(argc < 4){
	std::cout << "Usage ./Synthetic [nSend] [nElements] [nRuns] [0,1,2]" << std::endl;
	return 0;
    }

    // Benchmark parameter
    typedef int Data;
    unsigned nSend     = atoi(argv[1]);
    unsigned nElements = atoi(argv[2]);
    unsigned nRuns     = atoi(argv[3]);
    unsigned mode      = atoi(argv[4]);


    bool printTime = 0;
    std::vector<double> runtimes(nRuns, 0.0);

    switch(mode){
    case 0:
	printTime = sendMPI<Data>(nSend, nElements, runtimes);
	break;
    case 1: 
	printTime = sendCAL<Data>(nSend, nElements, runtimes);
       break;
    case 2: 
	printTime = sendGVON<Data>(nSend, nElements, runtimes);
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
