// BOOST
#include <boost/test/unit_test.hpp>
#include <boost/hana/tuple.hpp>

// STL
#include <iostream>   /* std::cout, std::endl */


// GRAYBAT
#include <graybat/Cage.hpp>
#include <graybat/communicationPolicy/ZMQ.hpp>

/*******************************************************************************
 * Point to Point Test Suites
 ******************************************************************************/
BOOST_AUTO_TEST_SUITE( graybat_update )

    namespace hana = boost::hana;

    size_t const nRuns = 1000;

    using ZMQ        = graybat::communicationPolicy::ZMQ;
    using ZMQConfig  = ZMQ::Config;

    ZMQConfig zmqConfig = {"tcp://127.0.0.1:5000",
                           "tcp://127.0.0.1:5001",
                           static_cast<size_t>(std::stoi(std::getenv("OMPI_COMM_WORLD_SIZE")))};

    ZMQ zmqCP(zmqConfig);

    auto communicationPolicies = hana::make_tuple(std::ref(zmqCP));



/***************************************************************************
 * Test Cases
 ****************************************************************************/
BOOST_AUTO_TEST_CASE( construct ){
    hana::for_each(communicationPolicies, [](auto cpRef){
            (void) cpRef;
        });

}


BOOST_AUTO_TEST_CASE( context ){
    hana::for_each(communicationPolicies, [](auto cpRef){
	    // Test setup
	    using CP      = typename decltype(cpRef)::type;
	    using Context = typename CP::Context;	    
	    CP& cp = cpRef.get();

	    // Test run
	    {	
            Context oldContext = cp.getGlobalContext();

            while(true){
                Context newContext(cp.updateContext(oldContext));
                if(newContext.size() > oldContext.size()){
                    std::cout << "NewContext size: " << newContext.size() << std::endl;
                    oldContext = newContext;
                }

            };



//		for(unsigned i = 0; i < nRuns; ++i){
//                    //std::cout << "Run: " << i << std::endl;
//                    Context newContext = cp.splitContext( true, oldContext);
//                    oldContext = newContext;
//
//		}
		
	    }

	});
    
}


//BOOST_AUTO_TEST_CASE( async_send_recv){
//    hana::for_each(communicationPolicies, [](auto cpRef){
//	    // Test setup
//	    using CP      = typename decltype(cpRef)::type;
//	    using Context = typename CP::Context;
//	    using Event   = typename CP::Event;
//	    CP& cp = cpRef.get();
//
//	    // Test run
//	    {
//
//		const unsigned nElements = 10;
//		const unsigned tag = 99;
//
//		Context context = cp.getGlobalContext();
//
//		for(unsigned i = 0; i < nRuns; ++i){
//		    std::vector<unsigned> recv (nElements, 0);
//		    std::vector<Event> events;
//
//		    for(unsigned vAddr = 0; vAddr < context.size(); ++vAddr){
//			std::vector<unsigned> data (nElements, 0);
//			std::iota(data.begin(), data.end(), context.getVAddr());
//			events.push_back(cp.asyncSend(vAddr, tag, context, data));
//
//		    }
//
//		    for(unsigned vAddr = 0; vAddr < context.size(); ++vAddr){
//			cp.recv(vAddr, tag, context, recv);
//
//			for(unsigned i = 0; i < recv.size(); ++i){
//			    BOOST_CHECK_EQUAL(recv[i], vAddr+i);
//
//			}
//
//		    }
//
//		    for(Event &e : events){
//			e.wait();
//
//		    }
//        }
//
//        }
//
//    });
//
//    }
BOOST_AUTO_TEST_SUITE_END()
