/**
 * Copyright 2016 Erik Zenker
 *
 * This file is part of Graybat.
 *
 * Graybat is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Graybat is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with Graybat.
 * If not, see <http://www.gnu.org/licenses/>.
 */

// BOOST
#include <boost/test/unit_test.hpp>
#include <boost/hana/tuple.hpp>

// STL
#include <iostream>   /* std::cout, std::endl */

// GRAYBAT
#include <graybat/Cage.hpp>
#include <graybat/communicationPolicy/ZMQ.hpp>
#include <graybat/communicationPolicy/BMPI.hpp>

// HANA

#include <boost/hana/for_each.hpp>

/*******************************************************************************
 * Communication Policies to Test
 ******************************************************************************/
namespace hana = boost::hana;

size_t const nRuns = 1;

using ZMQ        = graybat::communicationPolicy::ZMQ;
using BMPI       = graybat::communicationPolicy::BMPI;
using ZMQConfig  = ZMQ::Config;
using BMPIConfig = BMPI::Config;

ZMQConfig zmqConfig = {"tcp://127.0.0.1:5000",
                       "tcp://127.0.0.1:5001",
                       static_cast<size_t>(std::stoi(std::getenv("OMPI_COMM_WORLD_SIZE"))),
                       "context_cp_test"};

BMPIConfig bmpiConfig;

ZMQ zmqCP(zmqConfig);
BMPI bmpiCP(bmpiConfig);

auto communicationPolicies = hana::make_tuple(std::ref(zmqCP),
                                              std::ref(bmpiCP) );


/*******************************************************************************
 * Point to Point Test Suites
 ******************************************************************************/
BOOST_AUTO_TEST_SUITE( graybat_cp_point_to_point )


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
              for (unsigned i = 0; i < nRuns; ++i) {
                // std::cout << "Run: " << i << std::endl;
                Context newContext = cp.splitContext(true, oldContext);
                oldContext = newContext;
              }
            }

	});

}


BOOST_AUTO_TEST_CASE( async_send_recv){
    hana::for_each(communicationPolicies, [](auto cpRef){
	    // Test setup
	    using CP      = typename decltype(cpRef)::type;
	    using Context = typename CP::Context;
            using Event = typename CP::Event;
            CP &cp = cpRef.get();

            // Test run
            {

              const unsigned nElements = 10;
              const unsigned tag = 99;

              Context context = cp.getGlobalContext();

              for (unsigned i = 0; i < nRuns; ++i) {
                std::vector<unsigned> recv(nElements, 0);
                std::vector<Event> events;

                for (unsigned vAddr = 0; vAddr < context.size(); ++vAddr) {
                  std::vector<unsigned> data(nElements, 0);
                  std::iota(data.begin(), data.end(), context.getVAddr());
                  events.push_back(cp.asyncSend(vAddr, tag, context, data));
                }

                for (unsigned vAddr = 0; vAddr < context.size(); ++vAddr) {
                  cp.recv(vAddr, tag, context, recv);

                  for (unsigned i = 0; i < recv.size(); ++i) {
                    BOOST_CHECK_EQUAL(recv[i], vAddr + i);
                  }
                }

                for (Event &e : events) {
                  e.wait();
                }
              }

        }

    });

    }

    BOOST_AUTO_TEST_CASE( async_send_async_recv){
        hana::for_each(communicationPolicies, [](auto cpRef){
            // Test setup
            using CP      = typename decltype(cpRef)::type;
            using Context = typename CP::Context;
            using Event   = typename CP::Event;
            CP& cp = cpRef.get();

            // Test run
            {

                const unsigned nElements = 2;
                const unsigned tag = 99;

                Context context = cp.getGlobalContext();

                for(unsigned i = 0; i < nRuns; ++i){
                    std::vector<unsigned> recv (nElements, 0);
                    std::vector<Event> events;

                    for(unsigned vAddr = 0; vAddr < context.size(); ++vAddr){
                        std::vector<unsigned> data (nElements, 1);
                        std::iota(data.begin(), data.end(), context.getVAddr());
                        events.push_back(cp.asyncSend(vAddr, tag, context, data));
                    }

                    for(unsigned vAddr = 0; vAddr < context.size(); ++vAddr){
                        Event e = cp.asyncRecv(vAddr, tag, context, recv);
                        e.wait();
                        for(unsigned i = 0; i < recv.size(); ++i){
                            BOOST_CHECK_EQUAL(recv[i], vAddr+i);
                        }
                    }

                    for(Event &e : events){
                        e.wait();
                    }
                }

            }

        });

    }




    BOOST_AUTO_TEST_CASE( send_recv_all){
    hana::for_each(communicationPolicies, [](auto cpRef){
	    // Test setup
	    using CP      = typename decltype(cpRef)::type;
	    using Context = typename CP::Context;
            using Event = typename CP::Event;
            CP &cp = cpRef.get();

            // Test run
            {

              Context context = cp.getGlobalContext();

              const unsigned nElements = 10;

              for (unsigned i = 0; i < nRuns; ++i) {
                std::vector<unsigned> recv(nElements, 0);
                std::vector<Event> events;

                for (unsigned vAddr = 0; vAddr < context.size(); ++vAddr) {
                  std::vector<unsigned> data(nElements, 0);
                  std::iota(data.begin(), data.end(), context.getVAddr());
                  events.push_back(cp.asyncSend(vAddr, 99, context, data));
                }

                for (unsigned i = 0; i < context.size(); ++i) {
                  Event e = cp.recv(context, recv);

                  unsigned vAddr = e.source();

                  for (unsigned i = 0; i < recv.size(); ++i) {
                    BOOST_CHECK_EQUAL(recv[i], vAddr + i);
                  }
                }

                for (Event &e : events) {
                  e.wait();
                }
              }
            }

	});

}


BOOST_AUTO_TEST_CASE( send_recv_order ){
    hana::for_each(communicationPolicies, [](auto cpRef){
	    // Test setup
	    using CP      = typename decltype(cpRef)::type;
	    using Context = typename CP::Context;
	    using Event   = typename CP::Event;
            CP &cp = cpRef.get();

            // Test run
            {

              Context context = cp.getGlobalContext();

              const unsigned nElements = 10;
              const unsigned tag = 99;

              for (unsigned run_i = 0; run_i < nRuns; ++run_i) {

                std::vector<Event> events;

                std::vector<unsigned> recv1(nElements, 0);
                std::vector<unsigned> recv2(nElements, 0);
                std::vector<unsigned> recv3(nElements, 0);

                std::vector<unsigned> data1(nElements, context.getVAddr());
                std::vector<unsigned> data2(nElements, context.getVAddr() + 1);
                std::vector<unsigned> data3(nElements, context.getVAddr() + 2);

                for (unsigned vAddr = 0; vAddr < context.size(); ++vAddr) {
                  events.push_back(cp.asyncSend(vAddr, tag, context, data1));
                  events.push_back(cp.asyncSend(vAddr, tag, context, data2));
                  events.push_back(cp.asyncSend(vAddr, tag, context, data3));
                }

                for (unsigned vAddr = 0; vAddr < context.size(); ++vAddr) {
                  cp.recv(vAddr, tag, context, recv1);
                  cp.recv(vAddr, tag, context, recv2);
                  cp.recv(vAddr, tag, context, recv3);

                  for (unsigned i = 0; i < recv1.size(); ++i) {
                    BOOST_CHECK_EQUAL(recv1[i], vAddr);
                  }

                  for (unsigned i = 0; i < recv1.size(); ++i) {
                    BOOST_CHECK_EQUAL(recv2[i], vAddr + 1);
                  }

                  for (unsigned i = 0; i < recv1.size(); ++i) {
                    BOOST_CHECK_EQUAL(recv3[i], vAddr + 2);
                  }
                }

                for (Event &e : events) {
                  e.wait();
                }
              }
            }

	});

}


BOOST_AUTO_TEST_SUITE_END()


/*******************************************************************************
 * Collective Test Suites
 ******************************************************************************/
BOOST_AUTO_TEST_SUITE( graybat_cp_collectives )


/*******************************************************************************
 * Test Cases
 ******************************************************************************/
BOOST_AUTO_TEST_CASE( gather ){
    hana::for_each(communicationPolicies, [](auto cpRef){
	    // Test setup
	    using CP      = typename decltype(cpRef)::type;
	    using Context = typename CP::Context;
	    using Event   = typename CP::Event;
            CP &cp = cpRef.get();

            // Test run
            {

                Context context = cp.getGlobalContext();

                const unsigned nElements = 10;
                unsigned value = 9;

                for(unsigned run_i = 0; run_i < nRuns; ++run_i){

                    std::vector<Event> events;

                    std::vector<unsigned> send (nElements, value);
                    std::vector<unsigned> recv (nElements * context.size(), 0);

                    cp.gather(0, context, send, recv);

                    if(context.getVAddr() == 0){
                        for(auto d : recv){
                            BOOST_CHECK_EQUAL(d, value);
                        }

                    }

                    for(Event &e : events){
                        e.wait();

                    }

                }

            }

        });

}

BOOST_AUTO_TEST_CASE( gather_var ){
    hana::for_each(communicationPolicies, [](auto cpRef){
	    // Test setup
	    using CP      = typename decltype(cpRef)::type;
	    using Context = typename CP::Context;
	    using Event   = typename CP::Event;
            using VAddr = typename CP::VAddr;
            CP &cp = cpRef.get();

            // Test run
            {

                Context context = cp.getGlobalContext();
                std::vector<unsigned> recvCount;

                for(unsigned run_i = 0; run_i < nRuns; ++run_i){

                    std::vector<Event> events;

                    std::vector<unsigned> send (context.getVAddr() + 1, context.getVAddr());
                    std::vector<unsigned> recv;

                    cp.gatherVar(0, context, send, recv, recvCount);

                    if(context.getVAddr() == 0){
                        unsigned i = 0;
                        for(VAddr vAddr = 0; vAddr < context.size(); ++vAddr){
                            for(unsigned j = 0; j < vAddr + 1; j++){
                                BOOST_CHECK_EQUAL(recv.at(i), vAddr);
                                i++;
                            }

                        }

                    }

                    for(Event &e : events){
                        e.wait();

                    }

                }

            }

        });

}

BOOST_AUTO_TEST_CASE( all_gather ){
    hana::for_each(communicationPolicies, [](auto cpRef){
	    // Test setup
	    using CP      = typename decltype(cpRef)::type;
	    using Context = typename CP::Context;
	    using Event   = typename CP::Event;
            CP &cp = cpRef.get();
            // Test run
            {

                Context context = cp.getGlobalContext();

                const unsigned nElements = 10;
                unsigned value = 9;

                for(unsigned run_i = 0; run_i < nRuns; ++run_i){

                    std::vector<Event> events;

                    std::vector<unsigned> send (nElements, value);
                    std::vector<unsigned> recv (nElements * context.size(), 0);

                    cp.allGather(context, send, recv);

                    for(auto d : recv){
                        BOOST_CHECK_EQUAL(d, value);
                    }


                    for(Event &e : events){
                        e.wait();

                    }

                }

            }

        });

}

BOOST_AUTO_TEST_CASE( all_gather_var ){
    hana::for_each(communicationPolicies, [](auto cpRef){
	    // Test setup
	    using CP      = typename decltype(cpRef)::type;
	    using Context = typename CP::Context;
	    using Event   = typename CP::Event;
            using VAddr = typename CP::VAddr;
            CP &cp = cpRef.get();
            // Test run
            {

                Context context = cp.getGlobalContext();
                std::vector<unsigned> recvCount;

                for(unsigned run_i = 0; run_i < nRuns; ++run_i){

                    std::vector<Event> events;

                    std::vector<unsigned> send (context.getVAddr() + 1, context.getVAddr());
                    std::vector<unsigned> recv;

                    cp.allGatherVar(context, send, recv, recvCount);

                    unsigned i = 0;
                    for(VAddr vAddr = 0; vAddr < context.size(); ++vAddr){
                        for(unsigned j = 0; j < vAddr + 1; j++){
                            BOOST_CHECK_EQUAL(recv.at(i), vAddr);
                            i++;
                        }

                    }


                    for(Event &e : events){
                        e.wait();

                    }

                }

            }

        });

}

BOOST_AUTO_TEST_CASE( scatter ){
    hana::for_each(communicationPolicies, [](auto cpRef){
	    // Test setup
	    using CP      = typename decltype(cpRef)::type;
	    using Context = typename CP::Context;
	    using Event   = typename CP::Event;
            CP &cp = cpRef.get();

            // Test run
            {

                Context context = cp.getGlobalContext();

                const unsigned nElements = 10;
                unsigned value = 9;

                for(unsigned run_i = 0; run_i < nRuns; ++run_i){

                    std::vector<Event> events;

                    std::vector<unsigned> send (nElements * context.size(), value);
                    std::vector<unsigned> recv (nElements, 0);

                    cp.scatter(0, context, send, recv);

                    if(context.getVAddr() == 0){
                        for(auto d : recv){
                            BOOST_CHECK_EQUAL(d, value);
                        }

                    }

                    for(Event &e : events){
                        e.wait();

                    }

                }

            }

        });

}

BOOST_AUTO_TEST_CASE( reduce ){
    hana::for_each(communicationPolicies, [](auto cpRef){
	    // Test setup
	    using CP      = typename decltype(cpRef)::type;
	    using Context = typename CP::Context;
	    using Event   = typename CP::Event;
            CP &cp = cpRef.get();

            // Test run
            {

                Context context = cp.getGlobalContext();

                const unsigned nElements = 10;
                unsigned value = 9;

                for(unsigned run_i = 0; run_i < nRuns; ++run_i){

                    std::vector<Event> events;

                    std::vector<unsigned> send (nElements, value);
                    std::vector<unsigned> recv (nElements, 0);

                    cp.reduce(0, context, std::plus<unsigned>(), send, recv);

                    if(context.getVAddr() == 0){
                        for(auto d : recv){
                            BOOST_CHECK_EQUAL(d, value * context.size());
                        }

                    }

                    for(Event &e : events){
                        e.wait();

                    }

                }

            }

        });

}

BOOST_AUTO_TEST_CASE( broadcast ){
    hana::for_each(communicationPolicies, [](auto cpRef){
	    // Test setup
	    using CP      = typename decltype(cpRef)::type;
	    using Context = typename CP::Context;
            using Event = typename CP::Event;
            CP &cp = cpRef.get();

            // Test run
            {

              Context context = cp.getGlobalContext();

              const unsigned nElements = 10;
              unsigned value = 9;

              for (unsigned run_i = 0; run_i < nRuns; ++run_i) {

                std::vector<Event> events;

                std::vector<unsigned> data(nElements, value);

                cp.broadcast(0, context, data);

                for (auto d : data) {
                  BOOST_CHECK_EQUAL(d, value);
                }

                for (Event &e : events) {
                  e.wait();
                }
            }
            }

        });

}

BOOST_AUTO_TEST_SUITE_END()
