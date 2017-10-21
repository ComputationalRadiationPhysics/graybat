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
#include <memory> /* std::shared_ptr */
#include <functional> /* std::function */
#include <string> /* std::stoi */
#include <future> /* std::future */

// Boost.Asio
#include <boost/asio.hpp>

// Boost
#include <boost/optional.hpp>

// GRAYBAT
#include <graybat/communicationPolicy/asio/Connection.hpp>

namespace graybat {
namespace communicationPolicy {
namespace asio {

class Endpoint {

  using Callback = std::function<void(const boost::system::error_code&, const std::shared_ptr<Connection>&)>;
  using Future = std::future<const boost::system::error_code>;
  using Port = std::string;
  using Url = std::string;

public:
  Endpoint(boost::asio::io_service &, Port);
  Endpoint(boost::asio::io_service &);

  auto accept() -> std::shared_ptr<Connection>;
  auto asyncAccept(const Callback& = [](const boost::system::error_code&, const std::shared_ptr<Connection> &){}) -> Future;

  auto connect(Url, Port) -> std::shared_ptr<Connection>;
  auto asyncConnect(Url, Port, const Callback& = [](const boost::system::error_code&, const std::shared_ptr<Connection> &){}) -> Future;

  auto bind(Port) -> boost::system::error_code;

public:
    boost::asio::ip::tcp::endpoint localEndpoint_;

private:
  boost::asio::ip::tcp::acceptor acceptor_;
  boost::asio::ip::tcp::resolver resolver_;
  boost::asio::io_service &ioService_;
  std::vector<std::shared_ptr<Connection>> connections_;
};

inline Endpoint::Endpoint(boost::asio::io_service &ioService, Port port)
    : localEndpoint_(boost::asio::ip::tcp::v4(),
                     static_cast<unsigned short>(std::stoi(port))),
      acceptor_(ioService), resolver_(ioService), ioService_(ioService) {
        bind(port);
}

inline Endpoint::Endpoint(boost::asio::io_service &ioService)
    : acceptor_(ioService), resolver_(ioService), ioService_(ioService) {
}

inline auto Endpoint::asyncAccept(const Callback &onAccepted) -> Future {
  std::shared_ptr<std::promise<const boost::system::error_code>> accepted =
      std::make_shared<std::promise<const boost::system::error_code>>();
  std::shared_ptr<Connection> connection =
      std::make_shared<Connection>(ioService_);

  connections_.push_back(connection);
  acceptor_.async_accept(
      connections_.back()->socket_, connections_.back()->endpoint_,
      [this, &onAccepted, accepted, connection](const boost::system::error_code &error) {
          accepted->set_value(error);
          onAccepted(error, connection);
          asyncAccept(onAccepted);
      });
    return accepted->get_future();
}

inline auto Endpoint::accept() -> std::shared_ptr<Connection> {
  std::shared_ptr<Connection> connection = std::make_shared<Connection>(ioService_);
  boost::system::error_code error;
  acceptor_.accept(connection->socket_, connection->endpoint_, error);

  if (error) {
      return nullptr;
  }

  connections_.push_back(connection);
  return connection;
}

inline auto Endpoint::connect(std::string url, std::string port)
    -> std::shared_ptr<Connection> {

  std::shared_ptr<Connection> connection = std::make_shared<Connection>(ioService_);
  boost::asio::ip::tcp::resolver::iterator begin =
      resolver_.resolve({url, port});
  boost::asio::ip::tcp::resolver::iterator end;

  boost::system::error_code error;
  boost::asio::ip::tcp::resolver::iterator iter =
      boost::asio::connect(connection->socket_, begin, error);

  if (error) {
    return nullptr;
  }

  connection->endpoint_ = iter->endpoint();
  connections_.push_back(connection);
  return connection;
}

inline auto Endpoint::asyncConnect(Url url, Port port,
                                   const Callback &onConnected) -> Future {

  std::shared_ptr<std::promise<const boost::system::error_code>> connected =
      std::make_shared<std::promise<const boost::system::error_code>>();

  std::shared_ptr<Connection> connection =
      std::make_shared<Connection>(ioService_);

  {
    boost::asio::ip::tcp::resolver::iterator begin =
        resolver_.resolve({url, port});
    boost::asio::ip::tcp::resolver::iterator end;

    boost::system::error_code error;
    boost::asio::async_connect(
        connection->socket_, begin,
        [connected, connection,
         onConnected](const boost::system::error_code &error,
                      boost::asio::ip::tcp::resolver::iterator iter) {

          if (!error) {
            connection->endpoint_ = iter->endpoint();
          }
          connected->set_value(error);
          onConnected(error, connection);
        });
  }

  return connected->get_future();
}

inline auto Endpoint::bind(Port port) -> boost::system::error_code {
  boost::asio::ip::tcp::endpoint endpoint{
      boost::asio::ip::tcp::v4(), static_cast<unsigned short>(std::stoi(port))};
  boost::system::error_code error;

  acceptor_.open(endpoint.protocol());
  acceptor_.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
  while (acceptor_.bind(endpoint, error) ==
         boost::system::errc::address_in_use) {
    endpoint.port(endpoint.port() + 1);
  }

  if (!error) {
    acceptor_.listen();
    localEndpoint_ = endpoint;
  }

  return error;
}
}
}
} /* namespace graybat::communicationPolicy::asio */