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
#include <cstdlib> /* std::getenv */
#include <functional> /* std::plus, std::ref */
#include <iostream> /* std::cout, std::endl */
#include <list> /* std::list */
#include <string> /* std::string, std::stoi */
#include <vector>

// BOOST
#include <boost/hana/concat.hpp>
#include <boost/hana/for_each.hpp>
#include <boost/hana/tuple.hpp>
#include <boost/test/unit_test.hpp>

// GRAYBAT
#include <graybat/graybat.hpp>

// Test Utils
#include "../utils.hpp"

namespace hana = boost::hana;

namespace {
size_t const nRuns = 1;
auto cages = graybat::test::utils::getCages();
}

BOOST_AUTO_TEST_SUITE(graybat_serialization_policy_tests)

BOOST_AUTO_TEST_CASE(move_construct)
{
    hana::for_each(cages, [](auto cage) {
        // Test run
        {
            auto cage2 = std::move(cage);
            cage = std::move(cage2);
        }
    });
}

BOOST_AUTO_TEST_CASE(shouldSendCustomType)
{
    hana::for_each(cages, [](auto cageRef) {
        // Test setup
        using Cage = typename decltype(cageRef)::element_type;
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
            auto& cage = *cageRef;
            cage.setGraph(graybat::pattern::FullyConnected<GP>(cage.getPeers().size()));
            cage.distribute(graybat::mapping::Roundrobin());

            std::vector<Event> events;
            std::vector<CustomType> send(1, ct);
            std::vector<CustomType> recv(1);

            for (unsigned run_i = 0; run_i < nRuns; ++run_i) {
                // Send state to neighbor cells
                for (Vertex& v : cage.getHostedVertices()) {
                    for (Edge edge : cage.getOutEdges(v)) {
                        cage.send(edge, send, events);
                    }
                }

                // Recv state from neighbor cells
                for (Vertex& v : cage.getHostedVertices()) {
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