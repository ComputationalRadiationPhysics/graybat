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

namespace graybat {
namespace concepts {
// //
// // Cage concepts
// //
//
//    template <T> concept bool ContiguousContainer = requires(T t) {
//      typename T::value_type;
//
//      { t.data() } -> typename T::value_type *;
//      { t.size() } ->std::size_t;
//    }
//
//
//    template <T> concept bool BinaryOperator = requires(T t) {
//      { t(lhs, rhs) } -> decltype(lhs)
//    }
//
//
//    template <T> concept bool DistributionFunctor = requires(T t, unsigned peerID, unsigned contextSize, concept::CommunicationPolicy comm) {
//      { t(peerID, contextSize, comm) } -> std::vector<concept::CommunicationPolicy::Vertex>
//    }
//
//
//    template <T> concept bool GraphFactoryFunctor = requires(T t) {
//      { t() } -> graybat::graphPolicy::GraphDescription
//    }
//
//
// //
// // Policies
// //
//
//    template <typename T>
//    concept bool CommunicationPolicy = requires() {
//      typename graybat::communicationPolicy::Tag<T>;
//      typename graybat::communicationPolicy::VAddr<T>;
//      typename graybat::communicationPolicy::Context<T>;
//      typename graybat::communicationPolicy::Event<T>;
//      typename graybat::communicationPolicy::Config<T>;
//      typename graybat::communicationPolicy::ContextID<T>;
//
//      { t.send(vaddr, tag, context, data) }
//      ->void;
//      { t.asyncSend(vaddr, tag, context, data) }
//      ->CEvent;
//      { t.recv(vaddr, tag, context, data) }
//      ->void;
//      { t.recv(context, data) }
//      ->CEvent;
//      { t.asyncRecv(vaddr, tag, context, data) }
//      ->CEvent;
//
//      { t.gather(vaddr, context, container, data) }
//      ->void;
//      { t.gatherVar(vaddr, context, data, data, recvCount) }
//      ->void;
//      { t.allGather(context, data, data) }
//      ->void;
//      { t.allGatherVar(context, data, data, recvCount) }
//      ->void;
//
//      { t.synchronize(context) }
//      ->void;
//
//      { t.getGlobalContext() }
//      ->CContext;
//      { t.splitContext(b, context) }
//      ->CContext;
//    };
//    }
//
//
//    template <T> concept bool GraphPolicy = requires(T t) {
//        t.getVertices;
//        t.getVertexProperty;
//        t.getEdgeProperty;
//        t.getEdgeSource;
//        t.getEdgeTarget;
//        t.getAdjecentVertices;
//        t.getOutEdges;
//        t.getInEdges;
//    }
//
//
//    template <T> concept bool SerializationPolicy = requires(T t) {
//        t.serialize;
//        t.prepare;
//        t.restore;
//    }
}
}