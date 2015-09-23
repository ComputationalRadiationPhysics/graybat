#pragma once

// STL
#include <assert.h>   /* assert */
#include <array>      /* array */
#include <iostream>   /* std::cout */
#include <map>        /* std::map */
#include <exception>  /* std::out_of_range */
#include <sstream>    /* std::stringstream, std::istringstream */
#include <cstdlib>    /* std::env */
#include <string>     /* std::string, std::stoi */
#include <queue>      /* std::queue */
#include <utility>    /* std::move */
#include <thread>     /* std::thread */
#include <mutex>      /* std::mutex */

// CLIB
#include <assert.h>   /* assert */
#include <string.h>   /* strup */

// ZMQ
#include <zmq.hpp>    /* zmq::socket_t, zmq::context_t */

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
            typedef unsigned MsgID;
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
                    nPeers(1) {

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
                
		Event(MsgID msgID, Context context, VAddr vAddr, Tag tag, ZMQ& comm) :
                    msgID(msgID),
                    context(context),
                    vAddr(vAddr),
                    tag(tag),
                    comm(comm){
		}

		void wait(){
                    comm.wait(msgID, context, vAddr, tag);

		}

                bool ready(){
                    comm.ready(msgID, context, vAddr, tag);
                    return true;
                }

                Tag getTag(){
                    return tag;

                }

                MsgID     msgID;
                Context   context;
                VAddr     vAddr;
                Tag       tag;
                ZMQ&      comm;

                
	    };

	    // Message types for signaling server
            static const MsgType VADDR_REQUEST = 0;
            static const MsgType VADDR_LOOKUP  = 1;
            static const MsgType DESTRUCT      = 2;
            static const MsgType RETRY         = 3;
            static const MsgType ACK           = 4;

	    // Message types between peers
	    static const MsgType PEER          = 5;
	    static const MsgType CONFIRM       = 6;
            
            // Members
            Context initialContext;
            bool isMaster;
            
            zmq::context_t context;
            zmq::socket_t recvSocket;
            std::map<VAddr, zmq::socket_t> sendSockets;
            std::map<VAddr, Uri> phoneBook;
            utils::MultiKeyMap<std::queue<zmq::message_t>, MsgType, ContextID, VAddr, Tag> inBox;
            //utils::MultiKeyMap<std::queue<zmq::message_t>, ContextID, VAddr, Tag, MsgID> confirmBox;
	    unsigned msgID;
            std::thread recvHandler;
            std::mutex mtx;
            
            // Uris
            Uri masterUri;
            
            // Constructor
	    ZMQ() :
                isMaster(false),
                context(1),
                recvSocket(context, ZMQ_PULL),
		msgID(0),
                masterUri(std::getenv("GRAYBAT_ZMQ_MASTER_URI")){

                const Uri      localBaseUri      = std::getenv("GRAYBAT_ZMQ_LOCAL_BASE_URI");
                const unsigned localBasePort     = std::stoi(std::getenv("GRAYBAT_ZMQ_LOCAL_BASE_PORT"));
                const unsigned localPort         = searchBindPort(localBaseUri, localBasePort, recvSocket);
                const Uri      localUri          = localBaseUri + ":" + std::to_string(localPort);
                const unsigned globalContextSize = std::stoi(std::getenv("GRAYBAT_ZMQ_GLOBAL_CONTEXT_SIZE"));

                // Create socket for incoming connections
                {

                    // Retrieve vAddr from master
                    // --> Get own phonebook adress
                    {
                        VAddr vAddr(0);
                        
                        zmq::context_t context(1);
                        zmq::socket_t  socket (context, ZMQ_REQ);
                        socket.connect(masterUri.c_str());

                        // Send vAddr request
                        std::stringstream ss;
                        ss << VADDR_REQUEST << " " << localUri;
                        s_send(socket, ss.str().c_str());

                        // Recv vAddr
                        std::stringstream sss;
                        sss << s_recv(socket);
                        sss >> vAddr;

                        initialContext = Context(0, vAddr, globalContextSize);
                        
                    }

                    // Ask master for uris of other peers
                    // --> Fill phonebook with information
                    {
                        zmq::context_t context(1);
                        zmq::socket_t  socket (context, ZMQ_REQ);
                        socket.connect(masterUri.c_str());

                        for(unsigned vAddr = 0; vAddr < globalContextSize; vAddr++){

                            MsgType type = RETRY;
                            
                            while(type == RETRY){
                                // Send vAddr lookup
                                std::stringstream ss;
                                ss << VADDR_LOOKUP << " " << vAddr;
                                s_send(socket, ss.str().c_str());

                                // Recv uri
                                std::string remoteUri;
                                std::stringstream sss;
                                sss << s_recv(socket);
                                sss >> type;
                                if(type == ACK){
                                    sss >> remoteUri;   
                                    phoneBook[vAddr] = remoteUri;
                                    break;
                                }

                            }
                            //std::cout << "[" << initialContext.getVAddr() << "] " << vAddr << ":" << phoneBook[vAddr] << std::endl;

                        }

                    }

                    // Create sockets to the other peers
                    {
                        for(unsigned vAddr = 0; vAddr < globalContextSize; vAddr++){
                            sendSockets.emplace(vAddr, zmq::socket_t(context, ZMQ_PUSH) );
                            sendSockets.at(vAddr).connect(phoneBook.at(vAddr).c_str());
                        }
                        
                    }

                }

                // Create recv thread
                {
                    recvHandler = std::thread(&ZMQ::handleRecv, this);
                        
                }

	    }

	    // Destructor
	    ~ZMQ(){
                // Send exit to signaling server
                zmq::context_t context(1);
                zmq::socket_t  socket (context, ZMQ_REQ);
                socket.connect(masterUri.c_str());
                s_send(socket, std::to_string(DESTRUCT).c_str());

                // Send exit to receive handler
                std::array<unsigned,0>  null;
                asyncSendImpl(DESTRUCT, 0, initialContext, initialContext.getVAddr(), 0, null);
                recvHandler.join();
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

            void handleRecv(){

                while(true){

                    zmq::message_t message;
                    recvSocket.recv(&message);

                    {
                        size_t    msgOffset = 0;
                        MsgType   remoteMsgType;
                        MsgID     remoteMsgID;
                        ContextID remoteContextID;
                        VAddr     remoteVAddr;
                        Tag       remoteTag;

                        memcpy (&remoteMsgType,    static_cast<char*>(message.data()) + msgOffset, sizeof(MsgType));   msgOffset += sizeof(MsgType);
                        memcpy (&remoteMsgID,      static_cast<char*>(message.data()) + msgOffset, sizeof(MsgID));     msgOffset += sizeof(MsgID);
                        memcpy (&remoteContextID,  static_cast<char*>(message.data()) + msgOffset, sizeof(ContextID)); msgOffset += sizeof(ContextID);
                        memcpy (&remoteVAddr,      static_cast<char*>(message.data()) + msgOffset, sizeof(VAddr));     msgOffset += sizeof(VAddr);
                        memcpy (&remoteTag,        static_cast<char*>(message.data()) + msgOffset, sizeof(Tag));       msgOffset += sizeof(Tag);

                        //std::cout << "recv: " << remoteMsgType << " " << remoteMsgID << " " << remoteContextID << " " << remoteVAddr << " " << remoteTag << std::endl;

                        if(remoteMsgType == DESTRUCT){
                            return;
                        }
                        
                        if(remoteMsgType != CONFIRM){
                            std::array<unsigned,0>  null;
                            asyncSendImpl(CONFIRM, remoteMsgID, initialContext, remoteVAddr, remoteTag, null);
                        }
                        inBox(remoteMsgType, remoteContextID, remoteVAddr, remoteTag).push(std::move(message));
                            

              
                    }

                }

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
		asyncSendImpl(PEER, msgID, context, destVAddr, tag, sendData);
                return Event(msgID++, context, destVAddr, tag, *this);

	    }

            
            template <typename T_Send>
		    void asyncSendImpl(const MsgType msgType, const MsgID msgID, const Context context, const VAddr destVAddr, const Tag tag, T_Send& sendData){

                // Create message
                zmq::message_t message(sizeof(MsgType) +
                                       sizeof(MsgID) +
                                       sizeof(ContextID) +
                                       sizeof(VAddr) +
                                       sizeof(Tag) +
                                       sendData.size() * sizeof(typename T_Send::value_type));

                size_t    msgOffset(0);
                ContextID contextID(context.getID());
                VAddr     vAddr(context.getVAddr());
		memcpy (static_cast<char*>(message.data()) + msgOffset, &msgType,        sizeof(MsgType));   msgOffset += sizeof(MsgType);
		memcpy (static_cast<char*>(message.data()) + msgOffset, &msgID,          sizeof(MsgID));     msgOffset += sizeof(MsgID);		
                memcpy (static_cast<char*>(message.data()) + msgOffset, &contextID,      sizeof(ContextID)); msgOffset += sizeof(ContextID);
                memcpy (static_cast<char*>(message.data()) + msgOffset, &vAddr,          sizeof(VAddr));     msgOffset += sizeof(VAddr);
                memcpy (static_cast<char*>(message.data()) + msgOffset, &tag,            sizeof(Tag));       msgOffset += sizeof(Tag);
                memcpy (static_cast<char*>(message.data()) + msgOffset, sendData.data(), sizeof(typename T_Send::value_type) * sendData.size());

                //std::cout << "send: " << msgType << " " << msgID << " " << context.getID() << " " << destVAddr << " " << tag << std::endl;

                mtx.lock();
                sendSockets.at(destVAddr).send(message);
                mtx.unlock();

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

                zmq::message_t message;
                {
                    bool msgReceived = false;
                    while(!msgReceived){
                        if(inBox.test(PEER, context.getID(), srcVAddr, tag)){
                            message = std::move(inBox.at(PEER, context.getID(), srcVAddr, tag).front());
                            inBox.at(PEER, context.getID(), srcVAddr, tag).pop();


                            if(inBox.at(PEER, context.getID(), srcVAddr, tag).empty()){
                                inBox.erase(PEER, context.getID(), srcVAddr, tag);
                                 
                            }

                            msgReceived = true;
                            
                        }
                             
                    }
                         
                }
                     
                // Copy data from message
                {
                    size_t    msgOffset = 0;
                    MsgType   remoteMsgType;
                    MsgID     remoteMsgID;
                    ContextID remoteContextID;
                    VAddr     remoteVAddr;
                    Tag       remoteTag;
			 
                    memcpy (&remoteMsgType,    static_cast<char*>(message.data()) + msgOffset, sizeof(MsgType));   msgOffset += sizeof(MsgType);
                    memcpy (&remoteMsgID,      static_cast<char*>(message.data()) + msgOffset, sizeof(MsgID));     msgOffset += sizeof(MsgID);
                    memcpy (&remoteContextID,  static_cast<char*>(message.data()) + msgOffset, sizeof(ContextID)); msgOffset += sizeof(ContextID);
                    memcpy (&remoteVAddr,      static_cast<char*>(message.data()) + msgOffset, sizeof(VAddr));     msgOffset += sizeof(VAddr);
                    memcpy (&remoteTag,        static_cast<char*>(message.data()) + msgOffset, sizeof(Tag));       msgOffset += sizeof(Tag);


                    memcpy (recvData.data(),
                            static_cast<char*>(message.data()) + msgOffset,
                            sizeof(typename T_Recv::value_type) * recvData.size());
                                 
                        
                }
                
            }

            template <typename T_Recv>
            Event recv(const Context context, T_Recv& recvData){

                zmq::message_t message;
                {
                
                    std::vector<std::tuple<MsgType, ContextID, VAddr, Tag> > values;

                    while(values.empty()){
                        inBox.values(values, PEER, context.getID());
                    }

                    // for(auto v : values){
                    //     std::cout << std::get<2>(v) << std::endl;
                    // }

                    // for(auto v : values){
                    //     //message = std::move(v.second.front());
                    // }
                    
                }

                // Copy data from message
                {
                    size_t    msgOffset = 0;
                    MsgType   remoteMsgType;
                    MsgID     remoteMsgID;
                    ContextID remoteContextID;
                    VAddr     remoteVAddr;
                    Tag       remoteTag;
			 
                    memcpy (&remoteMsgType,    static_cast<char*>(message.data()) + msgOffset, sizeof(MsgType));   msgOffset += sizeof(MsgType);
                    memcpy (&remoteMsgID,      static_cast<char*>(message.data()) + msgOffset, sizeof(MsgID));     msgOffset += sizeof(MsgID);
                    memcpy (&remoteContextID,  static_cast<char*>(message.data()) + msgOffset, sizeof(ContextID)); msgOffset += sizeof(ContextID);
                    memcpy (&remoteVAddr,      static_cast<char*>(message.data()) + msgOffset, sizeof(VAddr));     msgOffset += sizeof(VAddr);
                    memcpy (&remoteTag,        static_cast<char*>(message.data()) + msgOffset, sizeof(Tag));       msgOffset += sizeof(Tag);


                    memcpy (recvData.data(),
                            static_cast<char*>(message.data()) + msgOffset,
                            sizeof(typename T_Recv::value_type) * recvData.size());
                                 
                        
                }

            }
            
            void wait(const MsgType msgID, const Context context, const VAddr vAddr, const Tag tag){
                while(!ready(msgID, context, vAddr, tag));
                      
            }

            bool ready(const MsgType msgID, const Context context, const VAddr vAddr, const Tag tag){

                zmq::message_t message;
                if(inBox.test(CONFIRM, context.getID(), vAddr, tag)){
                    message = std::move(inBox.at(CONFIRM, context.getID(), vAddr, tag).front());
                    inBox.at(CONFIRM, context.getID(), vAddr, tag).pop();

                    if(inBox.at(CONFIRM, context.getID(), vAddr, tag).empty()){
                        inBox.erase(CONFIRM, context.getID(), vAddr, tag);
                                 
                    }
                            
                    size_t    msgOffset = 0;
                    MsgType   remoteMsgType;
                    MsgID     remoteMsgID;
                    ContextID remoteContextID;
                    VAddr     remoteVAddr;
                    Tag       remoteTag;

                    memcpy (&remoteMsgType,    static_cast<char*>(message.data()) + msgOffset, sizeof(MsgType));   msgOffset += sizeof(MsgType);
                    memcpy (&remoteMsgID,      static_cast<char*>(message.data()) + msgOffset, sizeof(MsgID));     msgOffset += sizeof(MsgID);
                    memcpy (&remoteContextID,  static_cast<char*>(message.data()) + msgOffset, sizeof(ContextID)); msgOffset += sizeof(ContextID);
                    memcpy (&remoteVAddr,      static_cast<char*>(message.data()) + msgOffset, sizeof(VAddr));     msgOffset += sizeof(VAddr);
                    memcpy (&remoteTag,        static_cast<char*>(message.data()) + msgOffset, sizeof(Tag));       msgOffset += sizeof(Tag);

                    if(remoteMsgID == msgID){
                        return true;
                    }
                    else {
                        inBox(CONFIRM, context.getID(), vAddr, tag).push(std::move(message));
                    }
                            
                }

                return false;
                             
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
