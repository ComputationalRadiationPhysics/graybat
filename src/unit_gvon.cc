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

void test(){
    // Graph
    typedef Graph<SimpleProperty, SimpleProperty> MyGraph;
    typedef typename MyGraph::Vertex              Vertex;
    typedef typename MyGraph::Edge                Edge;
    typedef typename MyGraph::EdgeDescriptor      EdgeDescriptor;

    // Cal
    typedef CommunicationPolicy::MPI              Mpi;
    typedef CommunicationAbstractionLayer<Mpi>    MpiCAL;
    typedef typename MpiCAL::VAddr                VAddr;
    typedef typename MpiCAL::Event                Event;

    // GVON
    typedef GraphBasedVirtualOverlayNetwork<MyGraph, MpiCAL>  GVON;


    /***************************************************************************
     * Init Communication
     ****************************************************************************/
    // Create Graph
    const unsigned height = 10;
    const unsigned width  = height;
    std::vector<Vertex> graphVertices;
    std::vector<EdgeDescriptor> edges = Topology::grid<MyGraph>(height, width, graphVertices);
    MyGraph graph (edges, graphVertices); 

    // Inantiate communication objects
    MpiCAL cal;
    GVON gvon(cal);

    // Distribute work evenly
    VAddr myVAddr  = cal.getGlobalContext().getVAddr();
    unsigned nAddr = cal.getGlobalContext().size();
    std::vector<Vertex> hostedVertices = Distribute::roundrobin(myVAddr, nAddr, graph);

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

    test();
    return 0;

}
