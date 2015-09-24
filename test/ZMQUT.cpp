// BOOST
#include <boost/test/unit_test.hpp>

// STL
#include <functional> /* std::plus */
#include <iostream>   /* std::cout, std::endl */
#include <array>      /* std::array */
#include <numeric>    /* std::iota */

// ZMQ
#include <zmq.hpp>

// GRAYBAT
#include <graybat/Cage.hpp>
#include <graybat/communicationPolicy/ZMQ.hpp>
#include <graybat/communicationPolicy/BMPI.hpp>
#include <graybat/graphPolicy/BGL.hpp>
#include <graybat/mapping/Random.hpp>
#include <graybat/mapping/Consecutive.hpp>
#include <graybat/mapping/Roundrobin.hpp>
#include <graybat/pattern/FullyConnected.hpp>
#include <graybat/pattern/InStar.hpp>
#include <graybat/pattern/Grid.hpp>


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

    ZMQ zmq;

    Context context = zmq.getGlobalContext();

    const unsigned nElements = 10;
    
    std::vector<unsigned> recv (nElements, 0);

    std::vector<Event> events;

    for(unsigned vAddr = 0; vAddr < context.size(); ++vAddr){
        std::vector<unsigned> data (nElements, 0);
        std::iota(data.begin(), data.end(), context.getVAddr());
        events.push_back(zmq.asyncSend(vAddr, 99, context, data));
        
    }

    for(unsigned vAddr = 0; vAddr < context.size(); ++vAddr){
        zmq.recv(vAddr, 99, context, recv);

        for(unsigned i = 0; i < recv.size(); ++i){
            BOOST_CHECK_EQUAL(recv[i], vAddr+i);
            
        }

    }


    for(Event &e : events){
        e.wait();
    }


}


BOOST_AUTO_TEST_CASE( send_recv_all ){
    typedef graybat::communicationPolicy::ZMQ ZMQ;
    typedef typename ZMQ::Context             Context;
    typedef typename ZMQ::Event               Event;

    ZMQ zmq;

    Context context = zmq.getGlobalContext();

    const unsigned nElements = 10;
    
    std::vector<unsigned> recv (nElements, 0);

    std::vector<Event> events;

    for(unsigned vAddr = 0; vAddr < context.size(); ++vAddr){
        std::vector<unsigned> data (nElements, 0);
        std::iota(data.begin(), data.end(), context.getVAddr());
        events.push_back(zmq.asyncSend(vAddr, 99, context, data));
        
    }

    for(unsigned i = 0; i < context.size(); ++i){
        Event e = zmq.recv(context, recv);

	unsigned vAddr = e.vAddr;

        for(unsigned i = 0; i < recv.size(); ++i){
            BOOST_CHECK_EQUAL(recv[i], vAddr+i);
            
        }

    }


    for(Event &e : events){
        e.wait();
    }


}

BOOST_AUTO_TEST_CASE( send_recv_order ){
    typedef graybat::communicationPolicy::ZMQ ZMQ;
    typedef typename ZMQ::Context             Context;
    typedef typename ZMQ::Event               Event;

    ZMQ zmq;

    Context context = zmq.getGlobalContext();

    const unsigned nElements = 10;

    std::vector<Event> events;

    std::vector<unsigned> recv1 (nElements, 0);
    std::vector<unsigned> recv2 (nElements, 0);
    std::vector<unsigned> recv3 (nElements, 0);


    for(unsigned vAddr = 0; vAddr < context.size(); ++vAddr){
        std::vector<unsigned> data1 (nElements, context.getVAddr());
        std::vector<unsigned> data2 (nElements, context.getVAddr() + 1);
        std::vector<unsigned> data3 (nElements, context.getVAddr() + 2);
        events.push_back( zmq.asyncSend(vAddr, 99, context, data1));
        events.push_back( zmq.asyncSend(vAddr, 99, context, data2));
	events.push_back( zmq.asyncSend(vAddr, 99, context, data3));
        
    }

    for(unsigned vAddr = 0; vAddr < context.size(); ++vAddr){
        zmq.recv(vAddr, 99, context, recv1);
        zmq.recv(vAddr, 99, context, recv2);
        zmq.recv(vAddr, 99, context, recv3);

        for(unsigned i = 0; i < recv1.size(); ++i){
            BOOST_CHECK_EQUAL(recv1[i], vAddr);
            
        }

        for(unsigned i = 0; i < recv1.size(); ++i){
            BOOST_CHECK_EQUAL(recv2[i], vAddr + 1);
            
        }

        for(unsigned i = 0; i < recv1.size(); ++i){
            BOOST_CHECK_EQUAL(recv3[i], vAddr + 2);
            
        }

    }

    for(Event &e : events){
        e.wait();
    }

}

// NOT PASSING YET !
// BOOST_AUTO_TEST_CASE( multi_cage ){
//     Cage cage1;
//     cage1.setGraph(graybat::pattern::FullyConnected(cage1.getPeers().size()));
//     cage1.distribute(graybat::mapping::Roundrobin());

//     Cage cage2;
//     cage2.setGraph(graybat::pattern::FullyConnected(cage2.getPeers().size()));
//     cage2.distribute(graybat::mapping::Roundrobin());

// }



BOOST_AUTO_TEST_CASE( cage ){
    Cage cage;
    cage.setGraph(graybat::pattern::FullyConnected(cage.getPeers().size()));
    cage.distribute(graybat::mapping::Roundrobin());

    const unsigned nElements = 1000;
    
    std::vector<Event> events; 
    std::vector<unsigned> send(nElements,0);
    std::vector<unsigned> recv(nElements,0);

    for(unsigned i = 0; i < send.size();++i){
        send.at(i) = i;
    }

    //Send state to neighbor cells
    for(Vertex &v : cage.hostedVertices){
        for(Edge edge : cage.getOutEdges(v)){
            cage.send(edge, send, events);

        }
    }

    //Recv state from neighbor cells
    for(Vertex &v : cage.hostedVertices){
        for(Edge edge : cage.getInEdges(v)){
            cage.recv(edge, recv);
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


    std::cout << "finished" << std::endl;
}

BOOST_AUTO_TEST_SUITE_END()
