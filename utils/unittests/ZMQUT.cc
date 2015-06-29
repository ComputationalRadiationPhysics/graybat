// boost 
#include <boost/test/unit_test.hpp>

// graybat
#include <communicationPolicy/ZMQ.hpp>

// STL
#include <functional> /* std::plus */
#include <iostream>   /* std::cout, std::endl */
#include <array>      /* std::array */


BOOST_AUTO_TEST_SUITE( zmq )

BOOST_AUTO_TEST_CASE( allreduce ){
    typedef graybat::communicationPolicy::ZMQ ZMQ;
    typedef typename ZMQ::Context Context;
    
    ZMQ zmq;

    Context context = zmq.getGlobalContext();

    std::array<unsigned, 1> data {{1}};
    std::array<unsigned, 1> recv {{0}};

    zmq.allReduce(context, std::plus<unsigned>(), data, recv);

    std::cout << recv[0] << std::endl;
    
}

BOOST_AUTO_TEST_SUITE_END()
