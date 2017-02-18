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

namespace socket {

namespace traits {

template <typename T_CommunicationPolicy> struct UriType;

template <typename T_CommunicationPolicy> struct SocketType;

template <typename T_CommunicationPolicy> struct MessageType;

} // namespace traits

template <typename T_CommunicationPolicy>
using Uri = typename traits::UriType<T_CommunicationPolicy>::type;

template <typename T_CommunicationPolicy>
using Socket = typename traits::SocketType<T_CommunicationPolicy>::type;

template <typename T_CommunicationPolicy>
using Message = typename traits::MessageType<T_CommunicationPolicy>::type;

template <typename T_CommunicationPolicy> using ContextName = std::string;

} // namespace socket

} // namespace communicationPolicy

} // namespace graybat
