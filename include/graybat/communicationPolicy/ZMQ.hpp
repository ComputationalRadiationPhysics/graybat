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
#include <string.h> /* strup */

// STL
#include <array>     /* array */
#include <assert.h>  /* assert */
#include <exception> /* std::out_of_range */
#include <iostream>  /* std::cout */
#include <map>       /* std::map */
#include <sstream>   /* std::stringstream, std::istringstream */
#include <string>    /* std::string */

// ZMQ
#include <zmq.hpp> /* zmq::socket_t, zmq::context_t */

// GrayBat
#include <graybat/communicationPolicy/Traits.hpp>
#include <graybat/communicationPolicy/socket/Base.hpp> /* Base */
#include <graybat/communicationPolicy/zmq/Config.hpp>  /* Config */
#include <graybat/communicationPolicy/zmq/Context.hpp> /* Context */
#include <graybat/communicationPolicy/zmq/Event.hpp>   /* Event */
#include <graybat/communicationPolicy/zmq/Message.hpp> /* Message */
#include <graybat/communicationPolicy/zmq/Status.hpp>  /* Event */

namespace graybat {
namespace communicationPolicy {

/************************************************************************/ /**
  * @class ZMQ
  *
  * @brief Implementation of the Cage communicationPolicy interface
  *        based on ZMQ.
  *
  ***************************************************************************/
struct ZMQ;

namespace traits {

template <> struct ContextType<ZMQ> {
  using type = graybat::communicationPolicy::zmq::Context<ZMQ>;
};

template <> struct ContextIDType<ZMQ> { using type = unsigned; };

template <> struct EventType<ZMQ> {
  using type = graybat::communicationPolicy::zmq::Event<ZMQ>;
};

template <> struct StatusType<ZMQ> {
  using type = graybat::communicationPolicy::zmq::Status<ZMQ>;
};

template <> struct ConfigType<ZMQ> {
  using type = graybat::communicationPolicy::zmq::Config;

};
}

namespace socket {
namespace traits {

template <> struct UriType<ZMQ> { using type = std::string; };

template <> struct SocketType<ZMQ> { using type = ::zmq::socket_t; };

template <> struct MessageType<ZMQ> {
  using type = graybat::communicationPolicy::zmq::Message<ZMQ>;
};

}
}

struct ZMQ : public graybat::communicationPolicy::socket::Base<ZMQ> {

  // Type defs
  using Tag = graybat::communicationPolicy::Tag<ZMQ>;
  using ContextID = graybat::communicationPolicy::ContextID<ZMQ>;
  using MsgID = graybat::communicationPolicy::MsgID<ZMQ>;
  using VAddr = graybat::communicationPolicy::VAddr<ZMQ>;
  using Context = graybat::communicationPolicy::Context<ZMQ>;
  using Event = graybat::communicationPolicy::Event<ZMQ>;
  using Config = graybat::communicationPolicy::Config<ZMQ>;
  using MsgType = graybat::communicationPolicy::MsgType<ZMQ>;
  using Uri = graybat::communicationPolicy::socket::Uri<ZMQ>;
  using Socket = graybat::communicationPolicy::socket::Socket<ZMQ>;
  using Message = graybat::communicationPolicy::socket::Message<ZMQ>;
  using SocketBase = graybat::communicationPolicy::socket::Base<ZMQ>;

  // ZMQ Sockets
  ::zmq::context_t zmqContext;
  Socket recvSocket;
  Socket ctrlSocket;
  std::vector<Socket> sendSockets;
  std::vector<Socket> ctrlSendSockets;

  // Uri
  const Uri peerUri;
  const Uri ctrlUri;

  // Construct
  ZMQ(Config const config)
      : SocketBase(config)
      , zmqContext(1)
      , recvSocket(zmqContext, ZMQ_PULL)
      , ctrlSocket(zmqContext, ZMQ_PULL)
      , peerUri(bindToNextFreePort(recvSocket, config.peerUri))
      , ctrlUri(bindToNextFreePort(ctrlSocket, config.peerUri))
  {

      //std::cout << "--> ZMQ" << std::endl;
      SocketBase::init();
      //std::cout << "<-- ZMQ" << std::endl;
  }

  // Copy constructor
  ZMQ(ZMQ &) = delete;
  // Copy assignment constructor
  ZMQ &operator=(ZMQ &) = delete;
  // Move constructor
  ZMQ(ZMQ &&other) = delete;
  // Move assignment constructor
  ZMQ &operator=(ZMQ &&) = delete;

  // Destructor
  ~ZMQ() { SocketBase::deinit(); }

  /***********************************************************************/ /**
    *
    * @name Socket base utilities
    *
    * @{
    *
    ***************************************************************************/

  void createSocketsToPeers() {
    for (auto const &vAddr : initialContext) {
      (void)vAddr;
      sendSockets.emplace_back(Socket(zmqContext, ZMQ_PUSH));
      ctrlSendSockets.emplace_back(Socket(zmqContext, ZMQ_PUSH));
    }
  }

  template <typename T_Socket>
  void connectToSocket(T_Socket &socket, std::string const uri) {
    socket.connect(uri.c_str());
  }

  template <typename T_Socket>
  void recvFromSocket(T_Socket &socket, std::stringstream &ss) {
    ::zmq::message_t message;
    socket.recv(&message);
    ss << static_cast<char *>(message.data());
  }

  template <typename T_Socket>
  void recvFromSocket(T_Socket &socket, Message &message) {
    socket.recv(&message.getMessage());
  }

  template <typename T_Socket>
  void sendToSocket(T_Socket &socket, std::stringstream const &ss) {
    std::string string = ss.str();
    ::zmq::message_t message(sizeof(char) * string.size());
    memcpy(static_cast<char *>(message.data()), string.data(),
           sizeof(char) * string.size());
    socket.send(message);
  }

  template <typename T_Socket>
  void sendToSocket(T_Socket &socket, ::zmq::message_t &data) {
    socket.send(data);
  }

  Uri bindToNextFreePort(Socket& socket, const std::string peerUri)
  {
      std::string peerBaseUri = peerUri.substr(0, peerUri.rfind(":"));
      unsigned peerBasePort = std::stoi(peerUri.substr(peerUri.rfind(":") + 1));
      bool connected = false;

      std::string uri;
      while (!connected) {
          try {
              uri = peerBaseUri + ":" + std::to_string(peerBasePort);
              socket.bind(uri.c_str());
              connected = true;
          } catch (::zmq::error_t e) {
              // std::cout << e.what() << ". PeerUri \"" << uri << "\". Try to
              // increment port and rebind." << std::endl;
              peerBasePort++;
          }
      }

      return uri;
  }

}; // class ZMQ
} // namespace communicationPolicy
} // namespace graybat
