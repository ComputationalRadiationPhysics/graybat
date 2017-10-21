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

// BOOST
#include <boost/test/unit_test.hpp>
#include <boost/hana/tuple.hpp>

// STL
#include <vector>     // std::vector
#include <future>     // std::promise
#include <chrono>     // std::chrono::milliseconds

// GRAYBAT
#include <graybat/communicationPolicy/asio/Endpoint.hpp>
#include <graybat/communicationPolicy/asio/Connection.hpp>

const std::chrono::milliseconds successTimeout{1500};

/*******************************************************************************
 * Test Suites
 *******************************************************************************/
BOOST_AUTO_TEST_SUITE( endpoint )
using namespace graybat::communicationPolicy::asio;


/***************************************************************************
 * Test Cases
 ****************************************************************************/
BOOST_AUTO_TEST_CASE(shouldCreateEndpoint) {
  boost::asio::io_service io_service;
  Endpoint ep(io_service, "8888");
}

BOOST_AUTO_TEST_CASE(shouldRefuseConnection) {
  boost::asio::io_service io_service;
  Endpoint ep(io_service, "8888");

  BOOST_REQUIRE(!ep.connect("127.0.0.1", "9999"));
}

BOOST_AUTO_TEST_CASE(shouldBindAndConnect) {
  boost::asio::io_service io_service;
  Endpoint ep1(io_service);
  Endpoint ep2(io_service);
  ep1.bind("8888");
  ep2.bind("9999");

  BOOST_REQUIRE(ep1.connect("127.0.0.1", "9999"));
}

BOOST_AUTO_TEST_CASE(shouldRebindAndConnect) {
  boost::asio::io_service io_service;
  Endpoint ep1(io_service);
  Endpoint ep2(io_service);
  ep1.bind("8888");
  ep2.bind("8888");

  BOOST_REQUIRE(ep1.connect("127.0.0.1", "8889"));
}

BOOST_AUTO_TEST_CASE(shouldConnect) {
  boost::asio::io_service io_service;
  Endpoint ep1(io_service, "8888");
  Endpoint ep2(io_service, "9999");

  BOOST_REQUIRE(ep1.connect("127.0.0.1", "9999"));
}

BOOST_AUTO_TEST_CASE(shouldAsynchronousConnect) {
  boost::asio::io_service io_service;
  Endpoint ep1(io_service, "8888");
  Endpoint ep2(io_service, "9999");

  auto connected = ep1.asyncConnect("127.0.0.1", "9999");
  ep2.accept();
  io_service.run_one();

  BOOST_REQUIRE(connected.wait_for(successTimeout) == std::future_status::ready);
  BOOST_REQUIRE(!connected.get());
}

BOOST_AUTO_TEST_CASE(shouldTimeoutOnAsynchronousConnect) {
  boost::asio::io_service io_service;
  Endpoint ep1(io_service, "8888");
  Endpoint ep2(io_service, "9999");

  auto connected = ep1.asyncConnect("127.0.0.1", "9999");
  io_service.run_one();

  BOOST_REQUIRE(connected.wait_for(successTimeout) ==
              std::future_status::timeout);
}

BOOST_AUTO_TEST_CASE(shouldRefuseAsynchronousConnection) {
  boost::asio::io_service io_service;
  Endpoint ep1(io_service, "8888");

  auto connected = ep1.asyncConnect("127.0.0.1", "9999");
  io_service.run_one();

  BOOST_REQUIRE(connected.wait_for(successTimeout) == std::future_status::ready);
  BOOST_REQUIRE(connected.get());
}

BOOST_AUTO_TEST_CASE(shouldAcceptConnection) {
  boost::asio::io_service io_service;

  Endpoint ep1(io_service, "8888");
  Endpoint ep2(io_service, "9999");

  auto ep1ToEp2 = ep1.connect("127.0.0.1", "9999");
  auto ep2ToEp1 = ep2.accept();
  BOOST_REQUIRE(ep1ToEp2);
  BOOST_REQUIRE(ep2ToEp1);
}

BOOST_AUTO_TEST_CASE(shouldAsynchronousAcceptConnection) {
  boost::asio::io_service io_service;

  Endpoint ep1(io_service, "8888");
  Endpoint ep2(io_service, "9999");

  std::shared_ptr<Connection> ep2ToEp1;
  auto ep1ToEp2 = ep1.connect("127.0.0.1", "9999");
  auto accepted = ep2.asyncAccept([&ep2ToEp1](
      const boost::system::error_code &,
      const std::shared_ptr<Connection> &connection) { ep2ToEp1 = connection; });

  io_service.run_one();

  BOOST_REQUIRE(accepted.wait_for(successTimeout) == std::future_status::ready);
  BOOST_REQUIRE(ep1ToEp2);
  BOOST_REQUIRE(ep2ToEp1);
}

BOOST_AUTO_TEST_CASE(shouldAsynchronousConnectAndAsynchronousAccept) {
  boost::asio::io_service io_service;

  Endpoint ep1(io_service, "8888");
  Endpoint ep2(io_service, "9999");

  std::shared_ptr<Connection> ep1ToEp2;
  std::shared_ptr<Connection> ep2ToEp1;
  auto connected = ep1.asyncConnect(
      "127.0.0.1", "9999",
      [&ep1ToEp2](const boost::system::error_code&,
                  const std::shared_ptr<Connection>& connection) {
        ep1ToEp2 = connection;
      });
  auto accepted = ep2.asyncAccept(
      [&ep2ToEp1](const boost::system::error_code &,
                  const std::shared_ptr<Connection>& connection) {
        ep2ToEp1 = connection;
      });

  io_service.run_one();
  io_service.run_one();

  BOOST_REQUIRE(connected.wait_for(successTimeout) == std::future_status::ready);
  BOOST_REQUIRE(accepted.wait_for(successTimeout) == std::future_status::ready);
  BOOST_REQUIRE(ep1ToEp2);
}

BOOST_AUTO_TEST_CASE(shouldReceive) {
  boost::asio::io_service io_service;

  Endpoint ep1(io_service, "8888");
  Endpoint ep2(io_service, "9999");

  auto ep1ToEp2 = ep1.connect("127.0.0.1", "9999");
  auto ep2ToEp1 = ep2.accept();

  std::vector<std::int8_t> data1(1, 1);
  std::vector<std::int8_t> data2(1, 0);
  auto sent = ep1ToEp2->asyncSend(data1);
  io_service.run_one();

  ep2ToEp1->receive(data2);

  BOOST_REQUIRE(sent.wait_for(successTimeout) == std::future_status::ready);
  BOOST_REQUIRE(!sent.get());

  for (auto elem : data2) {
    BOOST_REQUIRE_EQUAL(elem, 1);
  }
}

BOOST_AUTO_TEST_CASE(shouldAsynchronousReceive) {
  boost::asio::io_service io_service;

  Endpoint ep1(io_service, "8888");
  Endpoint ep2(io_service, "9999");

  auto ep1ToEp2 = ep1.connect("127.0.0.1", "9999");
  auto ep2ToEp1 = ep2.accept();

  std::vector<std::int8_t> data1(1, 1);
  std::vector<std::int8_t> data2(1, 0);
  auto sent = ep1ToEp2->asyncSend(data1);
  auto received = ep2ToEp1->asyncReceive(data2);
  io_service.run_one();
  io_service.run_one();

  BOOST_REQUIRE(sent.wait_for(successTimeout) == std::future_status::ready);
  BOOST_REQUIRE(received.wait_for(successTimeout) == std::future_status::ready);
  BOOST_REQUIRE(!sent.get());
  BOOST_REQUIRE(!received.get());

  for (auto elem : data2) {
    BOOST_REQUIRE_EQUAL(elem, 1);
  }
}

BOOST_AUTO_TEST_CASE(shouldAsynchronousDoAll) {
  boost::asio::io_service io_service;

  Endpoint ep1(io_service, "8888");
  Endpoint ep2(io_service, "9999");

  std::shared_ptr<Connection> ep1ToEp2;
  std::shared_ptr<Connection> ep2ToEp1;
  auto connected = ep1.asyncConnect(
      "127.0.0.1", "9999",
      [&ep1ToEp2](const boost::system::error_code &,
                  const std::shared_ptr<Connection> &connection) {
        ep1ToEp2 = connection;
      });
  auto accepted = ep2.asyncAccept(
      [&ep2ToEp1](const boost::system::error_code &,
                  const std::shared_ptr<Connection> &connection) {
        ep2ToEp1 = connection;
      });

  io_service.run_one();
  io_service.run_one();

  BOOST_REQUIRE(connected.wait_for(successTimeout) == std::future_status::ready);
  BOOST_REQUIRE(accepted.wait_for(successTimeout) == std::future_status::ready);
  BOOST_REQUIRE(ep1ToEp2);
  BOOST_REQUIRE(ep2ToEp1);

  std::vector<std::int8_t> data1(100, 'P');
  std::vector<std::int8_t> data2(100, 0);
  auto received = ep2ToEp1->asyncReceive(data2);
  auto sent = ep1ToEp2->asyncSend(data1);

  io_service.run_one();
  io_service.run_one();

  BOOST_REQUIRE(sent.wait_for(successTimeout) == std::future_status::ready);
  BOOST_REQUIRE(received.wait_for(successTimeout) == std::future_status::ready);
  BOOST_REQUIRE(!sent.get());
  BOOST_REQUIRE(!received.get());

  for (auto elem : data2) {
      BOOST_REQUIRE_EQUAL(elem, 'P');
  }
}

BOOST_AUTO_TEST_CASE(shouldReceiveStreamData)
{
    std::stringstream send;
    std::stringstream recv;

    boost::asio::io_service io_service;

    Endpoint ep1(io_service, "8888");
    Endpoint ep2(io_service, "9999");

    auto ep1ToEp2 = ep1.connect("127.0.0.1", "9999");
    auto ep2ToEp1 = ep2.accept();

    send << "42";
    auto sent = ep1ToEp2->asyncStreamSend(send);
    io_service.run_one();

    ep2ToEp1->streamReceive(recv);

    BOOST_REQUIRE(sent.wait_for(successTimeout) == std::future_status::ready);
    BOOST_REQUIRE(!sent.get());

    BOOST_REQUIRE_EQUAL(recv.str(), "42");

}

BOOST_AUTO_TEST_CASE(shouldReceivePrefixSizeData)
{
    boost::asio::io_service io_service;

    Endpoint ep1(io_service, "8888");
    Endpoint ep2(io_service, "9999");

    auto ep1ToEp2 = ep1.connect("127.0.0.1", "9999");
    auto ep2ToEp1 = ep2.accept();

    std::vector<std::int8_t> data1(100, 'P');
    std::vector<std::int8_t> data2(100, 0);
    auto sent = ep1ToEp2->asyncSendPrefixSize(data1);
    io_service.run_one();
    ep2ToEp1->receivePrefixSize(data2);

    BOOST_REQUIRE(sent.wait_for(successTimeout) == std::future_status::ready);
    BOOST_REQUIRE(!sent.get());

    for (auto elem : data2) {
        BOOST_REQUIRE_EQUAL(elem, 'P');
    }
}

BOOST_AUTO_TEST_CASE(shouldReceiveFromDifferentThread)
{
    boost::asio::io_service io_service;

    Endpoint ep1(io_service, "8888");
    Endpoint ep2(io_service, "9999");

    auto ep1ToEp2 = ep1.connect("127.0.0.1", "9999");
    auto ep2ToEp1 = ep2.accept();

    std::vector<std::int8_t> data1(100, 'P');
    std::vector<std::int8_t> data2(100, 0);
    auto sent = ep1ToEp2->asyncSendPrefixSize(data1);
    io_service.run_one();
    std::thread t([&](){
            ep2ToEp1->receivePrefixSize(data2);});

    BOOST_REQUIRE(sent.wait_for(successTimeout) == std::future_status::ready);
    BOOST_REQUIRE(!sent.get());

    t.join();

    for (auto elem : data2) {
        BOOST_REQUIRE_EQUAL(elem, 'P');
    }
}

BOOST_AUTO_TEST_SUITE_END()
