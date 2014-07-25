#pragma once
#include <string>
#include <utility>
#include <map>
#include <utility>
#include <array>      /* array */
#include <assert.h>   /* assert */

/************************************************************************//**
* @class Communicator
*
* @brief A generic communicator implemented by a CommunicationPolicy
*
* The Communicator provides two classes of communication schemas. On one
* hand point to point communication both synchron and asynchron and on
* the other hand collective collective operations on a Context of
* Communicators. The interface tries by a simple as possible by the
* usage of C++ features  
* (<a href="http://www.boost.org/doc/libs/1_55_0/doc/html/mpi/tutorial.html">boost::mpi</a> interface was the example).
*
***************************************************************************/
template <class T_CommunicationPolicy>
class Communicator : public T_CommunicationPolicy {
private:
    typedef T_CommunicationPolicy                          CommunicationPolicy;
    typedef unsigned                                       Tag;                                            

public:
    typedef typename CommunicationPolicy::Event             Event;
    typedef typename CommunicationPolicy::Context           Context;
    typedef typename CommunicationPolicy::ContextID         ContextID;
    typedef unsigned                                        CommID;

public:

    /***************************************************************************
     *
     * POINT TO POINT COMMUNICATION
     *
     ***************************************************************************/

    /**
     * @brief Asyncron transmission of a message *sendData" to Communicator with CommID *destCommID*.
     * 
     * @param[in] destCommID CommID that will receive the message
     * @param[in] tag        Makes it possible to distinguish messages
     * @param[in] context    Context in which both sender and receiver are part of
     * @param[in] sendData   Some data reference unknown type T (that's immaterial) that will be send
     *
     * @return Event Can be waited (Event::wait())for or checked for (Event::ready())
     */
    template <typename T>
    Event asyncSend(const CommID destCommID, const Tag tag, const Context context, const T& sendData){
	return CommunicationPolicy::asyncSendData(sendData.data(), sendData.size(), destCommID, context, tag);
    }

    /**
     * @brief Asyncron receive of a message *recvData" from Communicator with CommID *srcCommID*
     * 
     * @param[in]  srcCommID  CommID that sended the message
     * @param[in]  tag        Makes it possible to distinguish messages
     * @param[in]  context    Context in which both sender and receiver are part of
     * @param[out] recvData   Some data reference unknown type T (that's immaterial) received data will be written to
     *
     * @return Event Can be waited (Event::wait())for or checked for (Event::ready())
     *
     */
    template <typename T>
    Event asyncRecv(const CommID srcCommID, const Tag tag, const Context context, const T& recvData){
	return CommunicationPolicy::asyncRecvData(recvData.data(), recvData.size(), srcCommID, context, tag);
    }



    /**************************************************************************
     *
     * COLLECTIVE OPERATIONS
     *
     **************************************************************************/ 
    /**
     * @brief Collects *sendData* from all members of the *context* and transmits it as a list
     *        to the Communicator with *rootCommID*.
     *
     * @param[in]  rootCommID Communicator that will receive collcted data from *context* members
     * @param[in]  context    Set of Communicators that want to send Data
     * @param[in]  sendData   Data that every Communicator in the *context* sends with **same** size
     * @param[out] recvData   Data from all *context* members, that Communicator with *rootCommID* will receive.
     *                        *recvData* of all other members of the *context* will be empty.
     *
     */
    template <typename T>
    void gather(const CommID rootCommID, const Context context, const T& sendData, std::vector<T>& recvData){
    	CommunicationPolicy::gather(&sendData, 1, recvData.data(), 1, rootCommID, context);
    }

    /**
     * @brief Collects *sendData* from all members of the *context* with varying size and transmits it as a list
     *        to Communicator with *rootCommID*.
     *
     * @todo There is some parameter missing which gives the information which Communicator send
     *       how many data. Until now the receiver *rootCommID* can´t say which data is from 
     *       which Communicator.
     * @todo Give some nice name, just adding 2 is very stupid.
     *
     * @param[in]  rootCommID Communicator that will receive collcted data from *context* members
     * @param[in]  context    Set of Communicators that want to send Data
     * @param[in]  sendData   Data that every Communicator in the *context* sends with **varying** size
     * @param[out] recvData   Data from all *context* members, that Communicator with *rootCommID* will receive.
     *                        *recvData* of all other members of the *context* will be empty.
     *
     */
    template <typename T>
    void gather2(const CommID rootCommID, const Context context, const std::vector<T>& sendData, std::vector<T>& recvData){
    	CommunicationPolicy::gather2(sendData.data(), sendData.size(), recvData.data(), sendData.size(), rootCommID, context);
    }

    /**
     * @brief Collects *sendData* from all members of the *context*  and transmits it as a list
     *        to every Communicator in the *context*
     *
     * @param[in]  context  Set of Communicators that want to send Data
     * @param[in]  sendData Data that every Communicator in the *context* sends with **same** size
     * @param[out] recvData Data from all *context* members, that all Communicators* will receive.
     *
     */
    template <typename T>
    void allGather(const Context context, const T& sendData, std::vector<T>& recvData){
    	CommunicationPolicy::allGather(&sendData, 1, recvData.data(), context);
    }

    /**
     * @brief Collects *sendData* from all members of the *context* with varying size and transmits it as a list
     *        to every Communicator in the *context*
     *
     * @todo There is some parameter missing which gives the information which Communicator send
     *       how many data. Until now the receiver *rootCommID* can´t say which data is from 
     *       which Communicator.
     * @todo Give some nice name, just adding 2 is very stupid.
     *
     * @param[in]  context  Set of Communicators that want to send Data
     * @param[in]  sendData Data that every Communicator in the *context* sends with **varying** size 
     * @param[out] recvData Data from all *context* members, that all Communicators* will receive.
     *
     */
    template <typename T>
    void allGather2(const Context context, const std::vector<T>& sendData, std::vector<T>& recvData){
    	CommunicationPolicy::allGather2(sendData.data(), sendData.size(), recvData, context);
    }


    /**
     * @brief Distributes *sendData* from Communicator *rootCommID* to all Communicators in *context*.
     *        Every Communicator will receive different data.
     *
     * @remark In Contrast to broadcast where every Communicator receives the same data
     *
     * @param[in]  rootCommID Communicator that want to distribute its data
     * @param[in]  context    Set of Communicators that want to receive Data
     * @param[in]  sendData   Data that Communicator with *rootCommID* will distribute over the Communicators of the *context*
     * @param[out] recvData   Data from Communicator with *rootCommID*.
     *
     */
    template <typename T>
    void scatter(const CommID rootCommID, const Context context, const std::vector<T>& sendData, const T& recvData){
    	CommunicationPolicy::scatter(sendData.data(), 1, &recvData, 1, rootCommID, context);
    }

    /**
     * @brief Distributes *sendData* of all Communicator in the *context* to all Communicators in the *context*.
     *        Every Communicator will receive data from every other Communicator (also the own data)
     *
     * @param[in]  context  Set of Communicators that want to receive Data
     * @param[in]  sendData Data that each Communicator wants to send
     * @param[out] recvData Data from all Communicator.
     *
     */
    template <typename T>
    void allToAll(const Context context, const T& sendData, const std::vector<T>& recvData){
    	CommunicationPolicy::allToAll(&sendData, 1, recvData.data(), recvData.size(), context);
    }

    /**
     * @brief Carry out a reduction with BinaryOperation *op* on all *sendData* elements from all Communicators
     *        whithin the *context*. The result will be received by the Communicator with *rootCommID*.
     *        
     *
     * @param[in] rootCommID Communicator that will receive the result of reduction
     * @param[in] context    Set of Communicators that 
     * @param[in] op         BinaryOperation that should be used for reduction
     * @param[in] sendData   Data that every Communicator contributes to the reduction
     * @param[out] recvData  Reduced sendData that will be received by Communicator with *rootCommID*
     *
     */
    template <typename T, typename Op>
    void reduce(const CommID rootCommID, const Context context, const Op op, const std::vector<T>& sendData, const T& recvData){
     	CommunicationPolicy::reduce(sendData.data(), &recvData, sendData.size(), op, rootCommID, context);
    }

    /**
     * @brief Carry out a reduction with BinaryOperation *op* on all *sendData* elements from all Communicators
     *        whithin the *context*. The result will be received by all Communicators.
     *        
     * @param[in] context    Set of Communicators that 
     * @param[in] op         BinaryOperation that should be used for reduction
     * @param[in] sendData   Data that every Communicator contributes to the reduction
     * @param[out] recvData  Reduced sendData that will be received by all Communicators.
     *
     */
    template <typename T, typename Op>
    void allReduce(const Context context, const Op op, const std::vector<T>& sendData, T& recvData){
	CommunicationPolicy::allReduce(sendData.data(), &recvData, sendData.size(), op, context); // <=== SEG FAULT
    }

    /**
     * @brief Send *sendData* from Communicator *rootCommID* to all Communicators in *context*.
     *        Every Communicator will receive the same data.
     *
     * @remark In Contrast to scatter where every Communicator receives different data
     *
     * @param[in] rootCommID Source Communicator
     * @param[in] context    Set of Communicators that want to receive Data
     * @param[in] sendData   Data that Communicator with *rootCommID* will send to the Communicators of the *context*
     * @param[out] recvData  Data from Communicator with *rootCommID*.
     *
     */
    template <typename T>
    void broadcast(const CommID rootCommID, const Context context, const T& data){
	CommunicationPolicy::broadcast(data.data(), data.size(), rootCommID, context);
	
    }

    /**
     * @brief Synchronizes all Communicators within *context* to the same point
     *        in the programm execution (barrier).
     *        
     */
    void synchronize(const Context context){
     	CommunicationPolicy::synchronize(context);
    }

    /**
     * @brief Synchronizes all Communicators within the globalContext
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
     * @brief Creates a new context from Communicator *ids* of an *oldContext*
     *
     */
    Context createContext(const std::vector<CommID> ids, const Context oldContext){
	return CommunicationPolicy::createContext(ids, oldContext);
    }

    /**
     * @brief Returns the context that contains all Communicators
     *
     */
    Context getGlobalContext(){
	return CommunicationPolicy::initialContext;
    }


};

