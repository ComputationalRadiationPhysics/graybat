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
#include <stack>      /* std::stack */
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

		Context(VAddr vAddr, unsigned nPeers) : 
		    isValid(true),
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
		Event() {
		}

		~Event(){
		}

		void wait(){
		}

		bool ready(){
                    return true;
		}
	    };


            struct message {

                message(const int srcUri) :
                    srcUri(srcUri){

                }

                const int srcUri;
                        
            };
                


	    // Type defs
	    typedef unsigned Tag;                                            
	    typedef unsigned ContextID;
	    typedef unsigned VAddr;

            
            
            typedef unsigned MsgType;
	    typedef std::string Uri;

            static const MsgType VADDR_REQUEST = 0;
            static const MsgType VADDR_LOOKUP  = 1;
            static const MsgType DESTRUCT      = 2;
            static const MsgType RETRY         = 3;
            static const MsgType ACK           = 4;
            
            // Members
            Context initialContext;
            std::map<VAddr, Uri> phoneBook;
            zmq::context_t context;
            zmq::socket_t socket;
            std::thread connectionManager;
            bool isMaster;
            Uri uri;

            utils::MultiKeyMap<std::stack<zmq::message_t>, ContextID, VAddr, Tag> inBox;
            
            // Const Members
            const std::string masterUri;
            
            // Constructor
	    ZMQ() :
                context(1),
                socket(context, ZMQ_REP),
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

                    // Retrieve own uri
                    const Uri localBaseUri   = getLocalUri();
                    const unsigned localPort = searchBindPort(localBaseUri, 5001, socket);
                    uri                      = localBaseUri + ":" + std::to_string(localPort);
                    
                    // Retrieve vAddr from master
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

                        initialContext = Context(vAddr, nPeers);
                        
                    }

                    // Ask master for uris of other peers
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
                            std::cout << "[" << initialContext.getVAddr() << "] " << vAddr << ":" << phoneBook[vAddr] << std::endl;

                        }

                    }
                    
                }

	    }

	    // Destructor
	    ~ZMQ(){
                if(isMaster){
                    zmq::context_t context(1);
                    zmq::socket_t  socket (context, ZMQ_REQ);
                    socket.connect(masterUri.c_str());
                    s_send(socket, std::to_string(DESTRUCT).c_str());
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
                            //std::cout << "VAddr request from: " << srcUri  << ":" << maxVAddr<< std::endl;
                            phoneBook[maxVAddr] = srcUri;
                            // Send requestet vAddr
                            s_send(socket, std::to_string(maxVAddr++).c_str());
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
                                //std::cout << "VAddr lookup for:" << remoteVAddr << ":" << phoneBook[remoteVAddr] << std::endl;
                                sss << ACK << " " << phoneBook[remoteVAddr];
                                s_send(socket, sss.str().c_str());
                            }


                            break;
                        }


                    case DESTRUCT:
                        s_send(socket, "");
                        return;
                        
                    default:
                        // Reply empty message
                        s_send(socket, "");
                        // TODO: throw exceptions since type should be specified
                        break;

                    };
                    
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
                // TODO: should be replaced by protocol framework (@see google protocol buffers)
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

                std::cout << "send [" << context.getVAddr() << "] ContextID:" << context.getID() << " DestVAddr:" << destVAddr << " Tag:" << tag << std::endl;
                
                zmq::context_t zmq_context(1);
                zmq::socket_t socket(zmq_context, ZMQ_REQ);

                socket.connect(phoneBook.at(destVAddr).c_str());
                socket.send(message);


                // TODO: fill event with information
                return Event();
                
            }


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
	    // template <typename T_Send>
	    // void send(const VAddr destVAddr, const Tag tag, const Context context, const T_Send& sendData){
	    // 	Uri destUri = getVAddrUri(context, destVAddr);
	    // 	context.comm.send(destUri, tag, sendData.data(), sendData.size());
	    // }

	    
	    /**
	     * @brief Non blocking receive of a message recvData from peer with virtual address srcVAddr.
	     * 
	     * @param[in]  srcVAddr   VAddr of peer that sended the message
	     * @param[in]  tag        Description of the message to better distinguish messages types
	     * @param[in]  context    Context in which both sender and receiver are included
	     * @param[out] recvData   Data reference of template type T will be received from sender peer.
	     *                        T need to provide the function data(), that returns the pointer
	     *                        to the data memory address. And the function size(), that
	     *                        return the amount of data elements to send. Notice, that
	     *                        std::vector and std::array implement this interface.
	     *
	     * @return Event
	     *
	     */
	    // template <typename T_Recv>
	    // Event asyncRecv(const VAddr srcVAddr, const Tag tag, const Context context, T_Recv& recvData){
	    // 	 Uri srcUri = getVAddrUri(context, srcVAddr);
	    // 	 mpi::request request = context.comm.irecv(srcUri, tag, recvData.data(), recvData.size());
	    // 	 return Event(request);
	    // }


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
                             message = std::move(inBox.at(context.getID(), srcVAddr, tag).top());
                             inBox.at(context.getID(), srcVAddr, tag).pop();
                             std::cout << "Found message in InBox" << std::endl;
                         }
                         else { 
                             socket.recv(&message);

                             // reply because of ZMQ req/rep pattern
                             zmq::message_t emptyReply;
                             socket.send(emptyReply);
                         
                         }
                     }
                     
                     // Copy data from message
                     // TODO: should be replaced by some good protocol framework (@see google protocol buffers
                     {
                         size_t msgOffset = 0;
                         ContextID remoteContextID;
                         VAddr     remoteVAddr;
                         Tag       remoteTag;
                         memcpy (&remoteContextID,  static_cast<char*>(message.data()) + msgOffset, sizeof(ContextID)); msgOffset += sizeof(ContextID);
                         memcpy (&remoteVAddr,      static_cast<char*>(message.data()) + msgOffset, sizeof(VAddr));     msgOffset += sizeof(VAddr);
                         memcpy (&remoteTag,        static_cast<char*>(message.data()) + msgOffset, sizeof(Tag));       msgOffset += sizeof(Tag);

                         std::cout << "recv [" << context.getVAddr() << "] ContextID:" << remoteContextID << " RemoteVAddr:" << remoteVAddr << " SrcVAddr:" << srcVAddr << " Tag:" << remoteTag << std::endl;
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
                                     std::cout << "Tag and remote tag are not the same: " << tag << " != " << remoteTag << std::endl;
                                     
                                 }
                             }
                             else {
                                 inBox(remoteContextID, remoteVAddr, remoteTag).push(std::move(message));
                                 std::cout << "Src VAddr and remote VAddr are not the same: " << srcVAddr << " != " << remoteVAddr << std::endl;
                                 
                             }
                         }
                         else {
                             inBox(remoteContextID, remoteVAddr, remoteTag).push(std::move(message));
                             std::cout << "Context ID and remote context ID are not the same: " << context.getID() << " != " << remoteContextID << std::endl;
                             
                         }
                         
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
	    /**
	     * @brief Collects *sendData* from all peers of the *context* and
	     *        transmits it as a list to the peer with
	     *        *rootVAddr*. Data of all peers has to be from the
	     *        **same** size.
	     *
	     * @param[in]  rootVAddr  Peer that will receive collcted data from *context* members
	     * @param[in]  context    Set of peers that want to send Data
	     * @param[in]  sendData   Data that every peer in the *context* sends.
	     *                        Data of all peers in the *context* need to have **same** size().
	     * @param[out] recvData   Data from all *context* members, that peer with virtual address 
	     *                        *rootVAddr* will receive. *recvData* of all other members of the 
	     *                        *context* will be empty.
	     */
	    // template <typename T_Send, typename T_Recv>
	    // void gather(const VAddr rootVAddr, const Context context, const T_Send& sendData, T_Recv& recvData){
	    // 	Uri rootUri = getVAddrUri(context, rootVAddr);
	    // 	mpi::gather(context.comm, sendData.data(), sendData.size(), recvData, rootUri);
	    // }

	
	    /**
	     * @brief Collects *sendData* from all members of the *context*
	     *        with **varying** size and transmits it as a list to peer
	     *        with *rootVAddr*.
	     *
	     * @todo Create some version of this function where recvCount is solid and
	     *       not dynamically determined. Since retrieving the size of send data
	     *       of every peer is a further gather operation and therefore extra
	     *       overhead.
	     *
	     * @todo Replace by boost gatherv version when available.
	     *       Patches are already submited
	     *
	     * @param[in]  rootVAddr  Peer that will receive collcted data from *context* members
	     * @param[in]  context    Set of peers that want to send Data
	     * @param[in]  sendData   Data that every peer in the *context* sends. The Data can have **varying** size
	     * @param[out] recvData   Data from all *context* peers, that peer with *rootVAddr* will receive.
	     *                        *recvData* of all other peers of the *context* will be empty. The received
	     *                        data is ordered by the VAddr of the peers.
	     * @param[out] recvCount  Number of elements each peer sends (can by varying).
	     *
	     */
	    // template <typename T_Send, typename T_Recv>
	    // void gatherVar(const VAddr rootVAddr, const Context context, const T_Send& sendData, T_Recv& recvData, std::vector<unsigned>& recvCount){
	    // 	// Retrieve number of elements each peer sends
	    // 	recvCount.resize(context.size());
	    // 	std::array<unsigned, 1> nElements{{(unsigned)sendData.size()}};
	    // 	allGather(context, nElements, recvCount);
	    // 	recvData.resize(std::accumulate(recvCount.begin(), recvCount.end(), 0U));

	    // 	Uri rootUri = getVAddrUri(context, rootVAddr);
	    // 	int rdispls[context.size()];

	    // 	// Create offset map 
	    // 	unsigned offset  = 0;
	    // 	for (unsigned i=0; i < context.size(); ++i) { 
	    // 	    rdispls[i] = offset; 
	    // 	    offset += recvCount[i];
		
	    // 	}
	    
	    // 	// Gather data with varying size
	    // 	MPI_Gatherv(const_cast<typename T_Send::value_type*>(sendData.data()), sendData.size(),
	    // 		    mpi::get_mpi_datatype<typename T_Send::value_type>(*(sendData.data())),
	    // 		    const_cast<typename T_Recv::value_type*>(recvData.data()),
	    // 		    const_cast<int*>((int*)recvCount.data()), rdispls,
	    // 		    mpi::get_mpi_datatype<typename T_Recv::value_type>(*(recvData.data())), 
	    // 		    rootUri, context.comm);
		
		
	    // }

	
	    /**
	     * @brief Collects *sendData* from all members of the *context*  and transmits it as a list
	     *        to every peer in the *context*
	     *
	     * @param[in]  context  Set of peers that want to send Data
	     * @param[in]  sendData Data that every peer in the *context* sends with **same** size
	     * @param[out] recvData Data from all *context* members, that all peers* will receive.
	     *
	     */
	    // template <typename T_Send, typename T_Recv>
	    // void allGather(const Context context, const T_Send& sendData, T_Recv& recvData){
                
		
	    // }

	
	    /**
	     * @brief Collects *sendData* from all peers of the *context*. Size of *sendData* can vary in  size.
	     *        The data is received by every peer in the *context*.
	     *
	     * @param[in]  context    Set of peers that want to send Data
	     * @param[in]  sendData   Data that every peer in the *context* sends with **varying** size 
	     * @param[out] recvData   Data from all *context* members, that all peers* will receive.
	     * @param[out] recvCount  Number of elements each peer sends (can by varying).
	     *
	     */
	     // template <typename T_Send, typename T_Recv>
	     // void allGatherVar(const Context context, const T_Send& sendData, T_Recv& recvData, std::vector<unsigned>& recvCount){
	     //     // Retrieve number of elements each peer sends
	     //     recvCount.resize(context.size());
	     //     allGather(context, std::array<unsigned, 1>{{(unsigned)sendData.size()}}, recvCount);
	     //     recvData.resize(std::accumulate(recvCount.begin(), recvCount.end(), 0U));

	     // 	 int rdispls[context.size()];

	     // 	 // Create offset map
	     // 	 unsigned offset  = 0;
	     // 	 for (unsigned i=0; i < context.size(); ++i) { 
	     // 	     rdispls[i] = offset; 
	     // 	     offset += recvCount[i];
		
	     // 	 }
	    
	     // 	 // Gather data with varying size
	     // 	 MPI_Allgatherv(const_cast<typename T_Send::value_type*>(sendData.data()), sendData.size(),
	     // 			mpi::get_mpi_datatype<typename T_Send::value_type>(*(sendData.data())),
	     // 			const_cast<typename T_Recv::value_type*>(recvData.data()),
	     // 			const_cast<int*>((int*)recvCount.data()), rdispls,
	     // 			mpi::get_mpi_datatype<typename T_Recv::value_type>(*(recvData.data())), 
	     // 			context.comm);
		 
		 
	     // }


	    /**
	     * @brief Distributes *sendData* from peer *rootVAddr* to all peers in *context*.
	     *        Every peer will receive different data.
	     *
	     * @remark In Contrast to broadcast where every peer receives the same data
	     *
	     * @param[in]  rootVAddr peer that want to distribute its data
	     * @param[in]  context    Set of peers that want to receive Data
	     * @param[in]  sendData   Data that peer with *rootVAddr* will distribute over the peers of the *context*
	     * @param[out] recvData   Data from peer with *rootVAddr*.
	     *
	     */
	    // template <typename T_Send, typename T_Recv>
	    // void scatter(const VAddr rootVAddr, const Context context, const T_Send& sendData, T_Recv& recvData){
	    // 	 Uri rootUri = getVAddrUri(context, rootVAddr);
	    // 	 mpi::scatter(context.comm, sendData.data(), recvData.data(), recvData.size(), rootUri);

	    // }

	
	    /**
	     * @brief Distributes *sendData* of all peer in the *context* to all peers in the *context*.
	     *        Every peer will receive data from every other peer (also the own data)
	     *
	     * @param[in]  context  Set of peers that want to receive Data
	     * @param[in]  sendData Data that each peer wants to send. Each peer will receive 
	     *             same number of data elements, but not the same data elements. sendData
	     *             will be divided in equal chunks of data and is then distributed.
	     *             
	     * @param[out] recvData Data from all peer.
	     *
	     */
	    // template <typename T_Send, typename T_Recv>
	    // void allToAll(const Context context, const T_Send& sendData, T_Recv& recvData){
	    //     unsigned elementsPerPeer = sendData.size() / context.size();

	    // 	mpi::all_to_all(context.comm, sendData.data(), elementsPerPeer, recvData.data());

	    // }

	
	    /**
	     * @brief Performs a reduction with a binary operator *op* on all *sendData* elements from all peers
	     *        whithin the *context*. The result will be received by the peer with *rootVAddr*.
	     *        Binary operations like std::plus, std::minus can be used. But, they can also be
	     *        defined as binary operator simular to std::plus etc.
	     *        
	     *
	     * @param[in]  rootVAddr peer that will receive the result of reduction
	     * @param[in]  context   Set of peers that 
	     * @param[in]  op        Binary operator that should be used for reduction
	     * @param[in]  sendData  Data that every peer contributes to the reduction
	     * @param[out] recvData  Reduced sendData that will be received by peer with *rootVAddr*.
	     *                       It will have same size of sendData and contains the ith
	     *                       reduced sendData values.
	     *
	     */
	    // template <typename T_Send, typename T_Recv, typename T_Op>
	    // void reduce(const VAddr rootVAddr, const Context context, const T_Op op, const T_Send& sendData, const T_Recv& recvData){
	    // 	 Uri rootUri = getVAddrUri(context, rootVAddr);
	    // 	 mpi::reduce(context.comm, sendData.data(), sendData.size(), recvData.data(), op, rootUri);

	    // }

	    /**
	     * @brief Performs a reduction with a binary operator *op* on all *sendData* elements from all peers
	     *        whithin the *context*. The result will be received by all peers.
	     *        
	     * @param[in] context    Set of peers that 
	     * @param[in] op         Binary operator that should be used for reduction
	     * @param[in] sendData   Data that every peer contributes to the reduction
	     * @param[out] recvData  Reduced sendData that will be received by all peers.
	     *                       It will have same size of sendData and contains the ith
	     *                       reduced sendData values.
	     *
	     */
	    // template <typename T_Send, typename T_Recv, typename T_Op>
	    // void allReduce(const Context context, T_Op op, const T_Send& sendData, T_Recv& recvData){
      
	     
	    // }

	
	    /**
	     * @brief Send *sendData* from peer *rootVAddr* to all peers in *context*.
	     *        Every peer will receive the same data.
	     *
	     * @remark In Contrast to scatter where every peer receives different data
	     *
	     * @param[in] rootVAddr Source peer
	     * @param[in] context    Set of peers that want to receive Data
	     * @param[in] sendData   Data that peer with *rootVAddr* will send to the peers of the *context*
	     * @param[out] recvData  Data from peer with *rootVAddr*.
	     *
	     */
	    // template <typename T_SendRecv>
	    // void broadcast(const VAddr rootVAddr, const Context context, const T_SendRecv& data){
	    // 	 Uri rootUri = uriMap.at(context.getID()).at(rootVAddr);
	    // 	 mpi::broadcast(context.comm, data.data(), data.size(), rootUri);
	    // }

	
	    /**
	     * @brief Synchronizes all peers within *context* to the same point
	     *        in the programm execution (barrier).
	     *        
	     */
	     // void synchronize(const Context context){
	     // 	 context.comm.barrier();
	     // }

	
	    /**
	     * @brief Synchronizes all peers within the globalContext
	     *        in the programm execution (barrier).
	     *
	     * @see getGlobalContext()
	     *        
	     */
	     // void synchronize(){
	     //     synchronize(getGlobalContext());
	     // }
	    /** @} */

    
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
                std::cout << "local: " << isMember << std::endl;

                // Peer with VAddr 0 collects new members
                if( oldContext.getVAddr() == 0){
                    std::array<unsigned, 1> nMembers {{ 0 }};
                    std::vector<VAddr> vAddrs;
                    
                    for(unsigned vAddr = 0; vAddr < oldContext.size(); ++vAddr){
                        std::array<unsigned, 1> remoteIsMember {{ 0 }};
                        ZMQ::recv(vAddr, 0, oldContext, remoteIsMember);
                        std::cout << "remote: " << remoteIsMember[0] << std::endl;


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
                    return Context(oldContext.getVAddr(), nMembers[0]);
                    
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
