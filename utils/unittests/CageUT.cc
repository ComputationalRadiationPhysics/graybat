// boost 
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE Cage
#include <boost/test/unit_test.hpp>

// graybat
#include <graybat.hpp>
#include <mapping/Random.hpp>
#include <mapping/Consecutive.hpp>

// stl
#include <array>
#include <vector>
#include <iostream>
#include <functional>

/***************************************************************************
 * Configuration
 ****************************************************************************/

// CommunicationPolicy
typedef graybat::communicationPolicy::BMPI CP;
    
// GraphPolicy
typedef graybat::graphPolicy::BGL<>   GP;
    
// Cave
typedef graybat::Cave<CP, GP>   MyCave;
typedef typename MyCave::Event  Event;
typedef typename MyCave::Vertex Vertex;
typedef typename MyCave::Edge   Edge;


BOOST_AUTO_TEST_SUITE(point_to_point)

MyCave allToAll(graybat::pattern::FullyConnected(2));
MyCave star(graybat::pattern::Star(2));

BOOST_AUTO_TEST_CASE( send_recv ){
    star.distribute(graybat::mapping::Consecutive());
    const unsigned nElements = 1000;
    
    std::vector<Event> events; 
    std::vector<unsigned> send(nElements,0);
    std::vector<unsigned> recv(nElements,0);

    for(unsigned i = 0; i < send.size();++i){
	send.at(i) = i;
    }

    // Send state to neighbor cells
    for(Vertex &v : star.hostedVertices){
	for(auto link : star.getOutEdges(v)){
	    Vertex destVertex = link.first;
	    Edge   destEdge   = link.second;
	    star.send(destVertex, destEdge, send);
	    
	}
    }

    // Recv state from neighbor cells
    for(Vertex &v : star.hostedVertices){
	for(auto link : star.getInEdges(v)){
	    Vertex srcVertex = link.first;
	    Edge   srcEdge   = link.second;

	    star.recv(srcVertex, srcEdge, recv);
	    for(unsigned i = 0; i < recv.size();++i){
		BOOST_CHECK_EQUAL(recv.at(i), i);
	    }

	}
	
    }
    
}


BOOST_AUTO_TEST_CASE( asyncSend_recv ){

    allToAll.distribute(graybat::mapping::Consecutive());

    const unsigned nElements = 1000;
    
    std::vector<Event> events; 
    std::vector<unsigned> send(nElements,0);
    std::vector<unsigned> recv(nElements,0);

    for(unsigned i = 0; i < send.size();++i){
	send.at(i) = i;
    }

    // Send state to neighbor cells
    for(Vertex &v : allToAll.hostedVertices){
	for(auto link : allToAll.getOutEdges(v)){
	    Vertex destVertex = link.first;
	    Edge   destEdge   = link.second;
	    events.push_back(allToAll.asyncSend(destVertex, destEdge, send));
	}
    }

    // Recv state from neighbor cells
    for(Vertex &v : allToAll.hostedVertices){
	for(auto link : allToAll.getInEdges(v)){
	    Vertex srcVertex = link.first;
	    Edge   srcEdge   = link.second;

	    allToAll.recv(srcVertex, srcEdge, recv);
	    for(unsigned i = 0; i < recv.size();++i){
		BOOST_CHECK_EQUAL(recv.at(i), i);
	    }

	}
	
    }
    
    // Wait to finish events
    for(unsigned i = 0; i < events.size(); ++i){
	events.back().wait();
	events.pop_back();
    }

}


BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE( collectives )

MyCave grid(graybat::pattern::Grid(3,3));

BOOST_AUTO_TEST_CASE( reduce ){

    grid.distribute(graybat::mapping::Consecutive());

    const unsigned nElements = 10;
    
    std::vector<unsigned> send(nElements, 1);
    std::vector<unsigned> recv(nElements,0);

    Vertex rootVertex = grid.getVertices().at(0);
    
    for(Vertex v: grid.hostedVertices){
	grid.reduce(rootVertex, v, std::plus<unsigned>(), send, recv);
    }

    if(grid.peerHostsVertex(rootVertex)){
	for(unsigned u: recv){
	    BOOST_CHECK_EQUAL(u, grid.getVertices().size());
	}
    }
    
    
}

BOOST_AUTO_TEST_CASE( gather ){

    grid.distribute(graybat::mapping::Consecutive());

    const unsigned nElements = 10;
    const unsigned testValue = 1;
    
    std::vector<unsigned> send(nElements, testValue);
    std::vector<unsigned> recv(nElements * grid.getVertices().size(), 0);

    Vertex rootVertex = grid.getVertices().at(0);
    
    for(Vertex v: grid.hostedVertices){
	bool reorder = true;
	grid.gather(rootVertex, v, send, recv, reorder);
    }

    if(grid.peerHostsVertex(rootVertex)){
	for(unsigned u: recv){
	    BOOST_CHECK_EQUAL(u, testValue);
	}
    }
    
    
}

BOOST_AUTO_TEST_SUITE_END()
