// boost 
#include <boost/test/unit_test.hpp>

// graybat
#include <communicationPolicy/ZMQ.hpp>

// STL
#include <functional> /* std::plus */
#include <iostream>   /* std::cout, std::endl */
#include <array>      /* std::array */

// ZMQ
#include <zmq.hpp>

BOOST_AUTO_TEST_SUITE( zmq )

BOOST_AUTO_TEST_CASE( send_recv ){
    typedef graybat::communicationPolicy::ZMQ ZMQ;
    typedef typename ZMQ::Context             Context;
    typedef typename ZMQ::Event               Event;

    BOOST_TEST_MESSAGE("Entry");
    
    ZMQ zmq;

    Context context = zmq.getGlobalContext();

    BOOST_TEST_MESSAGE("After zmq construction");


    const unsigned nElements = 10;
    const unsigned testValue = 5;

    std::vector<unsigned> data (nElements, testValue);
    std::vector<unsigned> recv (nElements, 0);


    if(context.getVAddr() == 0){
        Event e = zmq.asyncSend(1, 0, context, data);

        e.wait();
    }
    else {
        zmq.recv(0, 0, context, recv);

        for(unsigned u : recv){
            BOOST_CHECK_EQUAL(u, testValue);
        }
        
    }

    
}

BOOST_AUTO_TEST_SUITE_END()
