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

#include <graybat/communicationPolicy/Traits.hpp>

namespace graybat {

namespace communicationPolicy {

template <typename T_CommunicationPolicy> struct Base {

    using CommunicationPolicy = T_CommunicationPolicy;
    using VAddr = typename graybat::communicationPolicy::VAddr<CommunicationPolicy>;
    using Tag = typename graybat::communicationPolicy::Tag<CommunicationPolicy>;
    using Context = typename graybat::communicationPolicy::Context<CommunicationPolicy>;
    using Event = typename graybat::communicationPolicy::Event<CommunicationPolicy>;
    using Status = typename graybat::communicationPolicy::Status<CommunicationPolicy>;

    /***********************************************************************
     * Interface
     ***********************************************************************/

    /***********************************************************************/ /**
      *
      * @name Point to Point Communication Interface
      *
      * @{
      *
      ***************************************************************************/
    template <typename T_Send>
    void send(const VAddr destVAddr, const Tag tag, const Context context, const T_Send& sendData)
        = delete;

    template <typename T_Send>
    Event
    asyncSend(const VAddr destVAddr, const Tag tag, const Context context, const T_Send& sendData)
        = delete;

    template <typename T_Recv>
    void recv(const VAddr srcVAddr, const Tag tag, const Context context, T_Recv& recvData)
        = delete;

    template <typename T_Recv> Event recv(const Context context, T_Recv& recvData) = delete;

	template <typename T_Recv> Event asncRecv(const Context context, T_Recv& recvData) = delete;

    template <typename T_Recv>
    Event asyncRecv(const VAddr srcVAddr, const Tag tag, const Context context, T_Recv& recvData)
        = delete;

    /** Test if message is available from peer with srcVAddr and particular tag.
     *
     * @param[in]  srcVAddr   VAddr of peer that sended the message
     * @param[in]  tag        Description of the message to better distinguish messages types
     * @param[in]  context    Context in which both sender and receiver are included
     *
     * @return optional Status contains information if message is available
     */
    Status probe(const VAddr srcVAddr, const Tag tag, const Context context) = delete;

    /** Test if message is available from peer with srcVAddr and particular tag
     *  Returns immediately.
     *
     * @param[in]  srcVAddr   VAddr of peer that sended the message
     * @param[in]  tag        Description of the message to better distinguish messages types
     * @param[in]  context    Context in which both sender and receiver are included
     *
     * @return optional Status contains information if message is available
     */
    boost::optional<Status> asyncProbe(const VAddr srcVAddr, const Tag tag, const Context context)
        = delete;


    /** @} */

    /************************************************************************/ /**
      *
      * @name Collective Communication Interface
      *
      * @{
      *
      **************************************************************************/
    /**
     * @brief Collects *sendData* from all peers of the *context* and
     *        transmits it as a list to the peer with
     *        *rootVAddr*. Data of all peers has to be from the
     *        **same** size.
     *
     * @param[in]  rootVAddr  Peer that will receive collcted data from *context* members
     * @param[in]  context    Set of peers that want to send Data
     * @param[in]  sendData   Data that every peer in the *context* sends.
     *                        Data of all peers in the *context* need to have **same** size().
     * @param[out] recvData   Data from all *context* members, that peer with virtual address
     *                        *rootVAddr* will receive. *recvData* of all other members of the
     *                        *context* will be empty.
     */
    template <typename T_Send, typename T_Recv>
    void
    gather(const VAddr rootVAddr, const Context context, const T_Send& sendData, T_Recv& recvData);

    /**
     * @brief Collects *sendData* from all members of the *context*
     *        with **varying** size and transmits it as a list to peer
     *        with *rootVAddr*.
     *
     * @param[in]  rootVAddr  Peer that will receive collcted data from *context* members
     * @param[in]  context    Set of peers that want to send Data
     * @param[in]  sendData   Data that every peer in the *context* sends. The Data can have
     * **varying** size
     * @param[out] recvData   Data from all *context* peers, that peer with *rootVAddr* will
     * receive.
     *                        *recvData* of all other peers of the *context* will be empty. The
     * received
     *                        data is ordered by the VAddr of the peers.
     * @param[out] recvCount  Number of elements each peer sends (can by varying).
     *
     */
    template <typename T_Send, typename T_Recv>
    void gatherVar(
        const VAddr rootVAddr,
        const Context context,
        const T_Send& sendData,
        T_Recv& recvData,
        std::vector<unsigned>& recvCount);

    /**
     * @brief Collects *sendData* from all members of the *context*  and transmits it as a list
     *        to every peer in the *context*
     *
     * @param[in]  context  Set of peers that want to send Data
     * @param[in]  sendData Data that every peer in the *context* sends with **same** size
     * @param[out] recvData Data from all *context* members, that all peers* will receive.
     *
     */
    template <typename T_Send, typename T_Recv>
    void allGather(Context context, const T_Send& sendData, T_Recv& recvData);

    /**
     * @brief Collects *sendData* from all peers of the *context*. Size of *sendData* can vary in
     * size.
     *        The data is received by every peer in the *context*.
     *
     * @param[in]  context    Set of peers that want to send Data
     * @param[in]  sendData   Data that every peer in the *context* sends with **varying** size
     * @param[out] recvData   Data from all *context* members, that all peers* will receive.
     * @param[out] recvCount  Number of elements each peer sends (can by varying).
     *
     */
    template <typename T_Send, typename T_Recv>
    void allGatherVar(
        const Context context,
        const T_Send& sendData,
        T_Recv& recvData,
        std::vector<unsigned>& recvCount);

    /**
     * @brief Distributes *sendData* from peer *rootVAddr* to all peers in *context*.
     *        Every peer will receive different data.
     *
     * @remark In Contrast to broadcast where every peer receives the same data
     *
     * @param[in]  rootVAddr peer that want to distribute its data
     * @param[in]  context    Set of peers that want to receive Data
     * @param[in]  sendData   Data that peer with *rootVAddr* will distribute over the peers of the
     * *context*
     * @param[out] recvData   Data from peer with *rootVAddr*.
     *
     */
    template <typename T_Send, typename T_Recv>
    void
    scatter(const VAddr rootVAddr, const Context context, const T_Send& sendData, T_Recv& recvData);

    /**
     * @brief Distributes *sendData* of all peer in the *context* to all peers in the *context*.
     *        Every peer will receive data from every other peer (also the own data)
     *
     * @param[in]  context  Set of peers that want to receive Data
     * @param[in]  sendData Data that each peer wants to send. Each peer will receive
     *             same number of data elements, but not the same data elements. sendData
     *             will be divided in equal chunks of data and is then distributed.
     *
     * @param[out] recvData Data from all peer.
     *
     */
    template <typename T_Send, typename T_Recv>
    void allScatter(const Context context, const T_Send& sendData, T_Recv& recvData);

    /**
     * @brief Performs a reduction with a binary operator *op* on all *sendData* elements from all
     * peers
     *        whithin the *context*. The result will be received by the peer with *rootVAddr*.
     *        Binary operations like std::plus, std::minus can be used. But, they can also be
     *        defined as binary operator simular to std::plus etc.
     *
     *
     * @param[in]  rootVAddr peer that will receive the result of reduction
     * @param[in]  context   Set of peers that
     * @param[in]  op        Binary operator that should be used for reduction
     * @param[in]  sendData  Data that every peer contributes to the reduction
     * @param[out] recvData  Reduced sendData that will be received by peer with *rootVAddr*.
     *                       It will have same size of sendData and contains the ith
     *                       reduced sendData values.
     *
     */
    template <typename T_Send, typename T_Recv, typename T_Op>
    void reduce(
        const VAddr rootVAddr,
        const Context context,
        const T_Op op,
        const T_Send& sendData,
        T_Recv& recvData);

    /**
     * @brief Performs a reduction with a binary operator *op* on all *sendData* elements from all
     * peers
     *        whithin the *context*. The result will be received by all peers.
     *
     * @param[in] context    Set of peers that
     * @param[in] op         Binary operator that should be used for reduction
     * @param[in] sendData   Data that every peer contributes to the reduction
     * @param[out] recvData  Reduced sendData that will be received by all peers.
     *                       It will have same size of sendData and contains the ith
     *                       reduced sendData values.
     *
     */
    template <typename T_Send, typename T_Recv, typename T_Op>
    void allReduce(const Context context, T_Op op, const T_Send& sendData, T_Recv& recvData);

    /**
     * @brief Send *sendData* from peer *rootVAddr* to all peers in *context*.
     *        Every peer will receive the same data.
     *
     * @remark In Contrast to scatter where every peer receives different data
     *
     * @param[in] rootVAddr Source peer
     * @param[in] context    Set of peers that want to receive Data
     * @param[in] sendData   Data that peer with *rootVAddr* will send to the peers of the *context*
     * @param[out] recvData  Data from peer with *rootVAddr*.
     */
    template <typename T_SendRecv>
    void broadcast(const VAddr rootVAddr, const Context context, T_SendRecv& data);

    /**
     * @brief Synchronizes all peers within *context* to the same point
     *        in the programm execution (barrier).
     *
     */
    void synchronize(const Context context);
    /** @} */

    /***********************************************************************/ /**
      *
      * @name Context Interface
      *
      * @{
      *
      ***************************************************************************/

    /**
     * @brief Returns a subcontext of *oldContext* with all peers that want to participate.
     *        Peers which do not want to participate retrieve an invalid context.
     */
    Context splitContext(const bool isMember, const Context oldContext) = delete;

    /**
     * @brief Returns the context that contains all peers
     */
    Context getGlobalContext() = delete;

    /** @} */
};

/***********************************************************************
 * Implementation
 ***********************************************************************/
template <typename T_CommunicationPolicy>
template <typename T_Send, typename T_Recv>
void Base<T_CommunicationPolicy>::gather(
    const VAddr rootVAddr, const Context context, const T_Send& sendData, T_Recv& recvData)
{
    using RecvValueType = typename T_Recv::value_type;
    using CommunicationPolicy = T_CommunicationPolicy;
    using Event = Base<CommunicationPolicy>::Event;

    Event e = static_cast<CommunicationPolicy*>(this)->asyncSend(rootVAddr, 0, context, sendData);

    if (rootVAddr == context.getVAddr()) {
        for (auto const& vAddr : context) {
            size_t recvOffset = vAddr * sendData.size();
            std::vector<RecvValueType> tmpData(sendData.size());
            static_cast<CommunicationPolicy*>(this)->recv(vAddr, 0, context, tmpData);
            std::copy(tmpData.begin(), tmpData.end(), recvData.begin() + recvOffset);
        }
    }

    e.wait();
}

template <typename T_CommunicationPolicy>
template <typename T_Send, typename T_Recv>
void Base<T_CommunicationPolicy>::gatherVar(
    const VAddr rootVAddr,
    const Context context,
    const T_Send& sendData,
    T_Recv& recvData,
    std::vector<unsigned>& recvCount)
{
    using RecvValueType = typename T_Recv::value_type;
    using CommunicationPolicy = T_CommunicationPolicy;
    using Event = Base<CommunicationPolicy>::Event;

    std::array<unsigned, 1> nElements{ { (unsigned)sendData.size() } };
    recvCount.resize(context.size());
    static_cast<CommunicationPolicy*>(this)->allGather(context, nElements, recvCount);
    recvData.resize(std::accumulate(recvCount.begin(), recvCount.end(), 0U));

    Event e = static_cast<CommunicationPolicy*>(this)->asyncSend(rootVAddr, 0, context, sendData);

    if (rootVAddr == context.getVAddr()) {
        size_t recvOffset = 0;
        for (auto const& vAddr : context) {
            std::vector<RecvValueType> tmpData(recvCount.at(vAddr));
            static_cast<CommunicationPolicy*>(this)->recv(vAddr, 0, context, tmpData);
            std::memcpy(
                recvData.data() + recvOffset,
                tmpData.data(),
                tmpData.size() * sizeof(RecvValueType));
            recvOffset += recvCount.at(vAddr);
        }
    }

    e.wait();
}

template <typename T_CommunicationPolicy>
template <typename T_Send, typename T_Recv>
void Base<T_CommunicationPolicy>::allGather(
    Context context, const T_Send& sendData, T_Recv& recvData)
{
    using RecvValueType = typename T_Recv::value_type;
    using CommunicationPolicy = T_CommunicationPolicy;
    using Event = Base<CommunicationPolicy>::Event;

    std::vector<Event> events;

    for (auto const& vAddr : context) {
        events.push_back(
            static_cast<CommunicationPolicy*>(this)->asyncSend(vAddr, 0, context, sendData));
    }

    for (auto const& vAddr : context) {
        size_t recvOffset = vAddr * sendData.size();
        std::vector<RecvValueType> tmpData(sendData.size());
        static_cast<CommunicationPolicy*>(this)->recv(vAddr, 0, context, tmpData);
        std::memcpy(
            recvData.data() + recvOffset, tmpData.data(), tmpData.size() * sizeof(RecvValueType));
    }

    for (unsigned i = 0; i < events.size(); ++i) {
        events.back().wait();
        events.pop_back();
    }
}

template <typename T_CommunicationPolicy>
template <typename T_Send, typename T_Recv>
void Base<T_CommunicationPolicy>::allGatherVar(
    const Context context,
    const T_Send& sendData,
    T_Recv& recvData,
    std::vector<unsigned>& recvCount)
{
    using RecvValueType = typename T_Recv::value_type;
    using CommunicationPolicy = T_CommunicationPolicy;
    using Event = Base<CommunicationPolicy>::Event;

    std::vector<Event> events;

    std::array<unsigned, 1> nElements{ { (unsigned)sendData.size() } };
    recvCount.resize(context.size());
    static_cast<CommunicationPolicy*>(this)->allGather(context, nElements, recvCount);
    recvData.resize(std::accumulate(recvCount.begin(), recvCount.end(), 0U));

    for (auto const& vAddr : context) {
        events.push_back(
            static_cast<CommunicationPolicy*>(this)->asyncSend(vAddr, 0, context, sendData));
    }

    size_t recvOffset = 0;
    for (auto const& vAddr : context) {
        std::vector<RecvValueType> tmpData(recvCount.at(vAddr));
        static_cast<CommunicationPolicy*>(this)->recv(vAddr, 0, context, tmpData);
        std::memcpy(
            recvData.data() + recvOffset, tmpData.data(), tmpData.size() * sizeof(RecvValueType));
        recvOffset += recvCount.at(vAddr);
    }

    for (unsigned i = 0; i < events.size(); ++i) {
        events.back().wait();
        events.pop_back();
    }
}

template <typename T_CommunicationPolicy>
template <typename T_Send, typename T_Recv>
void Base<T_CommunicationPolicy>::scatter(
    const VAddr rootVAddr, const Context context, const T_Send& sendData, T_Recv& recvData)
{
    using SendValueType = typename T_Recv::value_type;
    using CommunicationPolicy = T_CommunicationPolicy;
    using Event = Base<CommunicationPolicy>::Event;

    std::vector<Event> events;

    if (rootVAddr == context.getVAddr()) {
        for (auto const& vAddr : context) {
            size_t sendOffset = vAddr * recvData.size();
            std::vector<SendValueType> tmpData(
                sendData.begin() + sendOffset, sendData.begin() + sendOffset + recvData.size());
            events.push_back(
                static_cast<CommunicationPolicy*>(this)->asyncSend(vAddr, 0, context, tmpData));
        }
    }

    static_cast<CommunicationPolicy*>(this)->recv(rootVAddr, 0, context, recvData);

    for (unsigned i = 0; i < events.size(); ++i) {
        events.back().wait();
        events.pop_back();
    }
}

template <typename T_CommunicationPolicy>
template <typename T_Send, typename T_Recv>
void Base<T_CommunicationPolicy>::allScatter(
    const Context context, const T_Send& sendData, T_Recv& recvData)
{
    using SendValueType = typename T_Recv::value_type;
    using CommunicationPolicy = T_CommunicationPolicy;
    using Event = Base<CommunicationPolicy>::Event;

    std::vector<Event> events;
    size_t nElementsPerPeer = static_cast<size_t>(recvData.size() / context.size());

    for (auto const& vAddr : context) {
        size_t sendOffset = vAddr * nElementsPerPeer;
        std::vector<SendValueType> tmpData(
            sendData.begin() + sendOffset, sendData.begin() + sendOffset + nElementsPerPeer);
        events.push_back(
            static_cast<CommunicationPolicy*>(this)->asyncSend(vAddr, 0, context, tmpData));
    }

    for (auto const& vAddr : context) {
        size_t recvOffset = vAddr * nElementsPerPeer;
        std::vector<SendValueType> tmpData(nElementsPerPeer);
        static_cast<CommunicationPolicy*>(this)->recv(vAddr, 0, context, tmpData);
        std::memcpy(
            recvData.data() + recvOffset, tmpData.data(), tmpData.size() * sizeof(SendValueType));
    }

    for (unsigned i = 0; i < events.size(); ++i) {
        events.back().wait();
        events.pop_back();
    }
}

template <typename T_CommunicationPolicy>
template <typename T_Send, typename T_Recv, typename T_Op>
void Base<T_CommunicationPolicy>::reduce(
    const VAddr rootVAddr,
    const Context context,
    const T_Op op,
    const T_Send& sendData,
    T_Recv& recvData)
{
    using RecvValueType = typename T_Recv::value_type;
    using CommunicationPolicy = T_CommunicationPolicy;
    using Event = Base<CommunicationPolicy>::Event;

    Event e = static_cast<CommunicationPolicy*>(this)->asyncSend(rootVAddr, 0, context, sendData);

    if (rootVAddr == context.getVAddr()) {
        for (auto const& vAddr : context) {
            std::vector<RecvValueType> tmpData(recvData.size());
            static_cast<CommunicationPolicy*>(this)->recv(vAddr, 0, context, tmpData);

            for (size_t i = 0; i < recvData.size(); ++i) {
                recvData.data()[i] = op(recvData.data()[i], tmpData[i]);
            }
        }
    }

    e.wait();
}

template <typename T_CommunicationPolicy>
template <typename T_Send, typename T_Recv, typename T_Op>
void Base<T_CommunicationPolicy>::allReduce(
    const Context context, T_Op op, const T_Send& sendData, T_Recv& recvData)
{
    using RecvValueType = typename T_Recv::value_type;
    using CommunicationPolicy = T_CommunicationPolicy;
    using Event = Base<CommunicationPolicy>::Event;

    std::vector<Event> events;
    for (auto const& vAddr : context) {
        Event e = static_cast<CommunicationPolicy*>(this)->asyncSend(vAddr, 0, context, sendData);
    }

    for (auto const& vAddr : context) {
        std::vector<RecvValueType> tmpData(recvData.size());
        static_cast<CommunicationPolicy*>(this)->recv(vAddr, 0, context, tmpData);

        for (size_t i = 0; i < recvData.size(); ++i) {
            recvData.data()[i] = op(recvData.data()[i], tmpData[i]);
        }
    }

    for (unsigned i = 0; i < events.size(); ++i) {
        events.back().wait();
        events.pop_back();
    }
}

template <typename T_CommunicationPolicy>
template <typename T_SendRecv>
void Base<T_CommunicationPolicy>::broadcast(
    const VAddr rootVAddr, const Context context, T_SendRecv& data)
{
    using CommunicationPolicy = T_CommunicationPolicy;

    std::vector<Event> events;

    if (rootVAddr == context.getVAddr()) {
        for (auto const& vAddr : context) {
            static_cast<CommunicationPolicy*>(this)->asyncSend(vAddr, 0, context, data);
        }
    }

    static_cast<CommunicationPolicy*>(this)->recv(rootVAddr, 0, context, data);

    for (unsigned i = 0; i < events.size(); ++i) {
        events.back().wait();
        events.pop_back();
    }
}

template <typename T_CommunicationPolicy>
void Base<T_CommunicationPolicy>::synchronize(const Context context)
{
    std::array<char, 0> null;

    if (context.getVAddr() == 0) {
        for (auto const& vAddr : context) {
            static_cast<CommunicationPolicy*>(this)->recv(vAddr, 0, context, null);
        }

        for (auto const& vAddr : context) {
            static_cast<CommunicationPolicy*>(this)->send(vAddr, 0, context, null);
        }

    } else {
        static_cast<CommunicationPolicy*>(this)->send(0, 0, context, null);
        static_cast<CommunicationPolicy*>(this)->recv(0, 0, context, null);
    }
}

} // namespace communicationPolicy

} // namespace graybat
