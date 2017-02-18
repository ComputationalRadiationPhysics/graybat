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

#include <boost/optional.hpp>

namespace graybat {

template <class T_Cage> struct CommunicationEdge {

    typedef T_Cage Cage;
    typedef unsigned EdgeID;
    typedef typename Cage::GraphPolicy GraphPolicy;
    typedef typename Cage::Vertex Vertex;
    typedef typename Cage::Event Event;
    typedef typename GraphPolicy::EdgeProperty EdgeProperty;
    typedef typename GraphPolicy::VertexProperty VertexProperty;

    EdgeID id;
    Vertex target;
    Vertex source;
    EdgeProperty& edgeProperty;
    Cage& cage;

    CommunicationEdge(
        const EdgeID id, Vertex source, Vertex target, EdgeProperty& edgeProperty, Cage& cage)
        : id(id)
        , target(target)
        , source(source)
        , edgeProperty(edgeProperty)
        , cage(cage)
    {
    }

    /***************************************************************************
     * Graph Operations
     ****************************************************************************/

    EdgeProperty& operator()()
    {
        return edgeProperty;
    }

    CommunicationEdge inverse()
    {
        boost::optional<CommunicationEdge> edge = cage.getEdge(target, source);
        if (edge) {
            return *edge;
        }
        return *this;
    }

    /***************************************************************************
     * Communication Operations
     ****************************************************************************/

    template <class T_Send> Event operator<<(const T_Send& data)
    {
        std::vector<Event> events;
        cage.send(*this, data, events);
        return events.back();
    }

    template <class T_Recv> void operator>>(T_Recv& data)
    {
        cage.recv(*this, data);
    }
};

} /* namespace graybat */
