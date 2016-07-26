#pragma once

// CLIB
#include <assert.h>   /* assert */
#include <string.h>   /* strup */

// STL
#include <assert.h>   /* assert */
#include <array>      /* array */
#include <iostream>   /* std::cout */
#include <map>        /* std::map */
#include <exception>  /* std::out_of_range */
#include <sstream>    /* std::stringstream, std::istringstream */
#include <string>     /* std::string */

// Boost.Asio
#include <boost/asio.hpp>

// GrayBat
#include <graybat/communicationPolicy/Traits.hpp>
#include <graybat/communicationPolicy/socket/Base.hpp>    /* Base */
#include <graybat/communicationPolicy/socket/Context.hpp> /* Context */
#include <graybat/communicationPolicy/socket/Event.hpp>   /* Event */
#include <graybat/communicationPolicy/asio/Message.hpp>   /* Message */
#include <graybat/communicationPolicy/asio/Config.hpp>    /* Config */

#define CHUNK_SIZE 8096

namespace graybat {

    namespace communicationPolicy {

     /************************************************************************//**
	 * @class Asio
	 *
	 * @brief Implementation of the Cage communicationPolicy interface
	 *        based on Asio.
	 *
	 ***************************************************************************/
        struct Asio;

        namespace traits {

            template<>
            struct ContextType<Asio> {
                using type = graybat::communicationPolicy::socket::Context<Asio>;
            };

            template<>
            struct ContextIDType<Asio> {
                using type = unsigned;
            };

            template<>
            struct EventType<Asio> {
                using type = graybat::communicationPolicy::socket::Event<Asio>;
            };

            template<>
            struct ConfigType<Asio> {
                using type = graybat::communicationPolicy::asio::Config;
            };


        }

        namespace socket {

            namespace traits {

                template<>
                struct UriType<Asio> {
                    using type = std::string;
                };

                template<>
                struct SocketType<Asio> {
                    using type = boost::asio::ip::tcp::socket;
                };

                template<>
                struct MessageType<Asio> {
                    using type = graybat::communicationPolicy::asio::Message<Asio>;
                };

            }

        }

        struct Asio : public graybat::communicationPolicy::socket::Base<Asio> {

            // Type defs
            using Tag        = graybat::communicationPolicy::Tag<Asio>;
            using ContextID  = graybat::communicationPolicy::ContextID<Asio>;
            using MsgID      = graybat::communicationPolicy::MsgID<Asio>;
            using VAddr      = graybat::communicationPolicy::VAddr<Asio>;
            using Context    = graybat::communicationPolicy::Context<Asio>;
            using Event      = graybat::communicationPolicy::Event<Asio>;
            using Config     = graybat::communicationPolicy::Config<Asio>;
            using MsgType    = graybat::communicationPolicy::MsgType<Asio>;
            using Uri        = graybat::communicationPolicy::socket::Uri<Asio>;
            using Socket     = graybat::communicationPolicy::socket::Socket<Asio>;
            using Message    = graybat::communicationPolicy::socket::Message<Asio>;
            using SocketBase = graybat::communicationPolicy::socket::Base<Asio>;

            // Asio Sockets
            boost::asio::io_service io_service;

            Socket signalingSocket;
            Socket recvSocket;
            Socket ctrlSocket;
            std::vector<Socket> sendSockets;
            std::vector<Socket> ctrlSendSockets;


            // Uris
            const Uri peerUri;
            const Uri ctrlUri;

            // Construct
            Asio(Config const config) :
                recvSocket(io_service),
                ctrlSocket(io_service),
                signalingSocket(io_service),
                peerUri(bindToNextFreePort(recvSocket, config.peerUri)),
                ctrlUri(bindToNextFreePort(recvSocket, config.peerUri)),
                SocketBase(config) {

                //std::cout << "PeerUri: " << peerUri << std::endl;
                //std::cout << "CtrlUri: " << ctrlUri << std::endl;
                SocketBase::init();

            }

            // Destructor
            ~Asio() {
                //SocketBase::deinit();
            }

            Asio(Asio &&other) = delete;

            Asio(Asio &other) = delete;


            /***********************************************************************//**
             *
	     * @name Socket base utilities
	     *
	     * @{
	     *
	     ***************************************************************************/

            void createSocketsToPeers() {
                for (unsigned vAddr = 0; vAddr < initialContext.size(); vAddr++) {
                    sendSockets.emplace_back(Socket(io_service));
                }
            }

            template<typename T_Socket>
            void connectToSocket(T_Socket &socket, std::string const uri) {
                std::string baseUri = uri.substr(0, uri.rfind(":")).substr(uri.rfind("//") + 2);
                std::string port = uri.substr(uri.rfind(":") + 1);

                boost::asio::ip::tcp::resolver resolver(io_service);
                boost::asio::ip::tcp::resolver::query url(baseUri, port);
                boost::asio::ip::tcp::resolver::iterator endpoint_iterator = resolver.resolve(url);

                std::cout << "connect to: " << baseUri << ":" << port << std::endl;
                boost::asio::connect(socket, endpoint_iterator);

            }

            Uri bindToNextFreePort(Socket &socket, const std::string peerUri) {
                unsigned peerBasePort = std::stoi(peerUri.substr(peerUri.rfind(":") + 1));
                bool connected = false;

                std::string uri;
                while (!connected) {
                    try {
                        boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::tcp::v4(), peerBasePort);
                        boost::asio::ip::tcp::acceptor acceptor(io_service, endpoint);
                        //acceptor.accept(socket); 
                        connected = true;
                    }
                    catch (...) {
                        //std::cout << e.what() << ". PeerUri \"" << uri << "\". Try to increment port and rebind." << std::endl;
                        peerBasePort++;
                    }

                }

                return uri;

            }

            template<typename T_Socket>
            void recvFromSocket(T_Socket &socket, std::stringstream &ss) {
                const size_t size = ss.str().size();
                std::array<char, CHUNK_SIZE> chunk;
                size_t transferred = 0;

                while (transferred != size) {
                    size_t remaining = size - transferred;
                    size_t read_size = (remaining > CHUNK_SIZE) ? CHUNK_SIZE : remaining;
                    boost::asio::read(socket, boost::asio::buffer(chunk, read_size));
                    ss.write(chunk.data(), read_size);
                    transferred += read_size;
                }

            }

            template<typename T_Socket>
            void recvFromSocket(T_Socket &socket, Message &message) {
                const size_t size = message.size();
                size_t transferred = 0;

                while (transferred != size) {
                    size_t remaining = size - transferred;
                    size_t read_size = (remaining > CHUNK_SIZE) ? CHUNK_SIZE : remaining;
                    boost::asio::read(socket,
                                      boost::asio::buffer(static_cast<void *>(&message.getMessage()[transferred]),
                                                          read_size));
                    transferred += read_size;
                }
            }

            template<typename T_Socket>
            void sendToSocket(T_Socket &socket, std::stringstream &ss) {
                const size_t size = ss.str().size();
                size_t transferred = 0;
                std::array<char, CHUNK_SIZE> chunk;

                while (transferred != size) {
                    size_t remaining = size - transferred;
                    size_t write_size = (remaining > CHUNK_SIZE) ? CHUNK_SIZE : remaining;
                    ss.read(chunk.data(), CHUNK_SIZE);
                    boost::asio::write(socket, boost::asio::buffer(chunk, write_size));
                    transferred += write_size;
                }

            }

            template<typename T_Socket>
            void sendToSocket(T_Socket &socket, std::vector<std::int8_t> &message) {
                const size_t size = message.size();
                size_t transferred = 0;

                while (transferred != size) {
                    size_t remaining = size - transferred;
                    size_t write_size = (remaining > CHUNK_SIZE) ? CHUNK_SIZE : remaining;
                    boost::asio::write(socket,
                                       boost::asio::buffer(static_cast<void *>(&message.data()[transferred]),
                                                           write_size));
                    transferred += write_size;
                }



                //socket.send(data);
            }

        }; // class Asio

    } // namespace communicationPolicy

} // namespace graybat
