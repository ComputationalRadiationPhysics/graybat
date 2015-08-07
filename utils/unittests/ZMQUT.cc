// boost 
#include <boost/test/unit_test.hpp>

// graybat
#include <Cage.hpp>
#include <communicationPolicy/ZMQ.hpp>
#include <communicationPolicy/BMPI.hpp>
#include <graphPolicy/BGL.hpp>
#include <mapping/Random.hpp>
#include <mapping/Consecutive.hpp>
#include <mapping/Roundrobin.hpp>
#include <pattern/FullyConnected.hpp>
#include <pattern/InStar.hpp>
#include <pattern/Grid.hpp>

// STL
#include <functional> /* std::plus */
#include <iostream>   /* std::cout, std::endl */
#include <array>      /* std::array */

// ZMQ
#include <zmq.hpp>


/***************************************************************************
 * Configuration
 ****************************************************************************/

// CommunicationPolicy
typedef graybat::communicationPolicy::ZMQ CP;
    
// GraphPolicy
typedef graybat::graphPolicy::BGL<>        GP;
    
// Cage
typedef graybat::Cage<CP, GP> Cage;
typedef typename Cage::Event  Event;
typedef typename Cage::Vertex Vertex;
typedef typename Cage::Edge   Edge;



BOOST_AUTO_TEST_SUITE( zmq )

BOOST_AUTO_TEST_CASE( send_recv ){
    typedef graybat::communicationPolicy::ZMQ ZMQ;
    typedef typename ZMQ::Context             Context;
    typedef typename ZMQ::Event               Event;

    BOOST_TEST_MESSAGE("Entry");
    
    ZMQ zmq;

    Context context = zmq.getGlobalContext();

    const unsigned nElements = 10;
    const unsigned testValue = 5;

    std::vector<unsigned> data (nElements, testValue);
    std::vector<unsigned> recv (nElements, 0);


    for(unsigned vAddr = 0; vAddr < context.size(); ++vAddr){
        Event e = zmq.asyncSend(vAddr, 99, context, data);
        e.wait();
    }

    for(unsigned vAddr = 0; vAddr < context.size(); ++vAddr){

        zmq.recv(vAddr, 99, context, recv);

        for(unsigned u : recv){
            BOOST_CHECK_EQUAL(u, testValue);
        }
        
    }

    std::cout << "Finished ZMQ test" << std::endl;

    while(true){}
}



BOOST_AUTO_TEST_CASE( cage ){
    Cage cage;
    cage.setGraph(graybat::pattern::FullyConnected(cage.getPeers().size()));
    cage.distribute(graybat::mapping::Roundrobin());

    // const unsigned nElements = 1000;
    
    // std::vector<Event> events; 
    // std::vector<unsigned> send(nElements,0);
    // std::vector<unsigned> recv(nElements,0);

    // for(unsigned i = 0; i < send.size();++i){
    //     send.at(i) = i;
    // }

    // std::cout << "Hosted vertices: " << cage.hostedVertices.size() << std::endl;

    // // Send state to neighbor cells
    // for(Vertex &v : cage.hostedVertices){
    //     for(Edge edge : cage.getOutEdges(v)){
    //         cage.send(edge, send, events);
    //     }
    // }

    // //Recv state from neighbor cells
    // for(Vertex &v : cage.hostedVertices){
    //     for(Edge edge : cage.getInEdges(v)){
    //         cage.recv(edge, recv);
    //         for(unsigned i = 0; i < recv.size();++i){
    //     	BOOST_CHECK_EQUAL(recv.at(i), i);
    //         }

    //     }
	
    // }
    
    // // Wait to finish events
    // for(unsigned i = 0; i < events.size(); ++i){
    //     events.back().wait();
    //     events.pop_back();
    // }

    // std::cout << "finished " << cage.hostedVertices.at(0).id << std::endl;

    // This while true is still important,
    // since the manager thread destructs when
    // its peer destructs.
    while(true){}
}

BOOST_AUTO_TEST_SUITE_END()
