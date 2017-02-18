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

#pragma once

// CLIB
#include <assert.h> /* assert */
#include <cstddef> /* nullptr_t */

// STL
#include <algorithm>
#include <cstring>
#include <exception>
#include <map>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <tuple>

// Boost
#include <boost/core/ignore_unused.hpp>

// GRAYBAT
#include <graybat/Concept.hpp>
#include <graybat/Edge.hpp>
#include <graybat/EventWrapper.hpp>
#include <graybat/Vertex.hpp>
#include <graybat/communicationPolicy/Traits.hpp>
#include <graybat/graphPolicy/Traits.hpp>
#include <graybat/pattern/None.hpp>
#include <graybat/serializationPolicy/Forward.hpp>
#include <graybat/utils/exclusivePrefixSum.hpp>

namespace graybat {

/************************************************************************/ /**
  * @class Cage
  *
  * @brief The Communication And Graph Environment enables to communicate
  *        on basis of a graph with methods provided by a communication
  *        policy.
  *
  * A cage is defined by its Communication and Graph policy. The communication
  * policy provides methods for point to point and collective operations.
  * The graph policy provides methods to query graph imformation of the
  * cage graph.
  *
  * @remark A peer can host several vertices.
  *
  *
  ***************************************************************************/
/// requires concept::SerializationPolicy<T_SerializationPolicy>
template <
    typename T_CommunicationPolicy,
    typename T_GraphPolicy,
    typename T_SerializationPolicy = graybat::serializationPolicy::Forward>
struct Cage {

    using CommunicationPolicy = T_CommunicationPolicy;
    using GraphPolicy = T_GraphPolicy;
    using SerializationPolicy = T_SerializationPolicy;
    using Type = Cage<CommunicationPolicy, GraphPolicy, SerializationPolicy>;

    using VAddr = graybat::communicationPolicy::VAddr<CommunicationPolicy>;
    using Context = graybat::communicationPolicy::Context<CommunicationPolicy>;
    using CPEvent = graybat::communicationPolicy::Event<CommunicationPolicy>;
    using CPConfig = graybat::communicationPolicy::Config<CommunicationPolicy>;
    using ContextID = graybat::communicationPolicy::ContextID<CommunicationPolicy>;
    using Event = graybat::EventWrapper<CPEvent>;
    using Edge = graybat::CommunicationEdge<Cage>;
    using Vertex = graybat::CommunicationVertex<Cage>;
    using EdgeDescription = graybat::graphPolicy::EdgeDescription<GraphPolicy>;
    using GraphDescription = graybat::graphPolicy::GraphDescription<GraphPolicy>;
    using VertexID = graybat::graphPolicy::VertexID;
    using EdgeID = graybat::graphPolicy::EdgeID;
    using GraphID = graybat::graphPolicy::GraphID;
    using Peer = size_t;

    template <class T_Functor>
    /// requires GraphFactoryFunctor<T_Functor>
    Cage(CPConfig const cpConfig, T_Functor graphFunctor)
        : communicator(new CommunicationPolicy(cpConfig))
        , graph(GraphPolicy(graphFunctor()))
    {
    }

    Cage(CPConfig const cpConfig)
        : communicator(new CommunicationPolicy(cpConfig))
        , graph(GraphPolicy(graybat::pattern::None<GraphPolicy>()()))
    {
    }

    Cage()
        : communicator(new CommunicationPolicy(CPConfig()))
        , graph(GraphPolicy(graybat::pattern::None<GraphPolicy>()()))
    {
    }

    // Copy constructor
    Cage(Cage&) = delete;

    // Copy assignment constructor
    Cage& operator=(Cage&) = delete;

    // Move constructor
    Cage(Cage&&) = default;

    // Move assignment constructor
    Cage& operator=(Cage&&) = default;

    // Destructor
    ~Cage()
    { /* std::cout << "Destruct Cage" << std::endl */
    }

    /***********************************************************************/ /**
      *
      * @name Graph Operations
      *
      * @{
      *
      ***************************************************************************/
    /// requires GraphFactoryFunctor<T_Functor>
    template <typename T_Functor> void setGraph(T_Functor graphFunctor);

    auto getVertices() -> std::vector<Vertex>;

    auto getVertex(const VertexID& vertexID) -> Vertex;

    auto getEdge(const Vertex& source, const Vertex& target) -> Edge;

    auto getAdjacentVertices(const Vertex& v) -> std::vector<Vertex>;

    auto getOutEdges(const Vertex& v) -> std::vector<Edge>;

    auto getInEdges(const Vertex& v) -> std::vector<Edge>;
    /** @} */

    /***********************************************************************/ /**
      *
      * @name Mapping Operations
      *
      * @{
      *
      ***************************************************************************/

    /**
     * @brief Distribution of the graph vertices to the peers of
     *        the global context. The distFunctor it the function
     *        responsible for this distribution.
     *
     * @param distFunctor Function for vertex distribution
     *                    with the following interface:
     *                    distFunctor(OwnVAddr, ContextSize, Graph)
     *
     */
    // requires concept::DistributionFunctor<T_Functor>
    template <class T_Functor> auto distribute(T_Functor distFunctor) -> void;

    /**
     * @brief Announces *vertices* of a *graph* to the network, so that other
     * peers
     *        know that these *vertices* are hosted by this peer.
     *
     * The general workflow includes two steps:
     *  1. Each peer, that hosts vertices of the *graph* announces its *vertices*
     *     * Each peer will send its hosted vertices and update its vertices
     * location
     *     * The host peers will create a new context for *graph*
     *  2. Vertices can now be located by locateVertex()
     *  3. use Graphpeer to communicate between vertices
     *
     * @remark This is a collective Operation on which either all host peers
     *         of the supergraph of *graph* have to take part or when *graph* has
     * no
     *         supergraph then all Communicatos from the globalContext (which
     * should
     *         be all peers in the network).
     *
     * @todo What happens if *vertices* is empty ?
     * @todo What happens when there already exist an context for *graph* ?
     * @todo What happens when not all *vertices* of a *graph* were announced ?
     *
     * @param[in] graph  Its vertices will be announced
     * @param[in] vertices A set of vertices, that will be hosted by this peer
     *
     */
    auto announce(const std::vector<Vertex>& vertices, const bool global = true) -> void;

    /**
     * @brief Returns the VAddr of the host of *vertex* in the graph
     *
     * @bug When the location of *vertex* is not known then
     *      the programm crashes by an exception.
     *      This exception should be handled for better
     *      debugging behaviour.
     *
     * @param[in] vertex Will be located.
     *
     */
    auto locateVertex(const Vertex& vertex) -> VAddr;

    /**
     * @brief Opposite operation of locateVertex(). It returns the
     *        vertices that are hosted by the peer with
     *        *vAddr*
     */
    auto getVerticesHostedBy(const VAddr& vAddr) -> std::vector<Vertex>;

    /**
     * @brief Return the vertices that are hosted by this cage.
     *        The cage needs to announce vertices first, otherwise
     *        there won't be hosted vertices.
     *
     * @return vertices hosted by this cage
     */
    auto getHostedVertices() -> std::vector<Vertex>&;

    /**
     * @brief Returns true if the *vertex* is hosted by the
     *        calling peer otherwise false.
     *
     */
    auto isHosting(const Vertex& vertex) -> bool;

    /**
     * @brief Returns a vector with as much elements as there are peers in the
     * current context
     *
     */
    auto getPeers() -> std::vector<Peer>;

    /**
     * @brief Returns a shared pointer to the inner communication policy object,
     *        which allows to use communication methods without the need to
     *        announce hosted vertices. This can be useful for mappings that
     *        need communication in advance.
     *
     * @return shared pointer to the communication policy object
     */
    auto getCommunicationPolicy() -> std::shared_ptr<CommunicationPolicy>;

    /** @} */

    /***********************************************************************/ /**
       *
       * @name Point to Point Communication Operations
       *
       * @{
       *
       ***************************************************************************/

    /**
     * @brief Synchron transmission of *data* to the *destVertex* on *edge*.
     *

     *
     * @param[in] graph The graph in which the communication takes place.
     * @param[in] edge Edge over which the *data* will be transmitted.
     * @param[in] data Data that will be send.
     */
    template <typename T>
    /// requires concept::ContiguousContainer<T>
    void send(const Edge& edge, const T& data);

    /**
     * @brief Asynchron transmission of *data* to the *destVertex* on *edge*.
     *

     *
     * @param[in] graph The graph in which the communication takes place.
     * @param[in] edge Edge over which the *data* will be transmitted.
     * @param[in] data Data that will be send.
     * @param[out] List of events the send event will be added to.
     *
     */
    template <typename T>
    /// requires concept::ContiguousContainer<T>
    void send(const Edge& edge, const T& data, std::vector<Event>& events);

    /**
     * @brief Synchron receive of *data* from the *srcVertex* on *edge*.
     *
     * @param[in]  graph The graph in which the communication takes place.
     * @param[in]  edge Edge over which the *data* will be transmitted.
     * @param[out] data Data that will be received
     *
     */
    template <typename T>
    /// requires concept::ContiguousContainer<T>
    void recv(const Edge& edge, T& data);

    template <typename T>
    /// requires concept::ContiguousContainer<T>
    Edge recv(T& data);

    /**
     * @brief Asynchron receive of *data* from the *srcVertex* on *edge*.
     *
     * @param[in]  graph The graph in which the communication takes place.
     * @param[in]  edge Edge over which the *data* will be transmitted.
     * @param[out] data Data that will be received
     * @param[out] List of events the send event will be added to.
     *
     */
    template <typename T>
    /// requires concept::ContiguousContainer<T>
    void recv(const Edge& edge, T& data, std::vector<Event>& events);

    /** @} */

    /**********************************************************************/ /**
      *
      * @name Collective Communication Operations
      *
      * @{
      *
      **************************************************************************/

    template <typename T, typename T_Op>
    /// requires concept::ContiguousContainer<T>
    /// requires concept::BinaeryOperator<T_Op>
    void reduce(
        const Vertex& rootVertex, const Vertex& srcVertex, T_Op op, const T sendData, T& recvData);

    template <typename T, typename T_Op>
    /// requires concept::ContiguousContainer<T>
    /// requires concept::BinaeryOperator<T_Op>
    void allReduce(const Vertex& srcVertex, T_Op op, const T sendData, T& recvData);

    template <typename T_Send, typename T_Recv>
    /// requires concept::ContiguousContainer<T_Send>
    /// requires concept::ContiguousContainer<T_Recv>
    void gather(
        const Vertex& rootVertex,
        const Vertex& srcVertex,
        const T_Send sendData,
        T_Recv& recvData,
        const bool reorder);

    template <typename T_Send, typename T_Recv>
    /// requires concept::ContiguousContainer<T_Send>
    /// requires concept::ContiguousContainer<T_Recv>
    void allGather(const Vertex& srcVertex, T_Send sendData, T_Recv& recvData, const bool reorder);

    /**
     * @brief Spread data from a vertex to all adjacent vertices
     *        connected by an outgoing edge (async).
     *
     * @param[in]  vertex to spread data from
     * @param[in]  data   that will be spreaded
     * @param[out] events where the events for this async operations will be
     * inserted.
     *
     */
    template <typename T>
    /// requires concept::ContiguousContainer<T>
    void spread(const Vertex& vertex, const T& data, std::vector<Event>& events);

    /**
     * @brief Spread data from a vertex to all adjacent vertices
     *        connected by an outgoing edge (sync).
     *
     * @param[in]  vertex to spread data from
     * @param[in]  data   that will be spreaded
     *
     */
    /// requires concept::ContiguousContainer<T>
    template <typename T> void spread(const Vertex& vertex, const T& data);

    /**
     * @brief Collects data from all incoming edges under the
     *        assumption that all vertices send the same  number
     *        of data.
     *
     *
     * @param[in]  vertex that collects data
     * @param[in]  data were collected data will be stored
     *
     */
    /// requires concept::ContiguousContainer<T>
    template <typename T> void collect(const Vertex& vertex, T& data);

    void synchronize();

    /** @} */

  private:
    /***************************************************************************
     *
     * MEMBER
     *
     ***************************************************************************/
    std::shared_ptr<CommunicationPolicy> communicator;
    GraphPolicy graph;
    SerializationPolicy serializer;
    std::vector<Vertex> hostedVertices;

    Context graphContext;

    /***************************************************************************
     *
     * MAPS
     *
     ***************************************************************************/
    // Maps vertices to its hosts
    std::map<VertexID, VAddr> vertexMap;

    // List of vertices of the hosts
    std::map<VAddr, std::vector<Vertex>> peerMap;

    /**
     * @brief Reorders data received from vertices into vertex id order.
     *
     */
    template <class T>
    void reorder(
        const std::vector<T>& data,
        const std::vector<unsigned>& recvCount,
        std::vector<T>& dataReordered);
};

//!
//! GRAPH OPERATIONS
//!

template <typename T_CommunicationPolicy, typename T_GraphPolicy, typename SerializationPolicy>
template <typename T_Functor>
auto Cage<T_CommunicationPolicy, T_GraphPolicy, SerializationPolicy>::setGraph(
    T_Functor graphFunctor) -> void
{
    graph = T_GraphPolicy(graphFunctor());
}

template <typename T_CommunicationPolicy, typename T_GraphPolicy, typename SerializationPolicy>
auto Cage<T_CommunicationPolicy, T_GraphPolicy, SerializationPolicy>::getVertices()
    -> std::vector<Vertex>
{
    using Iter = graybat::graphPolicy::AllVertexIter<GraphPolicy>;
    std::vector<Vertex> vertices;

    Iter vi_first, vi_last;
    std::tie(vi_first, vi_last) = graph.getVertices();

    while (vi_first != vi_last) {
        vertices.push_back(Vertex(
            graph.getVertexProperty(*vi_first).first,
            graph.getVertexProperty(*vi_first).second,
            *this));
        vi_first++;
    }

    return vertices;
}

template <typename T_CommunicationPolicy, typename T_GraphPolicy, typename SerializationPolicy>
auto Cage<T_CommunicationPolicy, T_GraphPolicy, SerializationPolicy>::getVertex(
    const VertexID& vertexID) -> Vertex
{
    typedef typename GraphPolicy::AllVertexIter Iter;

    Iter vi_first, vi_last;
    std::tie(vi_first, vi_last) = graph.getVertices();

    std::advance(vi_first, vertexID);

    return Vertex(
        graph.getVertexProperty(*vi_first).first, graph.getVertexProperty(*vi_first).second, *this);
}

template <typename T_CommunicationPolicy, typename T_GraphPolicy, typename SerializationPolicy>
auto Cage<T_CommunicationPolicy, T_GraphPolicy, SerializationPolicy>::getEdge(
    const Vertex& source, const Vertex& target) -> Edge
{
    std::pair<EdgeID, bool> edge = graph.getEdge(source.id, target.id);

    if (edge.second) {
        return Edge(
            graph.getEdgeProperty(edge.first).first,
            getVertex(graph.getEdgeSource(edge.first)),
            getVertex(graph.getEdgeTarget(edge.first)),
            graph.getEdgeProperty(edge.first).second,
            *this);
    } else {
        throw std::runtime_error("Edge between does not exist");
    }
}

template <typename T_CommunicationPolicy, typename T_GraphPolicy, typename SerializationPolicy>
auto Cage<T_CommunicationPolicy, T_GraphPolicy, SerializationPolicy>::getAdjacentVertices(
    const Vertex& v) -> std::vector<Vertex>
{
    typedef typename GraphPolicy::AdjacentVertexIter Iter;

    std::vector<Vertex> adjacentVertices;

    Iter avi_first, avi_last;
    std::tie(avi_first, avi_last) = graph.getAdjacentVertices(v.id);

    while (avi_first != avi_last) {
        adjacentVertices.push_back(Vertex(
            graph.getVertexProperty(*avi_first).first,
            graph.getVertexProperty(*avi_first).second,
            *this));
        avi_first++;
    }

    return adjacentVertices;
}

template <typename T_CommunicationPolicy, typename T_GraphPolicy, typename SerializationPolicy>
auto Cage<T_CommunicationPolicy, T_GraphPolicy, SerializationPolicy>::getOutEdges(const Vertex& v)
    -> std::vector<Edge>
{
    typedef typename GraphPolicy::OutEdgeIter Iter;
    std::vector<Edge> outEdges;

    Iter oi_first, oi_last;
    std::tie(oi_first, oi_last) = graph.getOutEdges(v.id);

    while (oi_first != oi_last) {
        outEdges.push_back(Edge(
            graph.getEdgeProperty(*oi_first).first,
            getVertex(graph.getEdgeSource(*oi_first)),
            getVertex(graph.getEdgeTarget(*oi_first)),
            graph.getEdgeProperty(*oi_first).second,
            *this));
        oi_first++;
    }

    return outEdges;
}

template <typename T_CommunicationPolicy, typename T_GraphPolicy, typename SerializationPolicy>
auto Cage<T_CommunicationPolicy, T_GraphPolicy, SerializationPolicy>::getInEdges(const Vertex& v)
    -> std::vector<Edge>
{
    typedef typename GraphPolicy::InEdgeIter Iter;
    std::vector<Edge> inEdges;

    Iter ii_first, ii_last;
    std::tie(ii_first, ii_last) = graph.getInEdges(v.id);

    while (ii_first != ii_last) {
        inEdges.push_back(Edge(
            graph.getEdgeProperty(*ii_first).first,
            getVertex(graph.getEdgeSource(*ii_first)),
            getVertex(graph.getEdgeTarget(*ii_first)),
            graph.getEdgeProperty(*ii_first).second,
            *this));
        ii_first++;
    }

    return inEdges;
}

//!
//! MAPPING OPERATIONS
//!

template <typename T_CommunicationPolicy, typename T_GraphPolicy, typename SerializationPolicy>
template <typename T_Functor>
auto Cage<T_CommunicationPolicy, T_GraphPolicy, SerializationPolicy>::distribute(
    T_Functor distFunctor) -> void
{
    hostedVertices = distFunctor(
        communicator->getGlobalContext().getVAddr(),
        communicator->getGlobalContext().size(),
        *this);

    announce(hostedVertices);
}

template <typename T_CommunicationPolicy, typename T_GraphPolicy, typename SerializationPolicy>
auto Cage<T_CommunicationPolicy, T_GraphPolicy, SerializationPolicy>::announce(
    const std::vector<Vertex>& vertices, const bool global) -> void
{
    // Get old context from graph
    Context oldContext = graphContext;

    if (global) {
        oldContext = communicator->getGlobalContext();
    }

    assert(oldContext.valid());

    graphContext = communicator->splitContext(vertices.size(), oldContext);

    // Each peer announces the vertices it hosts
    if (graphContext.valid()) {
        std::array<unsigned, 1> nVertices{ { static_cast<unsigned>(vertices.size()) } };
        std::vector<unsigned> vertexIDs;

        std::for_each(vertices.begin(), vertices.end(), [&vertexIDs](Vertex v) {
            vertexIDs.push_back(v.id);
        });

        // Send hostedVertices to all other peers
        for (auto const& vAddr : graphContext) {
            assert(nVertices[0] != 0);
            communicator->asyncSend(vAddr, 0, graphContext, nVertices);
            communicator->asyncSend(vAddr, 0, graphContext, vertexIDs);
        }

        // Recv hostedVertices from all other peers
        for (auto const& vAddr : graphContext) {
            std::vector<Vertex> remoteVertices;
            std::array<unsigned, 1> nVertices{ { 0 } };
            communicator->recv(vAddr, 0, graphContext, nVertices);
            std::vector<unsigned> vertexIDs(nVertices[0]);
            communicator->recv(vAddr, 0, graphContext, vertexIDs);

            for (unsigned u : vertexIDs) {
                vertexMap[u] = vAddr;
                remoteVertices.push_back(Cage::getVertex(u));
            }
            peerMap[vAddr] = remoteVertices;
        }
    }
}

template <typename T_CommunicationPolicy, typename T_GraphPolicy, typename SerializationPolicy>
auto Cage<T_CommunicationPolicy, T_GraphPolicy, SerializationPolicy>::locateVertex(
    const Vertex& vertex) -> VAddr
{
    auto it = vertexMap.find(vertex.id);
    if (it != vertexMap.end()) {
        return (*it).second;
    } else {
        std::stringstream errorMsg;
        errorMsg << "[" << communicator->getGlobalContext().getVAddr() << "] No host of vertex "
                 << vertex.id << " known.";
        throw std::runtime_error(errorMsg.str());
    }
}

template <typename T_CommunicationPolicy, typename T_GraphPolicy, typename SerializationPolicy>
auto Cage<T_CommunicationPolicy, T_GraphPolicy, SerializationPolicy>::getVerticesHostedBy(
    const VAddr& vAddr) -> std::vector<Vertex>
{
    return peerMap[vAddr];
}

template <typename T_CommunicationPolicy, typename T_GraphPolicy, typename SerializationPolicy>
auto Cage<T_CommunicationPolicy, T_GraphPolicy, SerializationPolicy>::getHostedVertices()
    -> std::vector<Vertex>&
{
    return hostedVertices;
}

template <typename T_CommunicationPolicy, typename T_GraphPolicy, typename SerializationPolicy>
auto Cage<T_CommunicationPolicy, T_GraphPolicy, SerializationPolicy>::isHosting(
    const Vertex& vertex) -> bool
{
    VAddr vaddr = graphContext.getVAddr();

    for (Vertex& v : getVerticesHostedBy(vaddr)) {
        if (vertex.id == v.id)
            return true;
    }
    return false;
}

template <typename T_CommunicationPolicy, typename T_GraphPolicy, typename SerializationPolicy>
auto Cage<T_CommunicationPolicy, T_GraphPolicy, SerializationPolicy>::getPeers()
    -> std::vector<Peer>
{
    unsigned nPeers = communicator->getGlobalContext().size();
    return std::vector<Peer>(nPeers);
}

template <typename T_CommunicationPolicy, typename T_GraphPolicy, typename SerializationPolicy>
auto Cage<T_CommunicationPolicy, T_GraphPolicy, SerializationPolicy>::getCommunicationPolicy()
    -> std::shared_ptr<CommunicationPolicy>
{
    return communicator;
}

//!
//! Point to Point Communication Operations
//!

template <typename T_CommunicationPolicy, typename T_GraphPolicy, typename SerializationPolicy>
template <typename T>
auto Cage<T_CommunicationPolicy, T_GraphPolicy, SerializationPolicy>::send(
    const Edge& edge, const T& data) -> void
{
    VAddr destVAddr = locateVertex(edge.target);
    decltype(auto) serialized = SerializationPolicy::serialize(data);
    communicator->send(destVAddr, edge.id, graphContext, serialized);
}

template <typename T_CommunicationPolicy, typename T_GraphPolicy, typename SerializationPolicy>
template <typename T>
auto Cage<T_CommunicationPolicy, T_GraphPolicy, SerializationPolicy>::send(
    const Edge& edge, const T& data, std::vector<Event>& events) -> void
{
    // std::cout << "send cage:" << edge.target.id << " " << edge.id << std::endl;
    VAddr destVAddr = locateVertex(edge.target);

    decltype(auto) serialized = SerializationPolicy::serialize(data);
    CPEvent e = communicator->asyncSend(destVAddr, edge.id, graphContext, serialized);
    events.push_back({ e, [serialized]() {} });
}

template <typename T_CommunicationPolicy, typename T_GraphPolicy, typename SerializationPolicy>
template <typename T>
auto Cage<T_CommunicationPolicy, T_GraphPolicy, SerializationPolicy>::recv(
    const Edge& edge, T& data) -> void
{
    // std::cout << "recv cage:" << edge.source.id << " " << edge.id << std::endl;
    VAddr srcVAddr = locateVertex(edge.source);

    decltype(auto) skeleton = SerializationPolicy::prepare(data);
    communicator->recv(srcVAddr, edge.id, graphContext, skeleton);
    SerializationPolicy::restore(data, skeleton);
}

template <typename T_CommunicationPolicy, typename T_GraphPolicy, typename SerializationPolicy>
template <typename T>
auto Cage<T_CommunicationPolicy, T_GraphPolicy, SerializationPolicy>::recv(T& data) -> Edge
{
    decltype(auto) skeleton = SerializationPolicy::prepare(data);
    CPEvent event = communicator->recv(graphContext, skeleton);
    SerializationPolicy::restore(data, skeleton);

    return Edge(
        graph.getEdgeProperty(event.getTag()).first,
        getVertex(graph.getEdgeSource(event.getTag())),
        getVertex(graph.getEdgeTarget(event.getTag())),
        graph.getEdgeProperty(event.getTag()).second,
        *this);
}

template <typename T_CommunicationPolicy, typename T_GraphPolicy, typename SerializationPolicy>
template <typename T>
auto Cage<T_CommunicationPolicy, T_GraphPolicy, SerializationPolicy>::recv(
    const Edge& edge, T& data, std::vector<Event>& events) -> void
{
    VAddr srcVAddr = locateVertex(edge.source);
    decltype(auto) skeleton = SerializationPolicy::prepare(data);
    CPEvent e = communicator->asyncRecv(srcVAddr, edge.id, graphContext, skeleton);

    events.push_back({ e, [&data, skeleton]() { SerializationPolicy::restore(data, skeleton); } });
}

//!
//! Collective Communication Operations
//!

template <typename T_CommunicationPolicy, typename T_GraphPolicy, typename SerializationPolicy>
template <typename T_Data, typename Op>
auto Cage<T_CommunicationPolicy, T_GraphPolicy, SerializationPolicy>::reduce(
    const Vertex& rootVertex,
    const Vertex& srcVertex,
    Op op,
    const T_Data sendData,
    T_Data& recvData) -> void
{
    using value_type = typename T_Data::value_type;

    static std::vector<value_type> reduce;
    static std::vector<value_type>* rootRecvData;
    static unsigned vertexCount = 0;
    static bool hasRootVertex = false;

    VAddr rootVAddr = locateVertex(rootVertex);
    VAddr srcVAddr = locateVertex(srcVertex);
    Context context = graphContext;
    std::vector<Vertex> vertices = getVerticesHostedBy(srcVAddr);

    vertexCount++;

    if (reduce.empty()) {
        reduce = std::vector<value_type>(sendData.size(), 0);
    }

    // Reduce locally
    for (std::size_t i = 0; i < reduce.size(); ++i) {
        reduce[i] = op(reduce[i], sendData.data()[i]);
    }

    // Remember pointer of recvData from rootVertex
    if (rootVertex.id == srcVertex.id) {
        hasRootVertex = true;
        rootRecvData = &recvData;
    }

    // Finally start reduction
    if (vertexCount == vertices.size()) {

        decltype(auto) serialized = SerializationPolicy::serialize(reduce);
        if (hasRootVertex) {
            decltype(auto) skeleton = SerializationPolicy::prepare(*rootRecvData);
            communicator->reduce(rootVAddr, context, op, serialized, skeleton);
            SerializationPolicy::restore(*rootRecvData, skeleton);
        } else {
            communicator->reduce(rootVAddr, context, op, serialized, serialized);
        }

        reduce.clear();
        vertexCount = 0;
    }
    assert(vertexCount <= vertices.size());
}

template <typename T_CommunicationPolicy, typename T_GraphPolicy, typename SerializationPolicy>
template <typename T, typename T_Op>
auto Cage<T_CommunicationPolicy, T_GraphPolicy, SerializationPolicy>::allReduce(
    const Vertex& srcVertex, T_Op op, const T sendData, T& recvData) -> void
{

    static std::vector<typename T::value_type> reduce;
    static unsigned vertexCount = 0;
    static std::vector<T*> recvDatas;

    VAddr srcVAddr = locateVertex(srcVertex);
    Context context = graphContext;
    std::vector<Vertex> vertices = getVerticesHostedBy(srcVAddr);

    recvDatas.push_back(&recvData);

    vertexCount++;

    if (reduce.empty()) {
        reduce = std::vector<typename T::value_type>(sendData.size(), 0);
    }

    // Reduce locally
    for (std::size_t i = 0; i < reduce.size(); ++i) {
        reduce[i] = op(reduce[i], sendData.data()[i]);
    }

    // Finally start reduction
    if (vertexCount == vertices.size()) {

        decltype(auto) serialized = SerializationPolicy::serialize(reduce);
        decltype(auto) skeleton = SerializationPolicy::prepare(*(recvDatas[0]));
        communicator->allReduce(context, op, serialized, skeleton);
        SerializationPolicy::restore(*(recvDatas[0]), skeleton);

        // Distribute Received Data to Hosted Vertices
        for (unsigned i = 1; i < recvDatas.size(); ++i) {
            std::copy(recvDatas[0]->begin(), recvDatas[0]->end(), recvDatas[i]->begin());
        }

        reduce.clear();
        vertexCount = 0;
    }
    assert(vertexCount <= vertices.size());
}

template <typename T_CommunicationPolicy, typename T_GraphPolicy, typename SerializationPolicy>
template <typename T_Send, typename T_Recv>
auto Cage<T_CommunicationPolicy, T_GraphPolicy, SerializationPolicy>::gather(
    const Vertex& rootVertex,
    const Vertex& srcVertex,
    const T_Send sendData,
    T_Recv& recvData,
    const bool reorder) -> void
{
    typedef typename T_Send::value_type SendValueType;
    typedef typename T_Recv::value_type RecvValueType;

    static std::vector<SendValueType> gather;
    static T_Recv* rootRecvData = NULL;
    static bool peerHostsRootVertex = false;
    static unsigned nGatherCalls = 0;

    nGatherCalls++;

    VAddr rootVAddr = locateVertex(rootVertex);
    Context context = graphContext;

    // Insert data of srcVertex to the end of the gather vector
    for (std::size_t i = 0; i < sendData.size(); ++i) {
        gather.push_back(sendData.data()[i]);
    }

    // Store recv pointer of rootVertex
    if (srcVertex.id == rootVertex.id) {
        rootRecvData = &recvData;
        peerHostsRootVertex = true;
    }

    if (nGatherCalls == hostedVertices.size()) {
        std::vector<unsigned> recvCount;
        if (peerHostsRootVertex) {
            communicator->gatherVar(rootVAddr, context, gather, *rootRecvData, recvCount);

            // Reorder the received data, so that the data
            // is in vertex id order. This operation is no
            // sorting since the mapping is known before.
            if (reorder) {
                std::vector<RecvValueType> recvDataReordered(recvData.size());
                Cage::reorder(*rootRecvData, recvCount, recvDataReordered);
                std::copy(
                    recvDataReordered.begin(), recvDataReordered.end(), rootRecvData->begin());
            }

        } else {
            communicator->gatherVar(rootVAddr, context, gather, recvData, recvCount);
        }

        gather.clear();
        nGatherCalls = 0;
    }
}

template <typename T_CommunicationPolicy, typename T_GraphPolicy, typename SerializationPolicy>
template <typename T_Send, typename T_Recv>
auto Cage<T_CommunicationPolicy, T_GraphPolicy, SerializationPolicy>::allGather(
    const Vertex& srcVertex, T_Send sendData, T_Recv& recvData, const bool reorder) -> void
{
    typedef typename T_Send::value_type SendValueType;
    typedef typename T_Recv::value_type RecvValueType;

    static std::vector<SendValueType> gather;
    static std::vector<T_Recv*> recvDatas;
    static unsigned nGatherCalls = 0;
    nGatherCalls++;
    boost::ignore_unused(srcVertex);

    Context context = graphContext;

    for (std::size_t i = 0; i < sendData.size(); ++i) {
        gather.push_back(sendData.data()[i]);
    }
    recvDatas.push_back(&recvData);

    if (nGatherCalls == hostedVertices.size()) {
        std::vector<unsigned> recvCount;

        communicator->allGatherVar(context, gather, *(recvDatas[0]), recvCount);

        // Reordering code
        if (reorder) {
            std::vector<RecvValueType> recvDataReordered(recvData.size());
            Cage::reorder(*(recvDatas[0]), recvCount, recvDataReordered);
            std::copy(recvDataReordered.begin(), recvDataReordered.end(), recvDatas[0]->begin());
        }

        // Distribute Received Data to Hosted Vertices
        // unsigned nElements = std::accumulate(recvCount.begin(), recvCount.end(),
        // 0);
        for (unsigned i = 1; i < recvDatas.size(); ++i) {
            std::copy(recvDatas[0]->begin(), recvDatas[0]->end(), recvDatas[i]->begin());
        }

        gather.clear();
    }
}

template <typename T_CommunicationPolicy, typename T_GraphPolicy, typename SerializationPolicy>
template <typename T>
auto Cage<T_CommunicationPolicy, T_GraphPolicy, SerializationPolicy>::spread(
    const Vertex& vertex, const T& data, std::vector<Event>& events) -> void
{
    std::vector<Edge> edges = getOutEdges(vertex);
    for (Edge edge : edges) {
        Cage::send(edge, data, events);
    }
}

template <typename T_CommunicationPolicy, typename T_GraphPolicy, typename SerializationPolicy>
template <typename T>
auto Cage<T_CommunicationPolicy, T_GraphPolicy, SerializationPolicy>::spread(
    const Vertex& vertex, const T& data) -> void
{
    std::vector<Edge> edges = getOutEdges(vertex);
    for (Edge edge : edges) {
        Cage::send(edge, data);
    }
}

template <typename T_CommunicationPolicy, typename T_GraphPolicy, typename SerializationPolicy>
template <typename T>
auto Cage<T_CommunicationPolicy, T_GraphPolicy, SerializationPolicy>::collect(
    const Vertex& vertex, T& data) -> void
{
    std::vector<Edge> edges = getInEdges(vertex);
    for (unsigned i = 0; i < edges.size(); ++i) {
        unsigned elementsPerEdge = data.size() / edges.size();
        std::vector<typename T::value_type> elements(elementsPerEdge);
        Cage::recv(edges[i], elements);
        std::memcpy(
            data.data() + (i * elementsPerEdge),
            elements.data(),
            elements.size() * sizeof(typename T::value_type));
    }
}

template <typename T_CommunicationPolicy, typename T_GraphPolicy, typename SerializationPolicy>
auto Cage<T_CommunicationPolicy, T_GraphPolicy, SerializationPolicy>::synchronize() -> void
{
    communicator->synchronize(graphContext);
}

//!
//! Utilities
//!

template <typename T_CommunicationPolicy, typename T_GraphPolicy, typename SerializationPolicy>
template <class T>
auto Cage<T_CommunicationPolicy, T_GraphPolicy, SerializationPolicy>::reorder(
    const std::vector<T>& data,
    const std::vector<unsigned>& recvCount,
    std::vector<T>& dataReordered) -> void
{
    std::vector<unsigned> prefixsum(graphContext.size(), 0);

    utils::exclusivePrefixSum(recvCount.begin(), recvCount.end(), prefixsum.begin());

    for (auto const& vAddr : graphContext) {
        const std::vector<Vertex> hostedVertices = getVerticesHostedBy(vAddr);
        const unsigned nElementsPerVertex
            = recvCount.at(vAddr) / static_cast<unsigned>(hostedVertices.size());

        for (unsigned hostVertex_i = 0; hostVertex_i < hostedVertices.size(); hostVertex_i++) {

            unsigned sourceOffset = prefixsum[vAddr] + (hostVertex_i * nElementsPerVertex);
            unsigned targetOffset = hostedVertices[hostVertex_i].id * nElementsPerVertex;

            std::copy(
                data.begin() + sourceOffset,
                data.begin() + sourceOffset + nElementsPerVertex,
                dataReordered.begin() + targetOffset);
        }
    }
}

} // namespace graybat
