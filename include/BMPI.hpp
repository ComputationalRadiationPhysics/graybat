#pragma once
#include <assert.h>   /* assert */
#include <array>      /* array */
#include <numeric>    /* std::accumulate */

#include <map>          /* std::map */
#include <exception>    /* std::out_of_range */
#include <sstream>      /* std::stringstream */
#include <algorithm>    /* std::transform */
#include <mpi.h>        /* MPI_* */
#include <dout.hpp>     /* dout */


#include <boost/mpi/environment.hpp>
#include <boost/mpi/communicator.hpp>

namespace mpi = boost::mpi;

namespace graybat {
    
    /**
     * @brief Construction of MPI_Op from binary operation *Op* with type *T*
     *
     * @todo is it possible to get rid of this static shit ?
     */
    // template<typename T, typename Op>
    // class UserOperation
    // {
    // public:
    // 	explicit UserOperation(Op& op){
    // 	    bool isCommutative = true;
    // 	    MPI_Op_create(&UserOperation<T, Op>::perform, isCommutative, &_mpiOp);
    // 	    _op = &op;
    // 	}

    // 	~UserOperation(){
    // 	    MPI_Op_free(&_mpiOp);
    // 	}

    // 	MPI_Op& getMpiOp(){
    // 	    return _mpiOp;
    // 	}

    // 	static void perform(void* vinvec, void* voutvec, int* plen, MPI_Datatype*)
    // 	{
    // 	    T* invec = static_cast<T*>(vinvec);
    // 	    T* outvec = static_cast<T*>(voutvec);
    // 	    std::transform(invec, invec + *plen, outvec, outvec, *_op);
    // 	}

    // private:
    // 	MPI_Op _mpiOp;
    // 	static Op* _op;

    // };

    // template<typename T, typename Op> Op* UserOperation<T, Op>::_op = 0;
    
    /**
     * @brief Type traits for transformation of primitive
     *        C++ data types to MPI data types.
     *
     */
    // template<typename T>
    // struct MPIDatatypes{
    // 	static constexpr MPI_Datatype type = MPI_CHAR;
    // };

    // template<>
    // struct MPIDatatypes<float>{
    // 	static constexpr MPI_Datatype type = MPI_FLOAT;
    // };

    // template<>
    // struct MPIDatatypes<double>{
    // 	static constexpr MPI_Datatype type = MPI_DOUBLE;
    // };

    // template<>
    // struct MPIDatatypes<int>{
    // 	static constexpr MPI_Datatype type = MPI_INT;
    // };

    // template<>
    // struct MPIDatatypes<unsigned>{
    // 	static constexpr MPI_Datatype type = MPI_UNSIGNED;
    // };

    // template<>
    // struct MPIDatatypes<const char>{
    // 	static constexpr MPI_Datatype type = MPI_CHAR;
    // };

    // template<>
    // struct MPIDatatypes<const bool>{
    // 	static constexpr MPI_Datatype type = MPI_CHAR;
    // };

    /**
     * @brief A context represents a set of peers
     *        which are able to communicate with each other.
     *
     */
    class Context2 {
    	typedef unsigned ContextID;
    	typedef unsigned VAddr;
	    
    public:
    	Context2() :
    	    id(0),
    	    isValid(false){

    	}

    Context2(ContextID contextID, mpi::communicator comm) : 
	comm(comm),
	id(contextID),
	isValid(true){
		
    }

    Context2& operator=(const Context2& otherContext){
	id            = otherContext.getID();
	isValid       = otherContext.valid();
	return *this;

    }

    size_t size() const{
	return comm.size();
    }

    VAddr getVAddr() const {
	return comm.rank();
    }

    ContextID getID() const {
	return id;
    }

    bool valid() const{
	return isValid;
    }


private:
    mpi::communicator comm;
    ContextID id;
    bool      isValid;
};

    /**
     * @brief An event is returned by non-blocking 
     *        communication operations and can be 
     *        asked whether an operation has finished
     *        or it can be waited for this operation to
     *        be finished.
     *
     */
    // class Event {
    // public:
    // 	Event(MPI_Request request) : request(request){

    // 	}

    // 	~Event(){

    // 	}

    // 	void wait(){
    // 	    MPI_Status status;
    // 	    MPI_Wait(&request, &status);
	
    // 	}

    // 	bool ready(){
    // 	    int flag = 0;
    // 	    MPI_Status status;
    // 	    MPI_Test(&request, &flag, &status);
    // 	    return bool(flag);
    // 	}

    // private:
    // 	MPI_Request request;
    // };

    namespace communicationPolicy {
    
	/************************************************************************//**
	 * @class MPI
	 *
	 * @brief A communication interface implemented by the
	 * CommunicationPolicy
	 *
	 * The MPI, provides two classes
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
	struct BMPI {


	    // Type defs
	    typedef unsigned         Tag;                                            
	    typedef unsigned         ContextID;
	    typedef unsigned         VAddr;

	    typedef graybat::Context2 Context;
	    // typedef graybat::Event   Event;

	    typedef unsigned MsgType;
	    typedef int Uri;


	    // Constructor
	    // BMPI() :contextCount(0),
	    // 	   uriMap(0),
	    // 	   contextMap(0),
	    // 	   initialContext(contextCount, initialVAddr(), MPICommSize(MPI_COMM_WORLD)) {

	    BMPI() :contextCount(0),
		    uriMap(0),
		    contextMap(0),
		    initialContext(contextCount, mpi::communicator()){

		mpi::environment env;
		contextMap.push_back(mpi::communicator());
		uriMap.push_back(std::vector<Uri>());
		
		for(unsigned i = 0; i < initialContext.size(); ++i){
		    uriMap.back().push_back(i);
		}

	    }

	    // Destructor
	    ~BMPI(){
		
	    }

	    // Member Variables
	    ContextID                      contextCount;
	    std::vector<std::vector<Uri>>  uriMap;
	    std::vector<mpi::communicator> contextMap;
	    Context                        initialContext;

	


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
	    // template <typename T_Send>
	    // Event asyncSend(const VAddr destVAddr, const Tag tag, const Context context, const T_Send& sendData){
	    // 	MPI_Request request;
	    // 	Uri destUri = getVAddrUri(context, destVAddr);
	    // 	MPI_Issend(const_cast<typename T_Send::value_type*>(sendData.data()), sendData.size(),
	    // 		   MPIDatatypes<typename T_Send::value_type>::type,
	    // 		   destUri, tag, contextMap[context.getID()], &request);
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
	    // 	MPI_Send(const_cast<typename T_Send::value_type*>(sendData.data()), sendData.size(),
	    // 		 MPIDatatypes<typename T_Send::value_Type>::type, destUri, tag, contextMap[context.getID()]);
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
	     // Event asyncRecv(const VAddr srcVAddr, const Tag tag, const Context context, const T_Recv& recvData){
	     // 	 MPI_Request request;
	     // 	 Uri srcUri = getVAddrUri(context, srcVAddr);
	     // 	 MPI_Irecv(const_cast<typename T_Recv::value_type*>(recvData.data()), recvData.size(),
	     // 		   MPIDatatypes<typename T_Recv::value_type>::type, srcUri, tag,
	     // 		   contextMap[context.getID()], &request);
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
	    // void recv(const VAddr srcVAddr, const Tag tag, const Context context, const T_Recv& recvData){
	    // 	MPI_Status status;
	    // 	Uri srcUri = getVAddrUri(context, srcVAddr);
	    // 	MPI_Recv(const_cast<typename T_Recv::value_type*>(recvData.data()), recvData.size(),
	    // 		 MPIDatatypes<typename T_Recv::value_type>::type,
	    // 		 srcUri, tag, contextMap[context.getID()], &status);
	    // }


    
	    /**************************************************************************
	     *
	     * COLLECTIVE COMMUNICATION INTERFACE
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

	    // 	MPI_Gather(const_cast<typename T_Send::value_type*>(sendData.data()), sendData.size(),
	    // 		   MPIDatatypes<typename T_Send::value_type>::type, 
	    // 		   const_cast<typename T_Recv::value_type*>(recvData.data()), sendData.size(),
	    // 		   MPIDatatypes<typename T_Recv::value_type>::type, 
	    // 		   rootUri, contextMap[context.getID()]);
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
	    // 	MPI_Gatherv(const_cast<typename T_Send::value_type *>(sendData.data()), sendData.size(),
	    // 		    MPIDatatypes<typename T_Send::value_type>::type, 
	    // 		    const_cast<typename T_Recv::value_type*>(recvData.data()),
	    // 		    const_cast<int*>((int*)recvCount.data()), rdispls,
	    // 		    MPIDatatypes<typename T_Recv::value_type>::type, 
	    // 		    rootUri, contextMap[context.getID()]);
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
	    // void allGather(Context context, const T_Send& sendData, T_Recv& recvData){
	    // 	MPI_Allgather(const_cast<typename T_Send::value_type*>(sendData.data()), sendData.size(),
	    // 		      MPIDatatypes<typename T_Send::value_type>::type, 
	    // 		      const_cast<typename T_Recv::value_type*>(recvData.data()), sendData.size(),
	    // 		      MPIDatatypes<typename T_Recv::value_type>::type, 
	    // 		      contextMap[context.getID()]);
	
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
	     // 			MPIDatatypes<typename T_Send::value_type>::type, 
	     // 			const_cast<typename T_Recv::value_type*>(recvData.data()),
	     // 			const_cast<int*>((int*)recvCount.data()), rdispls,
	     // 			MPIDatatypes<typename T_Recv::value_type>::type, 
	     // 			contextMap[context.getID()]);
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
	     // 	 MPI_Scatter(const_cast<typename T_Send::value_type*>(sendData.data()), recvData.size(),
	     // 		     MPIDatatypes<typename T_Send::value_type>::type, 
	     // 		     const_cast<typename T_Recv::value_type*>(recvData.data()), recvData.size(),
	     // 		     MPIDatatypes<typename T_Recv::value_type>::type, 
	     // 		     rootUri, contextMap[context.getID()]);

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
	     // 	 MPI_Alltoall(const_cast<typename T_Send::value_type*>(sendData.data()), elementsPerPeer,
	     // 		      MPIDatatypes<typename T_Send::value_type>::type, 
	     // 		      const_cast<typename T_Recv::value_type*>(recvData), elementsPerPeer,
	     // 		      MPIDatatypes<typename T_Recv::value_type>::type, 
	     // 		      contextMap[context.getID()]);

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

	     // 	 UserOperation<typename T_Send::value_type, T_Op> mpiOp(op);
	     // 	 Uri rootUri = getVAddrUri(context, rootVAddr);
	     // 	 MPI_Reduce(const_cast<typename T_Send::value_type*>(sendData.data()),
	     // 		    const_cast<typename T_Recv::value_type*>(recvData.data()), sendData.size(),
	     // 		    MPIDatatypes<typename T_Send::value_type>::type, mpiOp.getMpiOp(),
	     // 		    rootUri, contextMap[context.getID()]);

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
	    // 	UserOperation<typename T_Send::value_type, T_Op> mpiOp(op);
	    // 	MPI_Allreduce(const_cast<typename T_Send::value_type*>(sendData.data()),
	    // 		      const_cast<typename T_Recv::value_type*>(recvData.data()), sendData.size(),
	    // 		      MPIDatatypes<typename T_Send::value_type>::type, mpiOp.getMpiOp() ,
	    // 		      contextMap[context.getID()]);
	     
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
	     // 	 MPI_Bcast(const_cast<typename T_SendRecv::value_type*>(data.data()), data.size(),
	     // 		   MPIDatatypes<typename T_SendRecv::value_type>::type, rootUri, contextMap[context.getID()]);
	     // }

	
	    /**
	     * @brief Synchronizes all peers within *context* to the same point
	     *        in the programm execution (barrier).
	     *        
	     */
	     // void synchronize(const Context context){
	     // 	 MPI_Barrier(contextMap[context.getID()]);
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
    

    
	    /***************************************************************************
	     *
	     * ORGANISATION
	     *
	     ***************************************************************************/
	    /**
	     * @brief Creates a new context from peer *ids* of an *oldContext*
	     *
	     */
	    // Context createContext(const std::vector<VAddr> vAddrs, const Context oldContext){
	    // 	assert(vAddrs.size() > 0);
	    // 	MPI_Comm  newMPIContext;
	    // 	MPI_Group oldGroup, newGroup;

	    // 	// Translate vAddrs to uris
	    // 	std::vector<Uri> ranks;

	    // 	for(VAddr vAddr : vAddrs){
	    // 	    ranks.push_back(getVAddrUri(oldContext, vAddr));
	    // 	}

	    // 	// Create new context	    
	    // 	MPI_Comm_group(contextMap[oldContext.getID()], &oldGroup);
	    // 	MPI_Group_incl(oldGroup, ranks.size(), &(ranks[0]), &newGroup);
	    // 	MPI_Comm_create(contextMap[oldContext.getID()], newGroup, &newMPIContext);

	    // 	if(newMPIContext != MPI_COMM_NULL){
	    // 	    std::array<Uri, 1> uri;
	    // 	    MPI_Comm_rank(newMPIContext, uri.data());
	    // 	    Context newContext(++contextCount, uri[0], MPICommSize(newMPIContext));
	    // 	    contextMap.push_back(newMPIContext);

	    // 	    // Update UriMap
	    // 	    uriMap.push_back(std::vector<Uri>(newContext.size()));
	    // 	    std::vector<Uri> otherUris(newContext.size());
	    // 	    allGather(newContext, uri, otherUris);

	    // 	    for(unsigned i = 0; i < newContext.size(); ++i){
	    // 		uriMap[newContext.getID()][i] =  otherUris[i];
	    // 	    }
	    // 	    return newContext;

	    // 	}
	    // 	else {
	    // 	    // return invalid context
	    // 	    // for peers not anymore included
	    // 	    return Context();
		
	    // 	}
	    
	    // }

	
	    /**
	     * @brief Returns the context that contains all peers
	     *
	     */
	    // Context getGlobalContext(){
	    // 	return initialContext;
	    // }


	    /***************************************************************************
	     *
	     * MPI HELPER FUNCTION 
	     *
	     ***************************************************************************/

	    /**
	     * @brief Initilizes MPI if it was not initialized
	     *        before.
	     *
	     */
	    // void initMPI(){
	    // 	int flag = 0;
	    // 	MPI_Initialized(&flag);
	    // 	if(!flag){
	    // 	    int argc = 0;
	    // 	    char **argv;
	    // 	    MPI_Init(&argc, &argv);
	    // 	}
	    // }

	    /**
	     * @brief Returns the initial VAddr of the peer
	     *        in the global context. This is the
	     *        rank of the MPI process in the global
	     *        communicator.
	     *
	     */
	    // VAddr initialVAddr(){
	    // 	initMPI();
	    // 	Uri uriTmp;
	    // 	MPI_Comm_rank(MPI_COMM_WORLD, &uriTmp);
	    // 	return (VAddr)uriTmp;

	    // }

	    /**
	     * @brief Returns the number of processes
	     *        in a communicator and ensures
	     *        that MPI is initialized.
	     *
	     */
	    // size_t MPICommSize(MPI_Comm comm){
	    // 	initMPI();
	    // 	int n;
	    // 	MPI_Comm_size(comm, &n);
	    // 	return n;
	    // }

	    // void error(VAddr vAddr, std::string msg){
	    // 	using namespace dout;
	    // 	Dout dout = Dout::getInstance();
	    // 	dout(Flags::ERROR) << "[" << vAddr << "] " << msg;

	    // }

	    /**
	     * @brief Returns the uri of a vAddr in a
	     *        specific context.
	     *
	     */
	    // template <typename T_Context>
	    // inline Uri getVAddrUri(const T_Context context, const VAddr vAddr){
	    // 	Uri uri  = 0;
	    // 	try {
	    // 	    uri = uriMap.at(context.getID()).at(vAddr);

	    // 	} catch(const std::out_of_range& e){
	    // 	    std::stringstream errorStream;
	    // 	    errorStream << "MPI::getVAddrUri::" << e.what()<< " : Communicator with ID " << vAddr << " is not part of the context " << context.getID() << std::endl;
	    // 	    error(context.getID(), errorStream.str());
	    // 	    exit(1);
	    // 	}

	    // 	return uri;
	    // }

	};

    } // namespace communicationPolicy
	
} // namespace graybat
