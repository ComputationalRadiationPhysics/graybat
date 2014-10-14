#pragma once
#include <string>
#include <utility>
#include <map>
#include <utility>
#include <array>      /* array */
#include <assert.h>   /* assert */

/************************************************************************//**
* @class CommunicationAbstractionLayer
*
* @brief A generic communicator implemented by a CommunicationPolicy
*
* The CommunicationAbstractionLayer provides two classes of communication schemas. On one
* hand point to point communication both synchron and asynchron and on
* the other hand collective collective operations on a Context.
* The interface tries by a simple as possible by the
* usage of C++ features  
* (<a href="http://www.boost.org/doc/libs/1_55_0/doc/html/mpi/tutorial.html">boost::mpi</a> interface was the example).
*
***************************************************************************/
template <class T_CommunicationPolicy>
class CommunicationAbstractionLayer : public T_CommunicationPolicy {
private:
    typedef T_CommunicationPolicy                          CommunicationPolicy;
    typedef unsigned                                       Tag;                                            

public:
    typedef unsigned                                       ContextID;
    typedef unsigned                                       VAddr;

    /***************************************************************************
     *
     * EVENT INTERFACE
     *
     ***************************************************************************/
    struct Event : protected CommunicationPolicy::Event {
    	Event(typename CommunicationPolicy::Event event) : CommunicationPolicy::Event(event){

    	}

    	void wait() {
    	    CommunicationPolicy::Event::wait();
    	}


    	bool ready() {
    	    return CommunicationPolicy::Event::ready();
    	}

    };


    /***************************************************************************
     *
     * CONTEXT INTERFACE
     *
     ***************************************************************************/
    struct Context : protected CommunicationPolicy::Context {
    	typedef unsigned ContextID;

	Context() : CommunicationPolicy::Context(){

	}

    	Context(typename CommunicationPolicy::Context context) : CommunicationPolicy::Context(context){

    	}

    	size_t size() const {
    	    return CommunicationPolicy::Context::size();
    	}

    	VAddr getVAddr() const {
    	    return CommunicationPolicy::Context::getVAddr();
    	}

    	ContextID getID() const {
    	    return CommunicationPolicy::Context::getID();
    	}

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
     * @brief Asyncron transmission of a message *sendData" to peer with VAddr *destVAddr*.
     * 
     * @param[in] destVAddr VAddr that will receive the message
     * @param[in] tag        Makes it possible to distinguish messages
     * @param[in] context    Context in which both sender and receiver are part of
     * @param[in] sendData   Some data reference unknown type T (that's immaterial) that will be send
     *
     * @return Event Can be waited (Event::wait())for or checked for (Event::ready())
     */
    template <typename T>
    Event asyncSend(const VAddr destVAddr, const Tag tag, const Context context, const T& sendData){
	return Event(CommunicationPolicy::asyncSendData(sendData.data(), sendData.size(), destVAddr, context, tag));
    }

    /**
     * @brief Asyncron receive of a message *recvData" from peer with VAddr *srcVAddr*
     * 
     * @param[in]  srcVAddr  VAddr that sended the message
     * @param[in]  tag        Makes it possible to distinguish messages
     * @param[in]  context    Context in which both sender and receiver are part of
     * @param[out] recvData   Some data reference unknown type T (that's immaterial) received data will be written to
     *
     * @return Event Can be waited (Event::wait())for or checked for (Event::ready())
     *
     */
    template <typename T>
    Event asyncRecv(const VAddr srcVAddr, const Tag tag, const Context context, const T& recvData){
	return Event(CommunicationPolicy::asyncRecvData(recvData.data(), recvData.size(), srcVAddr, context, tag));
    }



    /**************************************************************************
     *
     * COLLECTIVE OPERATIONS INTERFACE
     *
     **************************************************************************/ 
    /**
     * @brief Collects *sendData* from all members of the *context* and transmits it as a list
     *        to the peer with *rootVAddr*.
     *
     * @param[in]  rootVAddr Peer that will receive collcted data from *context* members
     * @param[in]  context    Set of peers that want to send Data
     * @param[in]  sendData   Data that every peer in the *context* sends with **same** size
     * @param[out] recvData   Data from all *context* members, that peer with *rootVAddr* will receive.
     *                        *recvData* of all other members of the *context* will be empty.
     *
     */
    template <typename T>
    void gather(const VAddr rootVAddr, const Context context, const T& sendData, std::vector<T>& recvData){
    	CommunicationPolicy::gather(&sendData, 1, recvData.data(), 1, rootVAddr, context);
    }

    /**
     * @brief Collects *sendData* from all members of the *context* with varying size and transmits it as a list
     *        to peer with *rootVAddr*.
     *
     * @todo Give some nice name, just adding 2 is very stupid.
     * @todo Create some version of this function where recvCount is solid and
     *       not dynamically determined.
     *
     * @param[in]  rootVAddr peer that will receive collcted data from *context* members
     * @param[in]  context    Set of peers that want to send Data
     * @param[in]  sendData   Data that every peer in the *context* sends with **varying** size
     * @param[out] recvData   Data from all *context* members, that peer with *rootVAddr* will receive.
     *                        *recvData* of all other members of the *context* will be empty. The received
     *                        data is ordered by the VAddr of the peers
     * @param[out] recvCount  Number of elements each peer sends (can by varying).
     *
     */
    template <typename T>
    void gather2(const VAddr rootVAddr, const Context context, const std::vector<T>& sendData, std::vector<T>& recvData, std::vector<unsigned>& recvCount){
    	CommunicationPolicy::gather2(sendData.data(), sendData.size(), recvData, recvCount, rootVAddr, context);
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
    template <typename T>
    void allGather(const Context context, const std::vector<T>& sendData, std::vector<T>& recvData){
    	CommunicationPolicy::allGather(sendData.data(), 1, recvData.data(), context);
    }

    /**
     * @brief Collects *sendData* from all members of the *context* with varying size and transmits it as a list
     *        to every peer in the *context*
     *
     * @todo There is some parameter missing which gives the information which peer send
     *       how many data. Until now the receiver *rootVAddr* can´t say which data is from 
     *       which peer.
     * @todo Give some nice name, just adding 2 is very stupid.
     *
     * @param[in]  context    Set of peers that want to send Data
     * @param[in]  sendData   Data that every peer in the *context* sends with **varying** size 
     * @param[out] recvData   Data from all *context* members, that all peers* will receive.
     * @param[out] recvCount  Number of elements each peer sends (can by varying).
     *
     */
    template <typename T>
    void allGather2(const Context context, const std::vector<T>& sendData, std::vector<T>& recvData, std::vector<unsigned>& recvCount){
    	CommunicationPolicy::allGather2(sendData.data(), sendData.size(), recvData, context, recvCount);
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
    template <typename T>
    void scatter(const VAddr rootVAddr, const Context context, const std::vector<T>& sendData, const T& recvData){
    	CommunicationPolicy::scatter(sendData.data(), 1, &recvData, 1, rootVAddr, context);
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
    template <typename T>
    void allToAll(const Context context, const T& sendData, const std::vector<T>& recvData){
    	CommunicationPolicy::allToAll(&sendData, 1, recvData.data(), recvData.size(), context);
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
    template <typename T, typename Op>
    void reduce(const VAddr rootVAddr, const Context context, const Op op, const std::vector<T>& sendData, const T& recvData){
     	CommunicationPolicy::reduce(sendData.data(), &recvData, sendData.size(), op, rootVAddr, context);
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
    template <typename T, typename Op>
    void allReduce(const Context context, const Op op, const std::vector<T>& sendData, std::vector<T>& recvData){
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
    template <typename T>
    void broadcast(const VAddr rootVAddr, const Context context, const T& data){
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

