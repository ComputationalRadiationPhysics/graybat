#pragma once
#include <assert.h>   /* assert */
#include <array>      /* array */
#include <numeric>    /* std::accumulate */

/************************************************************************//**
* @class CommunicationAbstractionLayer
*
* @brief A communication interface implemented by the
* CommunicationPolicy
*
* The CommunicationAbstractionLayer, short CAL, provides two classes
* of communication schemas .On one hand point to point communication
* between two peers, both synchron and asynchron and on the other hand collective
* collective operations on a Context.
*
* The CAL also provides inner class definitions for Event and Context.
*
* Events are objects returned by non blocking functions and they 
* can be queried for the state of this function.
*
* A Context represents a set of peers that are able to communicate
* with each other
*
***************************************************************************/
template <class T_CommunicationPolicy>
class CommunicationAbstractionLayer : public T_CommunicationPolicy {
private:
    typedef T_CommunicationPolicy CommunicationPolicy;
    typedef unsigned              Tag;                                            

public:
    typedef unsigned ContextID;
    typedef unsigned VAddr;

    
    /***********************************************************************//**
     * @class Event
     *
     * @brief Interface defintion for Event class. Event's are
     *        returned by non blocking
     *        functions. CommunicationPolicy has to implement the
     *        Event interface according to the used communciation
     *        library.
     *
     ***************************************************************************/
    struct Event : protected CommunicationPolicy::Event {
    	Event(typename CommunicationPolicy::Event event) : CommunicationPolicy::Event(event){

    	}

	/**
	 * @brief Waits until the execution of the function that
	 *        returned this event has finished. The control flow
	 *        will be blocked while waiting.
	 */
    	void wait() {
    	    CommunicationPolicy::Event::wait();
    	}

	/**
	 * @brief  Checks whether function has already finished and
	 *         returns the result of this check.
	 *
	 * @return Whether function has already finished
	 */
    	bool ready() {
    	    return CommunicationPolicy::Event::ready();
    	}

    };


    /***********************************************************************//**
     * @class Context
     * 
     * @brief Interface definition for Context class. A Context is
     *        a set of peers that are able to communicate with each
     *        other. A Context is for every communication operation
     *        necessary. A particular Context has to be implemented
     *        by the CommunicationyPolicy.
     *
     ***************************************************************************/
    struct Context : protected CommunicationPolicy::Context {
    	typedef unsigned ContextID;

	Context() : CommunicationPolicy::Context(){

	}

    	Context(typename CommunicationPolicy::Context context) : CommunicationPolicy::Context(context){

    	}

	/**
	 * @return Number of peers in the Context
	 */
    	size_t size() const {
    	    return CommunicationPolicy::Context::size();
    	}

	/**
	 * @return The virtual address of the peer that retrieved that Context
	 */
    	VAddr getVAddr() const {
    	    return CommunicationPolicy::Context::getVAddr();
    	}

	/**
	 * @return The identifier of the Context
	 */
    	ContextID getID() const {
    	    return CommunicationPolicy::Context::getID();
    	}

	/**
	 * @return true  if the peer is part of this Context <br>
	 *         false otherwise
	 */
    	bool valid() const {
    	    return CommunicationPolicy::Context::valid();

    	}

    };

    /***************************************************************************
     *
     * POINT TO POINT COMMUNICATION INTERFACE
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
    Event asyncSend(const VAddr destVAddr, const Tag tag, const Context context, const T_Send& sendData){
	return Event(CommunicationPolicy::asyncSendData(sendData.data(), sendData.size(), destVAddr, context, tag));
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
    template <typename T_Send>
    void send(const VAddr destVAddr, const Tag tag, const Context context, const T_Send& sendData){
	CommunicationPolicy::sendData(sendData.data(), sendData.size(), destVAddr, context, tag);
    }

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
    template <typename T_Recv>
    Event asyncRecv(const VAddr srcVAddr, const Tag tag, const Context context, const T_Recv& recvData){
	return Event(CommunicationPolicy::asyncRecvData(recvData.data(), recvData.size(), srcVAddr, context, tag));
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
    void recv(const VAddr srcVAddr, const Tag tag, const Context context, const T_Recv& recvData){
	CommunicationPolicy::recvData(recvData.data(), recvData.size(), srcVAddr, context, tag);
    }


    
    /**************************************************************************
     *
     * COLLECTIVE OPERATIONS INTERFACE
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
    template <typename T_Send, typename T_Recv>
    void gather(const VAddr rootVAddr, const Context context, const T_Send& sendData, T_Recv& recvData){
	CommunicationPolicy::gather(sendData.data(), sendData.size(), recvData.data(), sendData.size(), rootVAddr, context);
    }

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
     * @param[in]  rootVAddr  Peer that will receive collcted data from *context* members
     * @param[in]  context    Set of peers that want to send Data
     * @param[in]  sendData   Data that every peer in the *context* sends. The Data can have **varying** size
     * @param[out] recvData   Data from all *context* peers, that peer with *rootVAddr* will receive.
     *                        *recvData* of all other peers of the *context* will be empty. The received
     *                        data is ordered by the VAddr of the peers.
     * @param[out] recvCount  Number of elements each peer sends (can by varying).
     *
     */
    template <typename T_Send, typename T_Recv>
    void gatherVar(const VAddr rootVAddr, const Context context, const T_Send& sendData, T_Recv& recvData, std::vector<unsigned>& recvCount){
	// Retrieve number of elements each peer sends
	recvCount.resize(context.size());
	allGather(context, std::array<unsigned, 1>{{(unsigned)sendData.size()}}, recvCount);
	recvData.resize(std::accumulate(recvCount.begin(), recvCount.end(), 0U));
	
	CommunicationPolicy::gatherVar(sendData.data(), sendData.size(), recvData.data(), recvCount.data(), rootVAddr, context);
    }

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
	CommunicationPolicy::allGather(sendData.data(), sendData.size(), recvData.data(), context);
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
    template <typename T_Send, typename T_Recv>
    void allGatherVar(const Context context, const T_Send& sendData, T_Recv& recvData, std::vector<unsigned>& recvCount){
	// Retrieve number of elements each peer sends
	recvCount.resize(context.size());
	allGather(context, std::array<unsigned, 1>{{(unsigned)sendData.size()}}, recvCount);
	recvData.resize(std::accumulate(recvCount.begin(), recvCount.end(), 0U));

	CommunicationPolicy::allGatherVar(sendData.data(), sendData.size(), recvData.data(), recvCount.data(), context);
    }


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
    template <typename T_Send, typename T_Recv>
    void scatter(const VAddr rootVAddr, const Context context, const T_Send& sendData, T_Recv& recvData){
	CommunicationPolicy::scatter(sendData.data(), recvData.size(), recvData.data(), recvData.size(), rootVAddr, context);
	std::cout << recvData[0] << std::endl;
    }

    /**
     * @brief Distributes *sendData* of all peer in the *context* to all peers in the *context*.
     *        Every peer will receive data from every other peer (also the own data)
     *
     * @param[in]  context  Set of peers that want to receive Data
     * @param[in]  sendData Data that each peer wants to send
     * @param[out] recvData Data from all peer.
     *
     */
    template <typename T_Send, typename T_Recv>
    void allToAll(const Context context, const T_Send& sendData, T_Recv& recvData){
	CommunicationPolicy::allToAll(&sendData, sendData.size(), recvData.data(), sendData.size(), context);
    }

    /**
     * @brief Carry out a reduction with BinaryOperation *op* on all *sendData* elements from all peers
     *        whithin the *context*. The result will be received by the peer with *rootVAddr*.
     *        
     *
     * @param[in] rootVAddr peer that will receive the result of reduction
     * @param[in] context    Set of peers that 
     * @param[in] op         BinaryOperation that should be used for reduction
     * @param[in] sendData   Data that every peer contributes to the reduction
     * @param[out] recvData  Reduced sendData that will be received by peer with *rootVAddr*
     *
     */
    template <typename T_Send, typename T_Recv, typename T_Op>
    void reduce(const VAddr rootVAddr, const Context context, const T_Op op, const T_Send& sendData, const T_Recv& recvData){
	CommunicationPolicy::reduce(sendData.data(), recvData.data(), sendData.size(), op, rootVAddr, context);
    }

    /**
     * @brief Carry out a reduction with BinaryOperation *op* on all *sendData* elements from all peers
     *        whithin the *context*.The result will be received by all peers.
     *        
     * @param[in] context    Set of peers that 
     * @param[in] op         BinaryOperation that should be used for reduction
     * @param[in] sendData   Data that every peer contributes to the reduction
     * @param[out] recvData  Reduced sendData that will be received by all peers.
     *
     */
    template <typename T_Send, typename T_Recv, typename T_Op>
    void allReduce(const Context context, const T_Op op, const T_Send& sendData, T_Recv& recvData){
	CommunicationPolicy::allReduce(sendData.data(), recvData.data(), sendData.size(), op, context); 
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
    template <typename T_SendRecv>
    void broadcast(const VAddr rootVAddr, const Context context, const T_SendRecv& data){
	CommunicationPolicy::broadcast(data.data(), data.size(), rootVAddr, context);
	
    }

    /**
     * @brief Synchronizes all peers within *context* to the same point
     *        in the programm execution (barrier).
     *        
     */
    void synchronize(const Context context){
	CommunicationPolicy::synchronize(context);
    }

    /**
     * @brief Synchronizes all peers within the globalContext
     *        in the programm execution (barrier).
     *
     * @see getGlobalContext()
     *        
     */
    void synchronize(){
	CommunicationPolicy::synchronize(getGlobalContext());
    }
    

    
    /***************************************************************************
     *
     * ORGANISATION
     *
     ***************************************************************************/
    /**
     * @brief Creates a new context from peer *ids* of an *oldContext*
     *
     */
    Context createContext(const std::vector<VAddr> ids, const Context oldContext){
	return Context(CommunicationPolicy::createContext(ids, oldContext));
    }

    /**
     * @brief Returns the context that contains all peers
     *
     */
    Context getGlobalContext(){
	return CommunicationPolicy::initialContext;
    }


};

