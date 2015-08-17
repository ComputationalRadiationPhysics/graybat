#pragma once

// STL
#include <assert.h>   /* assert */
#include <array>      /* array */
#include <numeric>    /* std::accumulate */
#include <iostream>   /* std::cout */
#include <map>        /* std::map */
#include <exception>  /* std::out_of_range */
#include <sstream>    /* std::stringstream, std::istringstream */
#include <algorithm>  /* std::transform, std::copy */
#include <thread>     /* std::thread */
#include <cstdlib>    /* std::env */
#include <string>     /* std::string, std::stoi */
#include <queue>      /* std::queue */
#include <utility>    /* std::move */

// Boost
#include <boost/any.hpp>

// CLIB
#include <assert.h>   /* assert */
#include <string.h>   /* strup */


// ZMQ
#include <zmq.hpp>

// GrayBat
#include <utils.hpp>  /* utils::MultiKeyMap */

namespace graybat {
    
    namespace communicationPolicy {
    
	/************************************************************************//**
	 * @class ZMQ
	 *
	 * @brief Implementation of the Cage communicationPolicy interface
	 *        based on ZMQ.
	 *
	 ***************************************************************************/
	struct ZMQ {
            
	    // Type defs
	    typedef unsigned Tag;                                            
	    typedef unsigned ContextID;
	    typedef unsigned VAddr;
            typedef unsigned MsgType;
	    typedef std::string Uri;

            
	    /**
	     * @brief A context represents a set of peers which are
	     *        able to communicate with each other.
	     *
	     */
	    class Context {
		typedef unsigned ContextID;
		typedef unsigned VAddr;
	    
	    public:
		Context() :
		    isValid(false),
                    vAddr(0),
                    nPeers(1){

		}

		Context(ContextID contextID, VAddr vAddr, unsigned nPeers) : 
		    isValid(true),
                    contextID(contextID),
                    vAddr(vAddr),
                    nPeers(nPeers){
		
		}

		// Context& operator=(const Context& otherContext){
		//     isValid       = otherContext.valid();
		//     return *this;

		// }

		size_t size() const{
                    return nPeers;
		}

		VAddr getVAddr() const {
                    return vAddr;
		}

                ContextID getID() const {
                    return 0;
                }

		bool valid() const{
		    return isValid;
		}

	    private:	
                bool  isValid;
                ContextID contextID;
                VAddr vAddr;
                unsigned nPeers;
	    };

	    /**
	     * @brief An event is returned by non-blocking 
	     *        communication operations and can be 
	     *        asked whether an operation has finished
	     *        or it can be waited for this operation to
	     *        be finished.
	     *
	     */
	    class Event {
	    public:
                
		Event(VAddr vAddr, Tag tag) : vAddr(vAddr), tag(tag) {
		}

		~Event(){
		}

		void wait(){

		}

		bool ready(){
                    return true;
		}

                Tag getTag(){
                    return tag;

                }

                VAddr vAddr;
                Tag   tag;

                
	    };


            struct message {

                message(const int srcUri) :
                    srcUri(srcUri){

                }

                const int srcUri;
                        
            };
                

            static const MsgType VADDR_REQUEST = 0;
            static const MsgType VADDR_LOOKUP  = 1;
            static const MsgType DESTRUCT      = 2;
            static const MsgType RETRY         = 3;
            static const MsgType ACK           = 4;
            
            // Members
            Context initialContext;
            std::map<VAddr, Uri> phoneBook;
            zmq::context_t context;
            zmq::socket_t recvSocket;
            std::thread connectionManager;
            bool isMaster;
            Uri uri;
            std::map<VAddr, zmq::socket_t> sendSockets;

            utils::MultiKeyMap<std::queue<zmq::message_t>, ContextID, VAddr, Tag> inBox;
            
            // Const Members
            const std::string masterUri;
            
            // Constructor
	    ZMQ() :
                context(1),
                recvSocket(context, ZMQ_PULL),
                isMaster(false),
                masterUri("tcp://127.0.0.1:5000"){

                const unsigned nPeers         = std::stoi(std::getenv("OMPI_COMM_WORLD_SIZE"));
                const unsigned peerID         = std::stoi(std::getenv("OMPI_COMM_WORLD_RANK"));

                // Start thread that manages peers
                if(peerID == 0){
                    isMaster = true;
                    connectionManager = std::thread(&ZMQ::managePeers, this);
                }

                // Create socket for incoming connections
                {

                    // Retrieve and reserve own uri
                    const Uri localBaseUri   = getLocalUri();
                    const unsigned localPort = searchBindPort(localBaseUri, 5001, recvSocket);
                    uri                      = localBaseUri + ":" + std::to_string(localPort);
                    
                    // Retrieve vAddr from master
                    // --> Get own phonebook adress
                    {
                        VAddr vAddr(0);
                        
                        zmq::context_t context(1);
                        zmq::socket_t  socket (context, ZMQ_REQ);
                        socket.connect(masterUri.c_str());

                        // Send vAddr request
                        std::stringstream ss;
                        ss << VADDR_REQUEST << " " << uri;
                        s_send(socket, ss.str().c_str());

                        // Recv vAddr
                        std::stringstream sss;
                        sss << s_recv(socket);
                        sss >> vAddr;

                        initialContext = Context(0, vAddr, nPeers);
                        
                    }

                    // Ask master for uris of other peers
                    // --> Fill phonebook with information
                    {
                        zmq::context_t context(1);
                        zmq::socket_t  socket (context, ZMQ_REQ);
                        socket.connect(masterUri.c_str());

                        for(unsigned vAddr = 0; vAddr < nPeers; vAddr++){

                            MsgType type = RETRY;
                            
                            while(type == RETRY){
                                // Send vAddr lookup
                                std::stringstream ss;
                                ss << VADDR_LOOKUP << " " << vAddr;
                                s_send(socket, ss.str().c_str());

                                // Recv uri
                                std::string uri;
                                std::stringstream sss;
                                sss << s_recv(socket);
                                sss >> type;
                                if(type == ACK){
                                    sss >> uri;   
                                    phoneBook[vAddr] = uri;
                                    break;
                                }

                            }
                            //std::cout << "[" << initialContext.getVAddr() << "] " << vAddr << ":" << phoneBook[vAddr] << std::endl;

                        }

                    }

                    // Create sockets to the other peers
                    {
                        for(unsigned vAddr = 0; vAddr < nPeers; vAddr++){
                            sendSockets.emplace(vAddr, zmq::socket_t(context, ZMQ_PUSH) );
                            sendSockets.at(vAddr).connect(phoneBook.at(vAddr).c_str());
                        }
                        
                    }
                    
                }

	    }

	    // Destructor
	    ~ZMQ(){
                zmq::context_t context(1);
                zmq::socket_t  socket (context, ZMQ_REQ);
                socket.connect(masterUri.c_str());
                s_send(socket, std::to_string(DESTRUCT).c_str());
                if(isMaster){
                    connectionManager.join();
                }
	    }

	    /***********************************************************************//**
             *
	     * @name ZMQ Utility functions
	     *
	     * @{
	     *
	     ***************************************************************************/
            
            
            std::string getLocalUri() const{
                return std::string("tcp://127.0.0.1");
            }

            unsigned searchBindPort(const std::string localBaseUri, const unsigned localBasePort, zmq::socket_t &socket){
                bool connected = false;

                unsigned port = localBasePort;
                
                while(!connected){
                    try {
                        std::string uri = localBaseUri + ":" + std::to_string(port);
                        socket.bind(uri.c_str());
                        connected = true;
                    }
                    catch(zmq::error_t e){
                        port++;
                    }
                }

                return port;
            }

            void managePeers(){
                zmq::message_t request;
                zmq::message_t reply;
                zmq::socket_t socket (context, ZMQ_REP);
                socket.bind(masterUri.c_str());
                std::map<VAddr, Uri> phoneBook;
                
                VAddr maxVAddr = 0;
                unsigned nPeers = 0;

                while(true){
                    std::stringstream ss;
                    ss << s_recv(socket);

                    Uri srcUri;
                    MsgType type;
                    ss >> type;

                    switch(type){
                        
                    case VADDR_REQUEST:
                        {
                            // Reply with correct information
                            ss >> srcUri;
                            phoneBook[maxVAddr] = srcUri;
                            // Send requestet vAddr
                            s_send(socket, std::to_string(maxVAddr++).c_str());
                            nPeers++;
                            break;
                        }
                        
                    case VADDR_LOOKUP:
                        {
                            VAddr remoteVAddr;
                            ss >> remoteVAddr;

                            std::stringstream sss;

                            if(phoneBook.count(remoteVAddr) == 0){
                                sss << RETRY;
                                s_send(socket, sss.str().c_str());
                            }
                            else {
                                sss << ACK << " " << phoneBook[remoteVAddr];
                                s_send(socket, sss.str().c_str());
                            }


                            break;
                        }


                    case DESTRUCT:
                        nPeers--;
                        s_send(socket, "");
                        break;
                        
                    default:
                        // Reply empty message
                        s_send(socket, "");
                        // TODO: throw exceptions since type should be specified
                        break;

                    };

                    // All peers have destructed, so stop managing Peers
                    if(nPeers == 0){
                        return;
                    }
                    
                }
                
            }





            //  Receive 0MQ string from socket and convert into C string
            //  Caller must free returned string. Returns NULL if the context
            //  is being terminated.
            static char *
            s_recv (void *socket) {
                char buffer [256];
                int size = zmq_recv (socket, buffer, 255, 0);
                if (size == -1)
                    return NULL;
                if (size > 255)
                    size = 255;
                buffer [size] = 0;
                return strdup (buffer);
            }

            //  Convert C string to 0MQ string and send to socket
            static int
            s_send (void *socket, const char *string) {
                int size = zmq_send (socket, string, strlen (string), 0);
                return size;
            }
            
	    /** @} */

            
	    /***********************************************************************//**
             *
	     * @name Point to Point Communication Interface
	     *
	     * @{
	     *
	     ***************************************************************************/


	    /**
	     * @brief Blocking transmission of a message sendData to peer with virtual address destVAddr.
	     * 
	     * @param[in] destVAddr  VAddr of peer that will receive the message
	     * @param[in] tag        Description of the message to better distinguish messages types
	     * @param[in] context    Context in which both sender and receiver are included
	     * @param[in] sendData   Data reference of template type T will be send to receiver peer.
	     *                       T need to provide the function data(), that returns the pointer
	     *                       to the data memory address. And the function size(), that
	     *                       return the amount of data elements to send. Notice, that
	     *                       std::vector and std::array implement this interface.
	     */
	    template <typename T_Send>
	    void send(const VAddr destVAddr, const Tag tag, const Context context, const T_Send& sendData){
                Event e = asyncSend(destVAddr, tag, context, sendData);
                e.wait();
	    }
            
	    /** 
	     * @brief Non blocking transmission of a message sendData to peer with virtual address destVAddr.
	     * 
	     * @param[in] destVAddr  VAddr of peer that will receive the message
	     * @param[in] tag        Description of the message to better distinguish messages types
	     * @param[in] context    Context in which both sender and receiver are included
	     * @param[in] sendData   Data reference of template type T will be.
	     *                       T need to provide the function data(), that returns the pointer
	     *                       to the data memory address. And the function size(), that
	     *                       return the amount of data elements to send. Notice, that
	     *                       std::vector and std::array implement this interface.
	     *
	     * @return Event
	     */
            template <typename T_Send>
            Event asyncSend(const VAddr destVAddr, const Tag tag, const Context context, T_Send& sendData){
                // Create message
                zmq::message_t message(sizeof(ContextID) +
                                       sizeof(VAddr) +
                                       sizeof(Tag) +
                                       sendData.size() * sizeof(typename T_Send::value_type));

                size_t msgOffset(0);
                ContextID contextID = context.getID();
                VAddr vAddr         = context.getVAddr();
                memcpy (static_cast<char*>(message.data()) + msgOffset, &contextID,      sizeof(ContextID)); msgOffset += sizeof(ContextID);
                memcpy (static_cast<char*>(message.data()) + msgOffset, &vAddr,          sizeof(VAddr));     msgOffset += sizeof(VAddr);
                memcpy (static_cast<char*>(message.data()) + msgOffset, &tag,            sizeof(Tag));       msgOffset += sizeof(Tag);
                memcpy (static_cast<char*>(message.data()) + msgOffset, sendData.data(), sizeof(typename T_Send::value_type) * sendData.size());


                sendSockets.at(destVAddr).send(message);

                return Event(destVAddr, tag);
                
            }


	    /**
	     * @brief Blocking receive of a message recvData from peer with virtual address srcVAddr.
	     * 
	     * @param[in]  srcVAddr   VAddr of peer that sended the message
	     * @param[in]  tag        Description of the message to better distinguish messages types
	     * @param[in]  context    Context in which both sender and receiver are included
	     * @param[out] recvData   Data reference of template type T will be received from sender peer.
	     *                        T need to provide the function data(), that returns the pointer
	     *                        to the data memory address. And the function size(), that
	     *                        return the amount of data elements to send. Notice, that
	     *                        std::vector and std::array implement this interface.
	     */
	     template <typename T_Recv>
	     void recv(const VAddr srcVAddr, const Tag tag, const Context context, T_Recv& recvData){
                 bool msgReceived = false;

                 while(!msgReceived){

                     zmq::message_t message;
                     // Get message either from the message box when it
                     // was received before or directly from an other
                     // peer.
                     {
                         if(inBox.test(context.getID(), srcVAddr, tag)){
                             message = std::move(inBox.at(context.getID(), srcVAddr, tag).front());
                             inBox.at(context.getID(), srcVAddr, tag).pop();

                             if(inBox.at(context.getID(), srcVAddr, tag).empty()){
                                 inBox.erase(context.getID(), srcVAddr, tag);
                                 
                             }
                             
                         }
                         else { 
                             recvSocket.recv(&message);
                         
                         }
                         
                     }
                     
                     // Copy data from message
                     {
                         size_t msgOffset = 0;
                         ContextID remoteContextID;
                         VAddr     remoteVAddr;
                         Tag       remoteTag;
                         memcpy (&remoteContextID,  static_cast<char*>(message.data()) + msgOffset, sizeof(ContextID)); msgOffset += sizeof(ContextID);
                         memcpy (&remoteVAddr,      static_cast<char*>(message.data()) + msgOffset, sizeof(VAddr));     msgOffset += sizeof(VAddr);
                         memcpy (&remoteTag,        static_cast<char*>(message.data()) + msgOffset, sizeof(Tag));       msgOffset += sizeof(Tag);


                         // Recv rest of message
                         if(context.getID() == remoteContextID) {
                             if(srcVAddr == remoteVAddr) {
                                 if(tag  == remoteTag){

                                     memcpy (recvData.data(),
                                             static_cast<char*>(message.data()) + msgOffset,
                                             sizeof(typename T_Recv::value_type) * recvData.size());
                                 
                                     msgReceived  = true;
                                 }
                                 else {
                                     inBox(remoteContextID, remoteVAddr, remoteTag).push(std::move(message));
                                     
                                 }
                             }
                             else {
                                 inBox(remoteContextID, remoteVAddr, remoteTag).push(std::move(message));
                                 
                             }
                         }
                         else {
                             inBox(remoteContextID, remoteVAddr, remoteTag).push(std::move(message));
                             
                         }
                         
                     }

                 }

	     }

            template <typename T_Recv>
	    Event recv(const Context context, T_Recv& recvData){
                bool msgReceived = false;

                while(!msgReceived){
                    zmq::message_t message;
                    recvSocket.recv(&message);
                
                    size_t msgOffset = 0;
                    ContextID remoteContextID;
                    VAddr     remoteVAddr;
                    Tag       remoteTag;
                    memcpy (&remoteContextID,  static_cast<char*>(message.data()) + msgOffset, sizeof(ContextID)); msgOffset += sizeof(ContextID);
                    memcpy (&remoteVAddr,      static_cast<char*>(message.data()) + msgOffset, sizeof(VAddr));     msgOffset += sizeof(VAddr);
                    memcpy (&remoteTag,        static_cast<char*>(message.data()) + msgOffset, sizeof(Tag));       msgOffset += sizeof(Tag);

                    if(context.getID() == remoteContextID) {
                        memcpy (recvData.data(),
                                static_cast<char*>(message.data()) + msgOffset,
                                sizeof(typename T_Recv::value_type) * recvData.size());

                        msgReceived = true;
                        return Event(remoteVAddr, remoteTag);
                    
                    }
                    else {
                        inBox(remoteContextID, remoteVAddr, remoteTag).push(std::move(message));
                    }
                    
                }

	    }

            
	    /** @} */
    
	    /************************************************************************//**
	     *
	     * @name Collective Communication Interface
	     *
	     * @{
	     *
	     **************************************************************************/
	   
    
	    /*************************************************************************//**
	     *
	     * @name Context Interface
	     *
	     * @{
	     *
	     ***************************************************************************/
	    /**
	     * @todo peers of oldContext retain their vAddr in the newcontext
	     *
	     */
	    Context splitContext(const bool isMember, const Context oldContext){
                zmq::context_t context(1);
                zmq::message_t reqMessage;

                // Request old master for new context
                std::array<unsigned, 1> member {{ isMember }};
                ZMQ::asyncSend(0, 0, oldContext, member);
                
                // Peer with VAddr 0 collects new members
                if( oldContext.getVAddr() == 0){
                    std::array<unsigned, 1> nMembers {{ 0 }};
                    std::vector<VAddr> vAddrs;
                    
                    for(unsigned vAddr = 0; vAddr < oldContext.size(); ++vAddr){
                        std::array<unsigned, 1> remoteIsMember {{ 0 }};
                        ZMQ::recv(vAddr, 0, oldContext, remoteIsMember);

                        if(remoteIsMember[0]) {
                            nMembers[0]++;
                            vAddrs.push_back(vAddr);
                        }
                    }
                    
                    for(VAddr vAddr : vAddrs){
                        ZMQ::asyncSend(vAddr, 0, oldContext, nMembers);
                    }
                        
                }

                if(isMember){
                    std::array<unsigned, 1> nMembers {{ 0 }};
                    ZMQ::recv(0, 0, oldContext, nMembers);
                    return Context(oldContext.getID() + 1, oldContext.getVAddr(), nMembers[0]);
                    
                }
                else{
                    return Context();
                }


		
	    }

	
	    /**
	     * @brief Returns the context that contains all peers
	     *
	     */
	    Context getGlobalContext(){
	     	return initialContext;
	    }
	    /** @} */

	};

    } // namespace communicationPolicy
	
} // namespace graybat
