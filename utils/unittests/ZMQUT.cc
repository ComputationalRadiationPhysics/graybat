// GrayBat
#include <communicationPolicy/ZMQ.hpp>


int main() {
    /***************************************************************************
     * Configuration
     ****************************************************************************/

    // CommunicationPolicy
    typedef graybat::communicationPolicy::ZMQ ZMQ;
    typedef typename ZMQ::Context Context;
    
    ZMQ zmq;

    Context context = zmq.getGlobalContext();

    std::array<unsigned, 1> data {{1}};
    std::array<unsigned, 1> recv {{0}};

    zmq.allReduce(context, std::plus<unsigned>(), data, recv);

    std::cout << recv[0] << std::endl;
    
    
    return 0;
}
