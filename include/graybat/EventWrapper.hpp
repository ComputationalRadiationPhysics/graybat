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

#include <functional>

namespace graybat {

// Event wrapper for communication policy events
/**
 * This event wraps other events and adds an
 * additional member to it. This is meant to
 * add further data to an event e.g.: shared
 * pointer which should exist until the user
 * remove the event.
 *
 * @tparam T_Event Type that models graybat::concept::Event
 * @tparam T_Addtional Abitraty type (usually something with a shared ptr)
 */
// models graybat::concept::Event
template <typename T_Event> class EventWrapper {
    using Tag = unsigned;
    using VAddr = unsigned;

  public:
    EventWrapper(T_Event event, std::function<void()> const& onReady)
        : event(event)
        , onReady(onReady)
    {
    }

    void wait()
    {
        event.wait();
        onReady();
    }

    bool ready()
    {
        auto isReady = event.ready();
        if (isReady) {
            onReady();
        }
        return isReady;
    }

    VAddr source()
    {
        return event.source();
    }

    Tag getTag()
    {
        return event.getTag;
    }

  private:
    T_Event event;
    std::function<void()> onReady;
};
}