

#include <graybat/Cage.hpp>
#include <graybat/communicationPolicy/BMPI.hpp>
#include <graybat/communicationPolicy/ZMQ.hpp>
#include <graybat/graphPolicy/BGL.hpp>
#include <graybat/mapping/Roundrobin.hpp>
#include <graybat/pattern/FullyConnected.hpp>

#include <benchmark/benchmark.h>

#include <boost/hana/tuple.hpp>
#include <boost/hana/append.hpp>

#include <vector>

// compile: g++ main.cpp -lbenchmark




using GP = graybat::graphPolicy::BGL<>;

using BMPI = graybat::communicationPolicy::BMPI;
using BMPICage = graybat::Cage<BMPI, GP>;
using BMPIConfig = BMPI::Config;

using ZMQ = graybat::communicationPolicy::ZMQ;
using ZMQCage = graybat::Cage<ZMQ, GP>;
using ZMQConfig = ZMQ::Config;

ZMQConfig zmqConfig = {
    "tcp://127.0.0.1:5000", "tcp://127.0.0.1:5001",
    static_cast<size_t>(std::stoi(std::getenv("OMPI_COMM_WORLD_SIZE"))),
    "context_cage_test"};
ZMQCage zmqCage(zmqConfig);

BMPIConfig bmpiConfig;
BMPICage bmpiCage(bmpiConfig);


////////////////////////////////////////////////////////////////////////////////
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
////////////////////////////////////////////////////////////////////////////////
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


BENCHMARK(meassureSingleMessageSendBmpi)->RangeMultiplier(10)->Range(1, 1000000);
BENCHMARK(meassureSingleMessageSendZmq)->RangeMultiplier(10)->Range(1, 1000000);


BENCHMARK_MAIN()
