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
#include <boost/hana/concat.hpp>
#include <boost/hana/for_each.hpp>
#include <boost/hana/tuple.hpp>
#include <boost/test/unit_test.hpp>

// STL
#include <functional> // std::plus
#include <vector>

// GRAYBAT
#include <graybat/graybat.hpp>

// Test Utils
#include "../utils.hpp"

/***************************************************************************
 * Test Suites
 ****************************************************************************/
namespace hana = boost::hana;

namespace {
auto cages = graybat::test::utils::getCages();
}

BOOST_AUTO_TEST_SUITE(edge)

/***************************************************************************
 * Test Cases
 ****************************************************************************/

BOOST_AUTO_TEST_CASE(send_recv)
{
    hana::for_each(cages, [](auto cageRef) {
        // Test setup
        using Cage = typename decltype(cageRef)::element_type;
        using GP = typename Cage::GraphPolicy;
        using Event = typename Cage::Event;
        using Vertex = typename Cage::Vertex;
        using Edge = typename Cage::Edge;

        // Test run
        {

            std::vector<Event> events;
            auto& grid = *cageRef;

            grid.setGraph(
                graybat::pattern::Grid<GP>(grid.getPeers().size(), grid.getPeers().size()));

            grid.distribute(graybat::mapping::Consecutive());

            const unsigned nElements = 10;
            const unsigned testValue = 5;

            std::vector<unsigned> send(nElements, testValue);
            std::vector<unsigned> recv(nElements, 0);

            for (Vertex v : grid.getHostedVertices()) {
                for (Edge edge : grid.getOutEdges(v)) {
                    Event e = edge << send;
                    events.push_back(e);
                }
            }

            for (Vertex v : grid.getHostedVertices()) {
                for (Edge edge : grid.getInEdges(v)) {
                    edge >> recv;
                    for (unsigned u : recv) {
                        BOOST_CHECK_EQUAL(u, testValue);
                    }
                }
            }

            // Wait to finish events
            for (unsigned i = 0; i < events.size(); ++i) {
                events.back().wait();
                events.pop_back();
            }
        }

    });
}

BOOST_AUTO_TEST_SUITE_END()
