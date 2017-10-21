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

// Stl
#include <vector>

// Google benchmark
#include <benchmark/benchmark.h>

// Graybat
#include <graybat/graybat.hpp>

using GP = graybat::graphPolicy::BGL<>;

#ifdef graybat_BMPI_CP_ENABLED
using BMPI = graybat::communicationPolicy::BMPI;
using BMPICage = graybat::Cage<BMPI, GP>;
using BMPIConfig = BMPI::Config;
BMPIConfig bmpiConfig;
BMPICage bmpiCage(bmpiConfig);
#endif

#ifdef graybat_ZMQ_CP_ENABLED
using ZMQ = graybat::communicationPolicy::ZMQ;
using ZMQCage = graybat::Cage<ZMQ, GP>;
using ZMQConfig = ZMQ::Config;

ZMQConfig zmqConfig = {
    "localhost:5000", "tcp://127.0.0.1:5001",
    static_cast<size_t>(std::stoi(std::getenv("OMPI_COMM_WORLD_SIZE"))),
    "context_cage_test"};
ZMQCage zmqCage(zmqConfig);
#endif



////////////////////////////////////////////////////////////////////////////////
#ifdef graybat_BMPI_CP_ENABLED
static void meassureSingleMessageSendBmpi(benchmark::State &state) {
  while (state.KeepRunning()) {

    auto &cage = bmpiCage;

    using Cage = decltype(bmpiCage);
    using GP = typename Cage::GraphPolicy;
    using Event = typename Cage::Event;
    using Vertex = typename Cage::Vertex;
    using Edge = typename Cage::Edge;

    cage.setGraph(graybat::pattern::FullyConnected<GP>(cage.getPeers().size()));
    cage.distribute(graybat::mapping::Roundrobin());

    const unsigned nElements = state.range(0);

    std::vector<Event> events;
    std::vector<unsigned> send(nElements, 0);
    std::vector<unsigned> recv(nElements, 0);

    for (unsigned i = 0; i < send.size(); ++i) {
      send.at(i) = i;
    }

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
      }
    }

    for (Event &e : events) {
      e.wait();
    }
  }
}

BENCHMARK(meassureSingleMessageSendBmpi)->RangeMultiplier(10)->Range(1, 1000000);
#endif

////////////////////////////////////////////////////////////////////////////////
#ifdef graybat_ZMQ_CP_ENABLED
static void meassureSingleMessageSendZmq(benchmark::State &state) {
    while (state.KeepRunning()) {

        auto &cage = zmqCage;

        using Cage = decltype(zmqCage);
        using GP = typename Cage::GraphPolicy;
        using Event = typename Cage::Event;
        using Vertex = typename Cage::Vertex;
        using Edge = typename Cage::Edge;

        cage.setGraph(graybat::pattern::FullyConnected<GP>(cage.getPeers().size()));
        cage.distribute(graybat::mapping::Roundrobin());

        const unsigned nElements = state.range(0);

        std::vector<Event> events;
        std::vector<unsigned> send(nElements, 0);
        std::vector<unsigned> recv(nElements, 0);

        for (unsigned i = 0; i < send.size(); ++i) {
            send.at(i) = i;
        }

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
            }
        }

        for (Event &e : events) {
            e.wait();
        }
    }
}

static void meassureAsyncReceiveFutureZmq(benchmark::State& state)
{
    while (state.KeepRunning()) {

        auto& cage = zmqCage;

        using Cage = decltype(zmqCage);
        using GP = typename Cage::GraphPolicy;
        using Event = typename Cage::Event;
        using Vertex = typename Cage::Vertex;
        using Edge = typename Cage::Edge;

        cage.setGraph(graybat::pattern::FullyConnected<GP>(cage.getPeers().size()));
        cage.distribute(graybat::mapping::Consecutive());

        const unsigned nElements = state.range(0);

        std::vector<Event> sendEvents;
        std::vector<std::future<void>> recvFutures;
        std::vector<unsigned> send(nElements, 0);
        std::list<std::vector<unsigned>> recv(0);

        for (unsigned i = 0; i < send.size(); ++i) {
            send.at(i) = i;
        }

        // Send state to neighbor cells
        for (Vertex& v : cage.getHostedVertices()) {
            for (Edge edge : cage.getOutEdges(v)) {
                cage.send(edge, send, sendEvents);
            }
        }

        // Recv state from neighbor cells
        for (Vertex& v : cage.getHostedVertices()) {
            for (Edge edge : cage.getInEdges(v)) {
                recv.push_back(std::vector<unsigned>(nElements, 0));
                recvFutures.emplace_back(cage.asyncRecv(edge, recv.back()));
            }
        }

        for (unsigned i = 0; i < recvFutures.size(); ++i) {
            recvFutures.back().wait();
        }

    }
}

static void meassureAsyncReceiveEventZmq(benchmark::State& state)
{
    while (state.KeepRunning()) {

        auto& cage = zmqCage;

        using Cage = decltype(zmqCage);
        using GP = typename Cage::GraphPolicy;
        using Event = typename Cage::Event;
        using Vertex = typename Cage::Vertex;
        using Edge = typename Cage::Edge;

        cage.setGraph(graybat::pattern::FullyConnected<GP>(cage.getPeers().size()));
        cage.distribute(graybat::mapping::Consecutive());

        const unsigned nElements = state.range(0);

        std::vector<Event> sendEvents;
        std::vector<Event> recvEvents;
        std::vector<unsigned> send(nElements, 0);
        std::list<std::vector<unsigned>> recv(0);

        for (unsigned i = 0; i < send.size(); ++i) {
            send.at(i) = i;
        }

        // Send state to neighbor cells
        for (Vertex& v : cage.getHostedVertices()) {
            for (Edge edge : cage.getOutEdges(v)) {
                cage.send(edge, send, sendEvents);
            }
        }

        // Recv state from neighbor cells
        for (Vertex& v : cage.getHostedVertices()) {
            for (Edge edge : cage.getInEdges(v)) {
                recv.push_back(std::vector<unsigned>(nElements, 0));
                cage.recv(edge, recv.back(), recvEvents);
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
    }
}


BENCHMARK(meassureSingleMessageSendZmq)->RangeMultiplier(10)->Range(1, 1000000);
BENCHMARK(meassureAsyncReceiveEventZmq)->RangeMultiplier(10)->Range(1, 1000000);
BENCHMARK(meassureAsyncReceiveFutureZmq)->RangeMultiplier(10)->Range(1, 1000000);
#endif

BENCHMARK_MAIN()
