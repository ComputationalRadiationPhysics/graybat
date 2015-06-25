// boost 
#include <boost/test/unit_test.hpp>

// graybat
#include <Cage.hpp>
#include <communicationPolicy/BMPI.hpp>
#include <graphPolicy/BGL.hpp>
#include <mapping/Consecutive.hpp>
#include <pattern/Grid.hpp>
#include <pattern/Chain.hpp>

// stl
#include <vector>
#include <functional> // std::plus


/***************************************************************************
 * Configuration
 ****************************************************************************/

// CommunicationPolicy
typedef graybat::communicationPolicy::BMPI CP;
    
// GraphPolicy
typedef graybat::graphPolicy::BGL<>        GP;
    
// Cage
typedef graybat::Cage<CP, GP> Cage;
typedef typename Cage::Event  Event;
typedef typename Cage::Vertex Vertex;
typedef typename Cage::Edge   Edge;


/***************************************************************************
 * Test Cases
 ****************************************************************************/

BOOST_AUTO_TEST_SUITE(edge)

Cage grid;

BOOST_AUTO_TEST_CASE( send_recv){
    std::vector<Event> events;


    grid.setGraph(graybat::pattern::Grid(grid.getPeers().size(), grid.getPeers().size()));
    grid.distribute(graybat::mapping::Consecutive());

    const unsigned nElements = 10;
    const unsigned testValue = 5;
    
    std::vector<unsigned> send(nElements, testValue);
    std::vector<unsigned> recv(nElements, 0);
    
    
    for(Vertex v : grid.hostedVertices){
        for(Edge edge : cage.getOutEdges(v)){
            Event e = edge << send;
            events.push_back(e);
        }
        
    }

    for(Vertex v : grid.hostedVertices){
        for(Edge edge : cage.getInEdges(v)){
            edge >> recv;
            for(unsigned u : recv){
                BOOST_CHECK_EQUAL(u, testValue);
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
