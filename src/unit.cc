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
#include <algorithm>  /* std::copy, std::fill */
#include <functional> /* std::minus */

#include <chrono>     /* std::chrono::high_resolution_clock */

// C Header
#include <cmath>      /* sqrt, pow */
#include <cstdlib>    /* atoi */
#include <assert.h>   /* assert */
#include <cstdlib>    /* rand */



void calUnitTest(){
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
    VAddr myVAddr   = cal.getGlobalContext().getVAddr();
    Context context = cal.getGlobalContext();

    /***************************************************************************
     * Scatter Test
     ****************************************************************************/
    {
	const VAddr root = 0;
	const int testValue = 10;
	std::vector<int> sendData(context.size() * 2, 0);
	std::vector<int> recvData(2, 0);

	if(myVAddr == root){
	    std::fill(sendData.begin(), sendData.end(), testValue);
	
	}

	cal.scatter(root, context, sendData, recvData);
	for(int i: recvData){
	    assert(i == testValue);
	    
	}
	
    }
    /***************************************************************************
     * All to All Test
     ****************************************************************************/
    // {
    // 	VAddr root = 0;
    // 	int testValue = 10;
    // 	std::vector<int> sendData(2, 0);
    // 	std::vector<int> recvData(context.size() * 2, 0);

    // 	if(myVAddr == root){
    // 	    std::fill(sendData.begin(), sendData.end(), testValue);
	
    // 	}

    // 	cal.allToAll(context, sendData, recvData);
    // 	for(int i: recvData){
    // 	    assert(recvData[i] == testValue);
    // 	}

    // }
    
    
}


int main(){

    calUnitTest();


}
