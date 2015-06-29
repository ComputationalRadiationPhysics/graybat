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

#include <cstdlib>    /* std::env */
#include <string>     /* std::string, std::stoi */


// ZMQ
#include <zmq.hpp>

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
		    isValid(false){

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

		// ContextID getID() const {
		//     return 0;
		// }

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
		}
	    };


	    // Type defs
	    typedef unsigned Tag;                                            
	    typedef unsigned ContextID;
	    typedef unsigned VAddr;

	    typedef unsigned MsgType;
	    typedef int      Uri;

            // Members
            Context initialContext;
            std::map<VAddr, zmq::socket_t> phoneBookIn;
            std::map<VAddr, zmq::socket_t> phoneBookOut;
            zmq::context_t context;

            
            // Constructor
	    ZMQ() :
                context(1){

                const std::string baseAddress = "tcp://127.0.0.1:1000";
                const unsigned nPeers         = std::stoi(std::getenv("OMPI_COMM_WORLD_SIZE"));
                const unsigned peerID         = std::stoi(std::getenv("OMPI_COMM_WORLD_RANK"));
                VAddr vAddr(0);


                {
                    zmq::context_t context(1);
                    zmq::message_t reqMessage;

                    // Master distributes vAddrs to peers
                    if(peerID == 0){
                        zmq::socket_t reply (context, ZMQ_REP);
                        reply.bind((baseAddress + std::to_string(0)).c_str());

                        for(VAddr vAddr = 1; vAddr < nPeers; ++vAddr){
                            reply.recv(&reqMessage);
                            toMessage(reqMessage, vAddr);
                            reply.send(reqMessage);
                            
                        }
                        
                    }
                    // Retrieve vAddr from master
                    else {
                        // Notify the master
                        zmq::socket_t request (context, ZMQ_REQ);
                        request.connect((baseAddress + std::to_string(0)).c_str());
                        request.send(reqMessage);

                        // Receive vAddr from master
                        reqMessage.rebuild();
                        request.recv(&reqMessage);
                        std::istringstream iss(static_cast<char*>(reqMessage.data()));
                        iss >> vAddr;
                        
                    }

                }

                // Create initial context from vAddr
                initialContext = Context(vAddr, nPeers);

                {
                    // Fill phoneBook with information
                    for(VAddr remoteVAddr = 0; remoteVAddr < nPeers; ++remoteVAddr){

                        // Create socket for outgoing connection to remote
                        std::string outCon = baseAddress + std::to_string(remoteVAddr * nPeers + remoteVAddr);
                        phoneBookOut.emplace(remoteVAddr, zmq::socket_t (context, ZMQ_PUB));
                        phoneBookOut.at(remoteVAddr).connect (outCon.c_str());
                        // std::cout <<  "OUT [" << vAddr<< "]" << outCon << std::endl;

                        // Create socket for incoming connection from remote
                        std::string inCon = baseAddress + std::to_string(vAddr * nPeers + remoteVAddr);
                        phoneBookIn.emplace(remoteVAddr, zmq::socket_t (context, ZMQ_SUB));
                        phoneBookIn.at(remoteVAddr).connect (inCon.c_str());
                        // std::cout <<  "IN [" << vAddr<< "]" << inCon << std::endl;
                        

                    }
                    
                }
                
	    }

	    // Destructor
	    ~ZMQ(){
		
	    }

            /**
             * @brief transforms primitive datatypes to a zmq message
             *
             */
            template <class T>
            void toMessage(zmq::message_t &message, const T data){
                std::stringstream ss;
                ss << data;

                message.rebuild(ss.str().size());
                memcpy ((void *) message.data (), ss.str().c_str(), ss.str().size());

            }



            
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
	    // template <typename T_Send>
	    // Event asyncSend(const VAddr destVAddr, const Tag tag, const Context context, const T_Send& sendData){
	    // 	Uri destUri = getVAddrUri(context, destVAddr);
	    // 	mpi::request request = context.comm.isend(destUri, tag, sendData.data(), sendData.size());
	    // 	return Event(request);

	    // }


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
	    // template <typename T_Recv>
	    // void recv(const VAddr srcVAddr, const Tag tag, const Context context, T_Recv& recvData){
	    // 	Uri srcUri = getVAddrUri(context, srcVAddr);
	    // 	context.comm.recv(srcUri, tag, recvData.data(), recvData.size());

	    // }
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
	    template <typename T_Send, typename T_Recv>
	    void allGather(const Context context, const T_Send& sendData, T_Recv& recvData){
                
		
	    }

	
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
	    template <typename T_Send, typename T_Recv, typename T_Op>
	    void allReduce(const Context context, T_Op op, const T_Send& sendData, T_Recv& recvData){
                if(context.getVAddr() == 0){
                    for(auto &socket : phoneBookIn){
                        zmq::message_t message(recvData.size());
                        socket.second.recv(&message);
                        memcpy (recvData.data(), (void *) message.data(), recvData.size());
                        //std::istringstream iss(static_cast<unsigned*>(message.data()));
                        //iss >> (*recvData.data());

                        
                    }
                    
                }
                else {

                }

                
                // TODO
	     
	    }

	
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
	     * @brief Creates a new context from peer *ids* of an *oldContext*
	     *
	     */
	    Context createContext(const std::vector<VAddr> vAddrs, const Context oldContext){
                


                
                return Context();
		
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
