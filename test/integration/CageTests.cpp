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

BOOST_AUTO_TEST_SUITE(graybat_cage_tests)

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

BOOST_AUTO_TEST_CASE(send_recv)
{
    hana::for_each(cages, [](auto cage) {
        // Test setup
        using Cage = typename decltype(cage)::element_type;
        using GP = typename Cage::GraphPolicy;
        using Event = typename Cage::Event;
        using Vertex = typename Cage::Vertex;
        using Edge = typename Cage::Edge;

        // Test run
        {
            const unsigned nElements = 1000;

            cage->setGraph(graybat::pattern::FullyConnected<GP>(cage->getPeers().size()));
            cage->distribute(graybat::mapping::Roundrobin());

            for (unsigned run_i = 0; run_i < nRuns; ++run_i) {
                std::vector<Event> events;
                std::vector<unsigned> send(nElements, 0);
                std::vector<unsigned> recv(nElements, 0);

                for (unsigned i = 0; i < send.size(); ++i) {
                    send.at(i) = i;
                }

                // Send state to neighbor cells
                for (Vertex& v : cage->getHostedVertices()) {
                    for (Edge edge : cage->getOutEdges(v)) {
                        cage->send(edge, send, events);
                    }
                }

                // Recv state from neighbor cells
                for (Vertex& v : cage->getHostedVertices()) {
                    for (Edge edge : cage->getInEdges(v)) {
                        cage->recv(edge, recv);
                        for (unsigned i = 0; i < recv.size(); ++i) {
                            BOOST_CHECK_EQUAL(recv.at(i), i);
                        }
                    }
                }

                for (Event& e : events) {
                    e.wait();
                }
            }
        }

    });
}

BOOST_AUTO_TEST_CASE(asyncSend_recv)
{
    hana::for_each(cages, [](auto cage) {
        // Test setup
        using Cage = typename decltype(cage)::element_type;
        using GP = typename Cage::GraphPolicy;
        using Event = typename Cage::Event;
        using Vertex = typename Cage::Vertex;
        using Edge = typename Cage::Edge;

        // Test run
        {
            cage->setGraph(graybat::pattern::FullyConnected<GP>(cage->getPeers().size()));
            cage->distribute(graybat::mapping::Consecutive());

            const unsigned nElements = 1000;

            for (unsigned run_i = 0; run_i < nRuns; ++run_i) {
                std::vector<Event> events;
                std::vector<unsigned> send(nElements, 0);
                std::vector<unsigned> recv(nElements, 0);

                for (unsigned i = 0; i < send.size(); ++i) {
                    send.at(i) = i;
                }

                // Send state to neighbor cells
                for (Vertex& v : cage->getHostedVertices()) {
                    for (Edge edge : cage->getOutEdges(v)) {
                        cage->send(edge, send, events);
                    }
                }

                // Recv state from neighbor cells
                for (Vertex& v : cage->getHostedVertices()) {
                    for (Edge edge : cage->getInEdges(v)) {
                        cage->recv(edge, recv);
                        for (unsigned i = 0; i < recv.size(); ++i) {
                            BOOST_CHECK_EQUAL(recv.at(i), i);
                        }
                    }
                }

                // Wait to finish events
                for (unsigned i = 0; i < events.size(); ++i) {
                    events.back().wait();
                    events.pop_back();
                }
            }
        }

    });
}

BOOST_AUTO_TEST_CASE(shouldAsyncSendAndAsyncRecv)
{
    hana::for_each(cages, [](auto cage) {
        // Test setup
        using Cage = typename decltype(cage)::element_type;
        using GP = typename Cage::GraphPolicy;
        using Event = typename Cage::Event;
        using Vertex = typename Cage::Vertex;
        using Edge = typename Cage::Edge;

        // Test run
        {
            cage->setGraph(graybat::pattern::FullyConnected<GP>(cage->getPeers().size()));
            cage->distribute(graybat::mapping::Consecutive());

            const unsigned nElements = 10;

            for (unsigned run_i = 0; run_i < nRuns; ++run_i) {
                std::vector<Event> sendEvents;
                std::vector<Event> recvEvents;
                std::vector<unsigned> send(nElements, 0);
                std::list<std::vector<unsigned>> recv(0);

                for (unsigned i = 0; i < send.size(); ++i) {
                    send.at(i) = i;
                }

                // Send state to neighbor cells
                for (Vertex& v : cage->getHostedVertices()) {
                    for (Edge edge : cage->getOutEdges(v)) {
                        cage->send(edge, send, sendEvents);
                    }
                }

                // Recv state from neighbor cells
                for (Vertex& v : cage->getHostedVertices()) {
                    for (Edge edge : cage->getInEdges(v)) {
                        recv.push_back(std::vector<unsigned>(nElements, 0));
                        cage->recv(edge, recv.back(), recvEvents);
                    }
                }

                // Wait to finish send events
                for (unsigned i = 0; i < sendEvents.size(); ++i) {
                    sendEvents.back().wait();
                    sendEvents.pop_back();
                }

                // Wait to finish recv events
                while (true) {
                    if (recvEvents.empty())
                        break;
                    for (auto it = recvEvents.begin(); it != recvEvents.end();) {
                        if (it->ready()) {
                            it = recvEvents.erase(it);
                        } else {
                            it++;
                        }
                    }
                }

                for (unsigned i = 0; i < recvEvents.size(); ++i) {
                    recvEvents.back().wait();
                    recvEvents.pop_back();
                }

                // Check values of async received data
                for (auto v : recv) {
                    for (unsigned i = 0; i < v.size(); ++i) {
                        BOOST_CHECK_EQUAL(v.at(i), i);
                    }
                }
            }
        }
    });
}

BOOST_AUTO_TEST_CASE(shouldAsyncReceiveWithFuture)
{
    hana::for_each(cages, [](auto cage) {
        // Test setup
        using Cage = typename decltype(cage)::element_type;
        using GP = typename Cage::GraphPolicy;
        using Event = typename Cage::Event;
        using Vertex = typename Cage::Vertex;
        using Edge = typename Cage::Edge;

        // Test run
        {
            cage->setGraph(graybat::pattern::FullyConnected<GP>(cage->getPeers().size()));
            cage->distribute(graybat::mapping::Consecutive());

            const unsigned nElements = 1000000;

            std::chrono::milliseconds timeout(1000);
            std::vector<Event> sendEvents;
            std::vector<std::future<void>> recvFutures;
            std::vector<unsigned> send(nElements, 0);
            std::list<std::vector<unsigned>> recv(0);

            for (unsigned i = 0; i < send.size(); ++i) {
                send.at(i) = i;
            }

            // Send state to neighbor cells
            for (Vertex& v : cage->getHostedVertices()) {
                for (Edge edge : cage->getOutEdges(v)) {
                    cage->send(edge, send, sendEvents);
                }
            }

            // Recv state from neighbor cells
            for (Vertex& v : cage->getHostedVertices()) {
                for (Edge edge : cage->getInEdges(v)) {
                    recv.push_back(std::vector<unsigned>(nElements, 0));
                    recvFutures.emplace_back(cage->asyncRecv(edge, recv.back()));
                }
            }

            for (unsigned i = 0; i < recvFutures.size(); ++i) {
                BOOST_CHECK(recvFutures.back().wait_for(timeout) == std::future_status::ready);
            }

            // Check values of async received data
            for (auto v : recv) {
                for (unsigned i = 0; i < v.size(); ++i) {
                    BOOST_CHECK_EQUAL(v.at(i), i);
                }
            }
        }
    });
}

BOOST_AUTO_TEST_CASE(reduce)
{
    hana::for_each(cages, [](auto cage) {
        // Test setup
        using Cage = typename decltype(cage)::element_type;
        using GP = typename Cage::GraphPolicy;
        using Vertex = typename Cage::Vertex;

        // Test run
        {
            cage->setGraph(graybat::pattern::Grid<GP>(3, 3));
            cage->distribute(graybat::mapping::Consecutive());

            const unsigned nElements = 10;

            std::vector<unsigned> send(nElements, 1);
            std::vector<unsigned> recv(nElements, 0);

            Vertex rootVertex = cage->getVertex(0);

            for (Vertex v : cage->getHostedVertices()) {
                cage->reduce(rootVertex, v, std::plus<unsigned>(), send, recv);
            }

            if (cage->isHosting(rootVertex)) {
                for (unsigned receivedElement : recv) {
                    BOOST_CHECK_EQUAL(receivedElement, cage->getVertices().size());
                }
            }
        }
    });
}

BOOST_AUTO_TEST_CASE(allReduce)
{
    hana::for_each(cages, [](auto cage) {
        // Test setup
        using Cage = typename decltype(cage)::element_type;
        using GP = typename Cage::GraphPolicy;
        using Vertex = typename Cage::Vertex;

        // Test run
        {
            cage->setGraph(
                graybat::pattern::Grid<GP>(cage->getPeers().size(), cage->getPeers().size()));
            cage->distribute(graybat::mapping::Consecutive());

            const unsigned nElements = 10;

            std::vector<unsigned> send(nElements, 1);
            std::vector<unsigned> recv(nElements, 0);

            for (Vertex v : cage->getHostedVertices()) {
                cage->allReduce(v, std::plus<unsigned>(), send, recv);
            }

            for (unsigned receivedElement : recv) {
                BOOST_CHECK_EQUAL(receivedElement, cage->getVertices().size());
            }
        }

    });
}

BOOST_AUTO_TEST_CASE(gather)
{
    hana::for_each(cages, [](auto cage) {
        // Test setup
        using Cage = typename decltype(cage)::element_type;
        using GP = typename Cage::GraphPolicy;
        using Vertex = typename Cage::Vertex;

        // Test run
        {
            cage->setGraph(graybat::pattern::Grid<GP>(3, 3));
            cage->distribute(graybat::mapping::Consecutive());

            const unsigned nElements = 10;
            const unsigned testValue = 1;
            const bool reorder = true;

            std::vector<unsigned> send(nElements, testValue);
            std::vector<unsigned> recv(nElements * cage->getVertices().size(), 0);

            Vertex rootVertex = cage->getVertex(0);

            for (Vertex v : cage->getHostedVertices()) {
                cage->gather(rootVertex, v, send, recv, reorder);
            }

            if (cage->isHosting(rootVertex)) {
                for (unsigned receivedElement : recv) {
                    BOOST_CHECK_EQUAL(receivedElement, testValue);
                }
            }
        }
    });
}

BOOST_AUTO_TEST_CASE(allGather)
{
    hana::for_each(cages, [](auto cage) {
        // Test setup
        using Cage = typename decltype(cage)::element_type;
        using GP = typename Cage::GraphPolicy;
        using Vertex = typename Cage::Vertex;

        // Test run
        {
            cage->setGraph(
                graybat::pattern::Grid<GP>(cage->getPeers().size(), cage->getPeers().size()));
            cage->distribute(graybat::mapping::Consecutive());

            const unsigned nElements = 10;
            const unsigned testValue = 1;
            const bool reorder = true;

            std::vector<unsigned> send(nElements, testValue);
            std::vector<unsigned> recv(nElements * cage->getVertices().size(), 0);

            for (Vertex v : cage->getHostedVertices()) {
                cage->allGather(v, send, recv, reorder);
            }

            for (unsigned receivedElement : recv) {
                BOOST_CHECK_EQUAL(receivedElement, testValue);
            }
        }
    });
}

BOOST_AUTO_TEST_CASE(spreadAndCollect)
{
    hana::for_each(cages, [](auto cage) {
        // Test setup
        using Cage = typename decltype(cage)::element_type;
        using GP = typename Cage::GraphPolicy;
        using Event = typename Cage::Event;
        using Vertex = typename Cage::Vertex;

        // Test run
        {
            cage->setGraph(graybat::pattern::InStar<GP>(cage->getPeers().size()));
            cage->distribute(graybat::mapping::Consecutive());

            const unsigned nElements = 10;
            const unsigned testValue = 1;
            std::vector<Event> events;

            std::vector<unsigned> send(nElements, testValue);

            for (Vertex v : cage->getHostedVertices()) {
                v.spread(send, events);
            }

            for (Vertex v : cage->getHostedVertices()) {
                std::vector<unsigned> recv(v.nInEdges() * nElements, 0);
                v.collect(recv);
                for (unsigned receivedElement : recv) {
                    BOOST_CHECK_EQUAL(receivedElement, testValue);
                }
            }

            for (unsigned i = 0; i < events.size(); ++i) {
                events.back().wait();
                events.pop_back();
            }
        }
    });
}

BOOST_AUTO_TEST_SUITE_END()
