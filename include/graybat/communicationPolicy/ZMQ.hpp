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
#include <cstdlib>    /* std::env */
#include <string>     /* std::string, std::stoi */
#include <queue>      /* std::queue */
#include <utility>    /* std::move */
#include <thread>     /* std::thread */
#include <mutex>      /* std::mutex */

// ZMQ
#include <zmq.hpp>    /* zmq::socket_t, zmq::context_t */

// HANA
#include <boost/hana.hpp>
namespace hana = boost::hana;

// GrayBat
#include <graybat/utils/MultiKeyMap.hpp>  /* utils::MultiKeyMap */

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
		    contextID(0),
                    vAddr(0),
		    nPeers(1),
		    isValid(false){

		}

		Context(ContextID contextID, VAddr vAddr, unsigned nPeers) :
		    contextID(contextID),
                    vAddr(vAddr),
                    nPeers(nPeers),
		    isValid(true){
		
		}

		size_t size() const{
                    return nPeers;
		}

		VAddr getVAddr() const {
                    return vAddr;
		}

                ContextID getID() const {
                    return contextID;
                }

		bool valid() const{
		    return isValid;
		}

	    private:	
                ContextID contextID;
                VAddr vAddr;
                unsigned nPeers;
                bool  isValid;		
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

	    // struct dummy1 {
	    // 	dummy1(){
	    // 	    std::cout << "Costrunct dummy1" << std::endl;
	    // 	}
	    // 	~dummy1(){
	    // 	    std::cout << "Destrunct dummy1" << std::endl;
	    // 	}

	    // };

	    // struct dummy2 {
	    // 	dummy2(){
	    // 	    std::cout << "Costrunct dummy2" << std::endl;
	    // 	}
	    // 	~dummy2(){
	    // 	    std::cout << "Destrunct dummy2" << std::endl;
	    // 	}

	    // };

	    

	    // Message types for signaling server
            static const MsgType VADDR_REQUEST   = 0;
            static const MsgType VADDR_LOOKUP    = 1;
            static const MsgType DESTRUCT        = 2;
            static const MsgType RETRY           = 3;
            static const MsgType ACK             = 4;
	    static const MsgType CONTEXT_INIT    = 5;
	    static const MsgType CONTEXT_REQUEST = 6;	    

	    // Message types between peers
	    static const MsgType PEER            = 7;
	    static const MsgType CONFIRM         = 8;
	    static const MsgType SPLIT           = 9;	    
            
            // Members
            Context initialContext;
            bool isMaster;

	    //dummy1 d1;	    	    
            zmq::context_t context;
	    //dummy2 d2;	    	    
	    
            zmq::socket_t recvSocket;
            std::map<ContextID, std::map<VAddr, std::size_t> >sendSocketMappings;
	    std::vector<zmq::socket_t> sendSockets;
            std::map<ContextID, std::map<VAddr, Uri> >phoneBook;
            std::map<ContextID, std::map<Uri, VAddr> >inversePhoneBook;	    
	    std::map<ContextID, Context> contexts;
            utils::MultiKeyMap<std::queue<zmq::message_t>, MsgType, ContextID, VAddr, Tag> inBox;
	    unsigned msgID;
	    const int zmqHwm;
	    

            std::thread recvHandler;
            std::mutex sendMtx;
            std::mutex recvMtx;	    
            
            // Uris
            Uri masterUri;
	    Uri localUri; 
            
            // Constructor
	    ZMQ() :
                isMaster(false),
                context(1),
                recvSocket(context, ZMQ_PULL),
		msgID(0),
		zmqHwm(10000),
                masterUri(std::getenv("GRAYBAT_ZMQ_MASTER_URI")){

                const Uri      localBaseUri      = std::getenv("GRAYBAT_ZMQ_LOCAL_BASE_URI");
                const unsigned localBasePort     = std::stoi(std::getenv("GRAYBAT_ZMQ_LOCAL_BASE_PORT"));
                const unsigned localPort         = searchBindPort(localBaseUri, localBasePort, recvSocket);
                               localUri          = localBaseUri + ":" + std::to_string(localPort);
                const unsigned globalContextSize = std::stoi(std::getenv("GRAYBAT_ZMQ_GLOBAL_CONTEXT_SIZE"));

                // Create socket for incoming connections
                {
		    // Request socket 
		    zmq::context_t zmqMasterContext(1);
		    zmq::socket_t  masterSocket (zmqMasterContext, ZMQ_REQ);
		    masterSocket.setsockopt( ZMQ_RCVHWM, &zmqHwm, sizeof(zmqHwm));
		    masterSocket.setsockopt( ZMQ_SNDHWM, &zmqHwm, sizeof(zmqHwm));		
		    masterSocket.connect(masterUri.c_str());		    

		    // Retrieve Context id from master
		    ContextID contextID = getInitialContextID(masterSocket, globalContextSize);
		    
                    // Retrieve vAddr from master
		    VAddr vAddr = getVAddr(masterSocket, contextID, localUri);

		    initialContext = Context(contextID, vAddr, globalContextSize);
		    contexts[initialContext.getID()] = initialContext;

		    // Ask master for uris of other peers
		    for(unsigned vAddr = 0; vAddr < initialContext.size(); vAddr++){
			Uri remoteUri = getUri(masterSocket, initialContext.getID(), vAddr);
			phoneBook[initialContext.getID()][vAddr] = remoteUri;
			inversePhoneBook[initialContext.getID()][remoteUri] = vAddr;
		    }
		    

                    // Create sockets to the other peers
		    for(unsigned vAddr = 0; vAddr < initialContext.size(); vAddr++){
		    	sendSockets.emplace_back(zmq::socket_t(context, ZMQ_PUSH));
		    	sendSocketMappings[initialContext.getID()][vAddr] = sendSockets.size() - 1;
			sendSockets.at(sendSocketMappings[initialContext.getID()].at(vAddr)).connect(phoneBook[initialContext.getID()].at(vAddr).c_str());
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
                std::array<unsigned ,1>  null;
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

	    ContextID getInitialContextID(zmq::socket_t &socket, const size_t contextSize){
		ContextID contextID = 0;
		// Send vAddr request
		std::stringstream ss;
		ss << CONTEXT_INIT << " " << contextSize;
		s_send(socket, ss.str().c_str());

		// Recv vAddr
		std::stringstream sss;
		sss << s_recv(socket);
		sss >> contextID;
		return contextID;

	    }

	    ContextID getContextID(zmq::socket_t &socket){
		ContextID contextID = 0;

		// Send vAddr request
		std::stringstream ss;
		ss << CONTEXT_REQUEST;
		s_send(socket, ss.str().c_str());

		// Recv vAddr
		std::stringstream sss;
		sss << s_recv(socket);
		sss >> contextID;
		return contextID;

	    }

	    VAddr getVAddr(zmq::socket_t &socket, const ContextID contextID, const Uri uri){
		VAddr vAddr(0);
                        
		// Send vAddr request
		std::stringstream ss;
		ss << VADDR_REQUEST << " " << contextID << " " << uri << " ";
		s_send(socket, ss.str().c_str());

		// Recv vAddr
		std::stringstream sss;
		sss << s_recv(socket);
		sss >> vAddr;

		return vAddr;
                        
	    }	    

	    Uri getUri(zmq::socket_t &socket, const ContextID contextID, const VAddr vAddr){
		MsgType type = RETRY;
                            
		while(type == RETRY){
		    // Send vAddr lookup
		    std::stringstream ss;
		    ss << VADDR_LOOKUP << " " << contextID << " " << vAddr;
		    s_send(socket, ss.str().c_str());

		    // Recv uri
		    std::string remoteUri;
		    std::stringstream sss;
		    sss << s_recv(socket);
		    sss >> type;
		    if(type == ACK){
			sss >> remoteUri;   
			return remoteUri;
			break;
			
		    }

		}

	    }

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


	    static char * s_recv (zmq::socket_t& socket) {
		zmq::message_t message(256);
		socket.recv(&message);
		if (message.size() == -1)
		    return NULL;
		if (message.size() > 255)
		    static_cast<char*>(message.data())[255] = 0;
		return strdup (static_cast<char*>(message.data()));
	    }

	    static int s_send (zmq::socket_t& socket, const char *string) {
		zmq::message_t message(sizeof(char) * strlen(string));
		memcpy (static_cast<char*>(message.data()), string, sizeof(char) * strlen(string));
		socket.send(message);
		return 0;
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

			std::cout << "recv handler: " << remoteMsgType << " " << remoteMsgID << " " << remoteContextID << " " << remoteVAddr << " " << remoteTag << std::endl;

                        if(remoteMsgType == DESTRUCT){
                            return;
                        }

			// DEBUG: for testing zmq construct
                        // if((remoteMsgType != CONFIRM)){
			if(remoteMsgType == PEER){
                            std::array<unsigned,0>  null;
			    Context context = contexts.at(remoteContextID);
                            asyncSendImpl(CONFIRM, remoteMsgID, context, remoteVAddr, remoteTag, null);
			}
			
			recvMtx.lock();
                        inBox(remoteMsgType, remoteContextID, remoteVAddr, remoteTag).push(std::move(message));
			recvMtx.unlock();			
              
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
		std::cout << "send method[" << context.getVAddr() << "]: " << context.getID() << " " << destVAddr << " " << tag << std::endl;		
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
                VAddr     srcVAddr(context.getVAddr());
		memcpy (static_cast<char*>(message.data()) + msgOffset, &msgType,        sizeof(MsgType));   msgOffset += sizeof(MsgType);
		memcpy (static_cast<char*>(message.data()) + msgOffset, &msgID,          sizeof(MsgID));     msgOffset += sizeof(MsgID);		
                memcpy (static_cast<char*>(message.data()) + msgOffset, &contextID,      sizeof(ContextID)); msgOffset += sizeof(ContextID);
                memcpy (static_cast<char*>(message.data()) + msgOffset, &srcVAddr,       sizeof(VAddr));     msgOffset += sizeof(VAddr);
                memcpy (static_cast<char*>(message.data()) + msgOffset, &tag,            sizeof(Tag));       msgOffset += sizeof(Tag);
                memcpy (static_cast<char*>(message.data()) + msgOffset, sendData.data(), sizeof(typename T_Send::value_type) * sendData.size());

                std::cout << "sendImpl: " << msgType << " " << msgID << " " << context.getID() << " " << destVAddr << " " << tag << std::endl;
		
		std::size_t sendSocket_i  = sendSocketMappings.at(context.getID()).at(destVAddr);
		zmq::socket_t &sendSocket = sendSockets.at(sendSocket_i);
		sendMtx.lock();
		sendSocket.send(message);
		sendMtx.unlock();

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
		std::cout << "recv method[" << context.getVAddr() << "]:" << context.getID() << " " << srcVAddr << " " << tag << std::endl;
		recvImpl(PEER, context, srcVAddr, tag, recvData);
		
	    }

	    template <typename T_Recv>
            Event recv(const Context context, T_Recv& recvData){
		return recvImpl(context, recvData);
	    }
	    
            template <typename T_Recv>
            void recvImpl(const MsgType msgType, const Context context, const VAddr srcVAddr, const Tag tag, T_Recv& recvData){
                std::cout << "recvImpl: " << msgType << " " << context.getID() << " " << srcVAddr << " " << tag << std::endl;
		
                zmq::message_t message;
                {
                    bool msgReceived = false;
                    while(!msgReceived){
			recvMtx.lock();
                        if(inBox.test(msgType, context.getID(), srcVAddr, tag)){
                            message = std::move(inBox.at(msgType, context.getID(), srcVAddr, tag).front());
                            inBox.at(msgType, context.getID(), srcVAddr, tag).pop();
			    

                            if(inBox.at(msgType, context.getID(), srcVAddr, tag).empty()){
                                inBox.erase(msgType, context.getID(), srcVAddr, tag);
                                 
                            }

                            msgReceived = true;
                            
                        }
			recvMtx.unlock();
                             
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
            Event recvImpl(const Context context, T_Recv& recvData){

                zmq::message_t message;
		VAddr destVAddr;
		Tag tag;
                {

		    std::vector<hana::tuple<MsgType, ContextID, VAddr, Tag> > keysList;
		    std::vector<std::reference_wrapper<std::queue<zmq::message_t> > > values;
		
		    while(values.empty()){
			recvMtx.lock();
			inBox.values(values, keysList, PEER, context.getID());
			recvMtx.unlock();
		    }

		    recvMtx.lock();
		    auto keys = keysList.front();
		    message = std::move(values.front().get().front());
		    values.front().get().pop();
		    
		    if(values.front().get().empty()){
		     	inBox.erase(keys);
		    }
		    recvMtx.unlock();		    

		    destVAddr = hana::at(keys, hana::size_c<2>);
		    tag = hana::at(keys, hana::size_c<3>);
                    
                }

		//Copy data from message
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
		return Event(msgID++, context, destVAddr, tag, *this);
            }
            
            void wait(const MsgType msgID, const Context context, const VAddr vAddr, const Tag tag){
		std::cout << "wait method: " << msgID << " " << context.getID() << " " << vAddr << " " << tag << std::endl;
                while(!ready(msgID, context, vAddr, tag));
                      
            }

            bool ready(const MsgType msgID, const Context context, const VAddr vAddr, const Tag tag){

                zmq::message_t message;

                if(inBox.test(CONFIRM, context.getID(), vAddr, tag)){
		    recvMtx.lock();		
                    message = std::move(inBox.at(CONFIRM, context.getID(), vAddr, tag).front());
                    inBox.at(CONFIRM, context.getID(), vAddr, tag).pop();

                    if(inBox.at(CONFIRM, context.getID(), vAddr, tag).empty()){
                        inBox.erase(CONFIRM, context.getID(), vAddr, tag);
                                 
                    }
		    recvMtx.unlock();            
		    
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
			recvMtx.lock();					
                        inBox(CONFIRM, context.getID(), vAddr, tag).push(std::move(message));
			recvMtx.unlock();            		    			
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
		zmq::context_t zmqMasterContext(1);
		zmq::socket_t  masterSocket (zmqMasterContext, ZMQ_REQ);
		masterSocket.connect(masterUri.c_str());

                zmq::message_t reqMessage;
		Context newContext;

                // Request old master for new context
                std::array<unsigned, 2> member {{ isMember }};
                ZMQ::asyncSendImpl(SPLIT, msgID++, oldContext, 0, 0, member);

                // Peer with VAddr 0 collects new members
                if( oldContext.getVAddr() == 0){
                    std::array<unsigned, 2> nMembers {{ 0 }};
                    std::vector<VAddr> vAddrs;
                    
                    for(unsigned vAddr = 0; vAddr < oldContext.size(); ++vAddr){
                        std::array<unsigned, 1> remoteIsMember {{ 0 }};
                        ZMQ::recvImpl(SPLIT, oldContext, vAddr, 0, remoteIsMember);

                        if(remoteIsMember[0]) {
                            nMembers[0]++;
                            vAddrs.push_back(vAddr);
                        }
                    }

		    nMembers[1] = getContextID(masterSocket);

                    for(VAddr vAddr : vAddrs){
                        ZMQ::asyncSendImpl(SPLIT, msgID++, oldContext, vAddr, 0, nMembers);
                    }
                        
                }

                 if(isMember){
                    std::array<unsigned, 2> nMembers {{ 0 , 0 }};
		    
                    ZMQ::recvImpl(SPLIT, oldContext, 0, 0, nMembers);
		    ContextID newContextID = nMembers[1];

		    newContext = Context(newContextID, getVAddr(masterSocket, newContextID, localUri), nMembers[0]);
		    contexts[newContext.getID()] = newContext;

		    // Update phonebook for new context
		    for(unsigned vAddr = 0; vAddr < newContext.size(); vAddr++){
			Uri remoteUri = getUri(masterSocket, newContext.getID(), vAddr);
		    	phoneBook[newContext.getID()][vAddr] = remoteUri;
			inversePhoneBook[newContext.getID()][remoteUri] = vAddr;
		    }
		    
		    for(unsigned vAddr = 0; vAddr < newContext.size(); vAddr++){
			Uri uri = phoneBook[newContext.getID()][vAddr];
			VAddr oldVAddr = inversePhoneBook[oldContext.getID()].at(uri);
			sendSocketMappings[newContext.getID()][vAddr] = sendSocketMappings[oldContext.getID()].at(oldVAddr);
		    }
		    
		 }
		 else{
		     // Invalid context for "not members"
		     newContext = Context();
		 }

		 // Barrier thus recvHandler is up to date with sendSocketMappings
		 std::array<unsigned, 0> null;
		 for(unsigned vAddr = 0; vAddr < oldContext.size(); ++vAddr){
		     ZMQ::asyncSendImpl(SPLIT, msgID++, oldContext, vAddr, 0, null);
		 }
		 for(unsigned vAddr = 0; vAddr < oldContext.size(); ++vAddr){
		     ZMQ::recvImpl(SPLIT, oldContext, vAddr, 0, null);
		 }

		 return newContext;

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
