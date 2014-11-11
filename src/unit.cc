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
#include <vector>     /* std::vector */
#include <array>      /* std::array */
#include <algorithm>  /* std::copy, std::fill */
#include <functional> /* std::minus, std::plus */


// C Header
#include <assert.h>   /* assert */




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
     * Gather Test
     ****************************************************************************/
    {
	const VAddr root = 0;
	const unsigned testValue = 10;
	std::vector<unsigned> sendData(2, 0);
	std::vector<unsigned> recvData(context.size() * 2, 0);

	std::fill(sendData.begin(), sendData.end(), testValue);
	
	cal.gather(root, context, sendData, recvData);

	if(myVAddr == root){
	    for(unsigned d: recvData){
		assert(d == testValue);
	    
	    }
	}
	
    }

    /***************************************************************************
     * GatherVar Test
     ****************************************************************************/
    {
	const VAddr root = 0;
	const unsigned testValue = 10;
	std::vector<unsigned> sendData(myVAddr+1, 0);
	std::vector<unsigned> recvData;
	std::vector<unsigned> recvCount;

	std::fill(sendData.begin(), sendData.end(), testValue);
	
	cal.gatherVar(root, context, sendData, recvData, recvCount);

	if(myVAddr == root){
	    for(unsigned d: recvData){
		assert(d == testValue);
	    
	    }
	}
	
    }

    /***************************************************************************
     * AllGather Test
     ****************************************************************************/
    {
	const unsigned testValue = 10;
	std::vector<unsigned> sendData(2, 0);
	std::vector<unsigned> recvData(context.size() * 2, 0);

	std::fill(sendData.begin(), sendData.end(), testValue);
	
	cal.allGather(context, sendData, recvData);

	for(unsigned d: recvData){
	    assert(d == testValue);
	    
	}
	
    }


    /***************************************************************************
     * AllGatherVar Test
     ****************************************************************************/
    {
	const unsigned testValue = 10;
	std::vector<unsigned> sendData(2, 0);
	std::vector<unsigned> recvData;
	std::vector<unsigned> recvCount;

	std::fill(sendData.begin(), sendData.end(), testValue);
	
	cal.allGatherVar(context, sendData, recvData, recvCount);

	for(unsigned d: recvData){
	    assert(d == testValue);
	    
	}
	
    }

    
    
    /***************************************************************************
     * Scatter Test
     ****************************************************************************/
    {
	const VAddr root = 0;
	const unsigned testValue = 10;
	std::vector<unsigned> sendData(context.size() * 2, 0);
	std::vector<unsigned> recvData(2, 0);

	if(myVAddr == root){
	    std::fill(sendData.begin(), sendData.end(), testValue);
	
	}

	cal.scatter(root, context, sendData, recvData);
	for(unsigned d: recvData){
	    assert(d == testValue);
	    
	}
	
    }

    
    /***************************************************************************
     * All to All Test
     ****************************************************************************/
    {
    	unsigned testValue = 10;
    	std::vector<unsigned> sendData(context.size() * 5, 0);
    	std::vector<unsigned> recvData(context.size() * 5, 0);

	std::fill(sendData.begin(), sendData.end(), testValue);

    	cal.allToAll(context, sendData, recvData);
    	for(unsigned d: recvData){
    	    assert(d == testValue);
	    
    	}

    }


    /***************************************************************************
     * Reduce Test
     ****************************************************************************/
    {
	const VAddr root = 0;
	std::vector<unsigned> sendData(context.size(), 1);
	std::vector<unsigned> recvData(context.size(), 0);
	cal.reduce(root, context, std::plus<unsigned>(), sendData, recvData);

	if(myVAddr == root){
	    for(unsigned d : recvData){
		assert(d == context.size());
	    
	    }
	    
	}
	    
    }

    
    /***************************************************************************
     * All Reduce Test
     ****************************************************************************/
    {
	std::vector<unsigned> sendData(context.size(), 1);
	std::vector<unsigned> recvData(context.size(), 0);
	cal.allReduce(context, std::plus<unsigned>(), sendData, recvData);

	for(unsigned d : recvData){
	    assert(d == context.size());
	    
	}
	    
	    
    }

    
    /***************************************************************************
     * Broadcast Test
     ****************************************************************************/
    {
	const VAddr root = 0;
	unsigned testValue = 10;
	std::vector<unsigned> data(context.size(), 0);

	if(myVAddr == root){
	    std::fill(data.begin(), data.end(), testValue);
	}

	
	cal.broadcast(root, context, data);

	for(unsigned d : data){
	    assert(d == testValue);
	    
	}
	    
	    
    }
    
}


void gvonUnitTest(){
    // Graph
    typedef Graph<SimpleProperty, SimpleProperty>   MyGraph;
    typedef typename MyGraph::Vertex                Vertex;
    typedef typename MyGraph::Edge                  Edge;
    typedef typename MyGraph::EdgeDescriptor        EdgeDescriptor;

    // Cal
    typedef CommunicationPolicy::MPI                  Mpi;
    typedef CommunicationAbstractionLayer<Mpi>        MpiCAL;
    typedef typename MpiCAL::VAddr                    VAddr;
    typedef typename MpiCAL::Event                    Event;

    // GVON
    typedef GraphBasedVirtualOverlayNetwork<MyGraph, MpiCAL>  GVON;


    /***************************************************************************
     * Init Communication
     ****************************************************************************/
    // Create Graph
    const unsigned height = 10;
    const unsigned width  = height;
    std::vector<Vertex> graphVertices;
    std::vector<EdgeDescriptor> edges = Topology::gridDiagonal<MyGraph>(height, width, graphVertices);
    MyGraph graph (edges, graphVertices); 

    // Inantiate communication objects
    MpiCAL cal;
    GVON gvon(cal);

    // Distribute work evenly
    VAddr myVAddr  = cal.getGlobalContext().getVAddr();
    unsigned nAddr = cal.getGlobalContext().size();
    std::vector<Vertex> hostedVertices = Distribute::consecutive(myVAddr, nAddr, graph);

    // Announce vertices
    gvon.announce(graph, hostedVertices);


    
    /***************************************************************************
     * asyncSend/Recv Test
     ****************************************************************************/
    {
	const unsigned testValue = 10;
	
	std::vector<unsigned> sendData(10, testValue);
	std::vector<unsigned> recvData(10, 0);
    
	for(Vertex v : hostedVertices){
	    for(std::pair<Vertex, Edge> edge : graph.getOutEdges(v)){
		Vertex targetVertex = edge.first;
		Edge   outEdge      = edge.second;
		Event e = gvon.asyncSend(graph, targetVertex, outEdge, sendData);
	    }

	}

	for(Vertex v : hostedVertices){
	    for(std::pair<Vertex, Edge> edge : graph.getInEdges(v)){
		Vertex sourceVertex = edge.first;
		Edge   inEdge       = edge.second;
		gvon.recv(graph, sourceVertex, inEdge, recvData);
		for(unsigned d : recvData){
		    assert(d == testValue);

		}
		
	    }

	}

    }
    
}



int main(){

    calUnitTest();
    //gvonUnitTest();
    return 0;

}
