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

// STL
#include <array>
#include <vector>
#include <iostream>   /* std::cout, std::endl */
#include <functional> /* std::plus, std::ref */
#include <cstdlib>    /* std::getenv */
#include <string>     /* std::string, std::stoi */
#include <list>       /* std::list */

// BOOST
#include <boost/test/unit_test.hpp>
#include <boost/hana/tuple.hpp>
#include <boost/hana/append.hpp>

// GRAYBAT
#include <graybat/Cage.hpp>
#include <graybat/communicationPolicy/BMPI.hpp>
#include <graybat/communicationPolicy/ZMQ.hpp>
#include <graybat/graphPolicy/BGL.hpp>
#include <graybat/serializationPolicy/ByteCast.hpp>
#include <graybat/serializationPolicy/Forward.hpp>
#include <graybat/mapping/Random.hpp>
#include <graybat/mapping/Consecutive.hpp>
#include <graybat/mapping/Roundrobin.hpp>
#include <graybat/pattern/FullyConnected.hpp>
#include <graybat/pattern/InStar.hpp>
#include <graybat/pattern/Grid.hpp>

/*******************************************************************************
 * Test Suites
 ******************************************************************************/

/***************************************************************************
 * Test Cases
 ****************************************************************************/
BOOST_AUTO_TEST_SUITE(graybat_serialization_policy_tests)

/*******************************************************************************
 * Communication Policies to Test
 ******************************************************************************/
    namespace hana = boost::hana;
    size_t const nRuns = 1;

    using ZMQ = graybat::communicationPolicy::ZMQ;
    using BMPI = graybat::communicationPolicy::BMPI;
    using GP = graybat::graphPolicy::BGL<>;
    using Serialization = graybat::serializationPolicy::ByteCast;
    using ZMQCage = graybat::Cage<ZMQ, GP, Serialization>;
    using BMPICage = graybat::Cage<BMPI, GP, Serialization>;
    using ZMQConfig = ZMQ::Config;
    using BMPIConfig = BMPI::Config;

    ZMQConfig zmqConfig = {
        "tcp://127.0.0.1:5000", "tcp://127.0.0.1:5001",
        static_cast<size_t>(std::stoi(std::getenv("OMPI_COMM_WORLD_SIZE"))),
        "context_serialization_test"};

    BMPIConfig bmpiConfig;

    ZMQCage zmqCage(zmqConfig);
    BMPICage bmpiCage(bmpiConfig);

    auto cages = hana::make_tuple(std::ref(zmqCage), std::ref(bmpiCage));

    BOOST_AUTO_TEST_CASE(move_construct) {
        hana::for_each(cages, [](auto cageRef) {
            // Test run
            {
                auto &cage = cageRef.get();
                auto cage2 = std::move(cage);
                cage = std::move(cage2);
            }
        });
    }

    BOOST_AUTO_TEST_CASE(shouldSendCustomType) {
        hana::for_each(cages, [](auto cageRef) {
            // Test setup
            using Cage = typename decltype(cageRef)::type;
            using GP = typename Cage::GraphPolicy;
            using Event = typename Cage::Event;
            using Vertex = typename Cage::Vertex;
            using Edge = typename Cage::Edge;

            struct CustomType {
                uint8_t a;
                uint8_t b;
                std::array<uint64_t, 4096> c;
                uint64_t d;
            };

            CustomType ct;
            ct.a = 1;
            ct.b = 2;
            ct.d = 3;

            // Test run
            {
                auto &cage = cageRef.get();
                cage.setGraph(
                    graybat::pattern::FullyConnected<GP>(cage.getPeers().size()));
                cage.distribute(graybat::mapping::Roundrobin());

                std::vector<Event> events;
                std::vector<CustomType> send(1, ct);
                std::vector<CustomType> recv(1);

                for (unsigned run_i = 0; run_i < nRuns; ++run_i) {
                    // Send state to neighbor cells
                    for (Vertex &v : cage.getHostedVertices()) {
                        for (Edge edge : cage.getOutEdges(v)) {
                            cage.send(edge, send, events);
                        }
                    }

                    // Recv state from neighbor cells
                    for (Vertex &v : cage.getHostedVertices()) {
                        for (Edge edge : cage.getInEdges(v)) {
                            cage.recv(edge, recv);
                            BOOST_CHECK_EQUAL(recv.at(0).a, 1);
                            BOOST_CHECK_EQUAL(recv.at(0).b, 2);
                            BOOST_CHECK_EQUAL(recv.at(0).d, 3);
                        }
                    }
                }
            }

        });
    }
BOOST_AUTO_TEST_SUITE_END()