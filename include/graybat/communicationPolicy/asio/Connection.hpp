/**
 * Copyright 2017 Erik Zenker
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

// STL
#include <cstdint> // std::int8_t
#include <cstdlib> // EXIT_FAILURE, std::exit
#include <future> // std::future
#include <iostream> // std::cout, std::cerr
#include <memory> // std::shared_ptr
#include <sstream> // std::stringstream

// Boost.Asio
#include <boost/asio.hpp>

namespace graybat {
namespace communicationPolicy {
namespace asio {

class Connection {

    using Callback = std::function<void(const boost::system::error_code&, std::size_t)>;
    using Future = std::future<const boost::system::error_code>;

  public:
    Connection(boost::asio::io_service&);

  public: // Blocking
    auto receive(std::vector<std::int8_t>&) -> boost::system::error_code;
    auto streamReceive(std::stringstream&) -> boost::system::error_code;
    auto receivePrefixSize(std::vector<std::int8_t>&) -> boost::system::error_code ;
    template <typename T_Buf>
    auto send(T_Buf&) -> boost::system::error_code ;


  public: // Nonblocking
    auto asyncReceive(
        std::vector<std::int8_t>&, Callback = [](const boost::system::error_code&, std::size_t) {})
        -> Future;

    template <typename T_Buf>
    auto
    asyncSend(const T_Buf&, const Callback& = [](const boost::system::error_code&, std::size_t) {})
        -> Future;

    auto asyncStreamSend(
        const std::stringstream&,
        const Callback& = [](const boost::system::error_code&, std::size_t) {}) -> Future;

    auto asyncSendPrefixSize(
        const std::vector<std::int8_t>&,
        const Callback& = [](const boost::system::error_code&, std::size_t) {}) -> Future;

  public:
    boost::asio::ip::tcp::socket socket_;
    boost::asio::ip::tcp::endpoint endpoint_;
};

inline Connection::Connection(boost::asio::io_service& ioService)
    : socket_(ioService)
{
}

inline auto Connection::asyncReceive(std::vector<std::int8_t>& buffer, Callback onReceived)
    -> Future
{
    std::shared_ptr<std::promise<const boost::system::error_code>> received
        = std::make_shared<std::promise<const boost::system::error_code>>();
    boost::asio::async_read(
        socket_,
        boost::asio::buffer(buffer),
        [onReceived,
         received](const boost::system::error_code& error, std::size_t bytesTransferred) {
            received->set_value(error);
            onReceived(error, bytesTransferred);
        });
    return received->get_future();
}

inline boost::system::error_code Connection::receive(std::vector<std::int8_t>& buffer)
{
    boost::system::error_code error;
    boost::asio::read(socket_, boost::asio::buffer(buffer), error);
    return error;
}

inline auto Connection::receivePrefixSize(std::vector<std::int8_t>& payloadBuffer)
    -> boost::system::error_code
{
    std::array<std::int8_t, 8> headerBuffer;
    std::stringstream headerStream;
    boost::system::error_code error;
    uint64_t maxLength = 0;
    uint64_t remaining = 0;

    // Receive header
    if (!socket_.read_some(boost::asio::buffer(headerBuffer), error)) {
        return error;
    }
    std::copy(headerBuffer.begin(), headerBuffer.end(), std::ostream_iterator<char>(headerStream));
    headerStream >> maxLength;
    remaining = maxLength;

    // Receive payload
    payloadBuffer.resize(remaining);
    if (!boost::asio::read(socket_, boost::asio::buffer(payloadBuffer), error)) {
        return error;
    }
    return {};
}

inline auto Connection::streamReceive(std::stringstream& payloadStream) -> boost::system::error_code
{
    std::vector<std::int8_t> buffer;
    auto error = receivePrefixSize(buffer);
    std::copy(buffer.begin(), buffer.end(), std::ostream_iterator<std::int8_t>(payloadStream));
    return error;
}

template <typename T_Buf>
inline boost::system::error_code Connection::send(T_Buf& buffer)
{
    boost::system::error_code error;
    boost::asio::write(socket_, boost::asio::buffer(buffer.data(), buffer.size()), error);
    return error;
}

template <typename T_Buf>
auto Connection::asyncSend(const T_Buf& buffer, const Callback& onSent) -> Future
{
    std::shared_ptr<std::promise<const boost::system::error_code>> sent
        = std::make_shared<std::promise<const boost::system::error_code>>();
    boost::asio::async_write(
        socket_,
        boost::asio::buffer(buffer.data(), buffer.size()),
        [onSent, sent](const boost::system::error_code& error, std::size_t transferedBytes) {
            sent->set_value(error);
            onSent(error, transferedBytes);
        });
    return sent->get_future();
}

auto Connection::asyncSendPrefixSize(
    const std::vector<std::int8_t>& payloadBuffer, const Callback& onSent) -> Future
{
    auto prefixedPayload = std::make_shared<std::vector<uint8_t>>(payloadBuffer.size() + 8, 0);
    std::copy(payloadBuffer.begin(), payloadBuffer.end(), prefixedPayload->begin() + 8);

    std::string sizeString = std::to_string(payloadBuffer.size());
    std::copy(sizeString.begin(), sizeString.end(), prefixedPayload->begin());

    return asyncSend(
        *prefixedPayload.get(),
        [prefixedPayload,
         onSent](const boost::system::error_code& ec, std::size_t transferedBytes) {
            onSent(ec, transferedBytes);
        });
}

auto Connection::asyncStreamSend(const std::stringstream& payloadStream, const Callback& onSent)
    -> Future
{
    std::string payloadString = payloadStream.str();
    auto payloadBuffer = std::make_shared<std::vector<std::int8_t>>(payloadString.size(), 0);
    std::copy(payloadString.begin(), payloadString.end(), payloadBuffer->begin());
    return asyncSendPrefixSize(
        *payloadBuffer.get(),
        [payloadBuffer, onSent](const boost::system::error_code& ec, std::size_t transferedBytes) {
            onSent(ec, transferedBytes);
        });
}

} /* asio */
} /* communicationPolicy */
} /* graybat */