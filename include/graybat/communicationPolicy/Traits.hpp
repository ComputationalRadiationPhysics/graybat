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

namespace communicationPolicy {

namespace traits {

template <typename T_CommunicationPolicy> struct ContextType;

template <typename T_CommunicationPolicy> struct ContextIDType;

template <typename T_CommunicationPolicy> struct EventType;

template <typename T_CommunicationPolicy> struct StatusType;

template <typename T_CommunicationPolicy> struct ConfigType;

} // namespace traits

template <typename T_CommunicationPolicy> using VAddr = unsigned;

template <typename T_CommunicationPolicy> using Tag = unsigned;

enum class MsgTypeType : std::int8_t {
    VADDR_REQUEST = 0,
    VADDR_LOOKUP = 1,
    DESTRUCT = 2,
    RETRY = 3,
    ACK = 4,
    CONTEXT_INIT = 5,
    CONTEXT_REQUEST = 6,
    PEER = 7,
    CONFIRM = 8,
    SPLIT = 9
};

template <typename T_CommunicationPolicy> using MsgType = MsgTypeType;

template <typename T_CommunicationPolicy> using MsgID = unsigned;

template <typename T_CommunicationPolicy>
using Context = typename traits::ContextType<T_CommunicationPolicy>::type;

template <typename T_CommunicationPolicy>
using ContextID = typename traits::ContextIDType<T_CommunicationPolicy>::type;

template <typename T_CommunicationPolicy>
using Event = typename traits::EventType<T_CommunicationPolicy>::type;

template <typename T_CommunicationPolicy>
using Status = typename traits::StatusType<T_CommunicationPolicy>::type;

template <typename T_CommunicationPolicy>
using Config = typename traits::ConfigType<T_CommunicationPolicy>::type;

} // namespace communicationPolicy

} // namespace graybat
