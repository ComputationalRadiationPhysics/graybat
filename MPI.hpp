#pragma once
#include <types.hpp>    /* CommNode */

#include <stdlib.h>     /* srand, rand */
#include <time.h>       /* time */
#include <iostream>     /* cout */
#include <string>       /* string */
#include <map>          /* map */
#include <mpi.h>        /* MPI_* */

namespace CommunicationPolicy {

    template<typename T>
    struct MPIDatatypes{
	static constexpr MPI_Datatype type = MPI_CHAR;
    };

    template<>
    struct MPIDatatypes<int>{
    	static constexpr MPI_Datatype type = MPI_INT;
    };

    template<>
    struct MPIDatatypes<unsigned>{
    	static constexpr MPI_Datatype type = MPI_UNSIGNED;
    };


    class MPI{

    private:
	typedef unsigned MsgType;

    protected:
	// TODO URI should be unsigned 
	// and then mapped to policy specific
	// URI !
	typedef int URI;
	typedef MPI_Comm Context;
	typedef MPI_Op BinaryOperation;

	struct BinaryOperations {
	    static constexpr BinaryOperation MAX  = MPI_MAX;
	    static constexpr BinaryOperation MIN  = MPI_MIN;
	    static constexpr BinaryOperation SUM  = MPI_SUM;
	    static constexpr BinaryOperation PROD = MPI_PROD;
	};

	// Member
	std::map<Context, URI> uri;
	Context initialContext;

	MPI(){
	    initialContext = MPI_COMM_WORLD;
	    URI uriTmp;
	    MPI_Comm_rank(MPI_COMM_WORLD, &uriTmp);
	    uri.insert(std::make_pair(initialContext, uriTmp));
	    std::cout << "Init MPI " << uri.at(initialContext)<< std::endl;
	    
	}

	~MPI(){
	    MPI_Finalize();
	}

	/**************************************************************************
	 *
	 * POINT TO POINT COMMUNICATION
	 *
	 **************************************************************************/ 
	template <typename T>
	void sendData(T* data, const size_t count, const URI dest, const Context context, const MsgType msgType){
	    std::cout << "[" << uri.at(context) << "] " << "MPI::Send to " << dest << std::endl;
	    MPI_Send(data, count, MPIDatatypes<T>::type, dest, msgType, context);

	}

	template <typename T>
	void asyncSendData(T* data, const size_t count, const URI dest, const Context context, const MsgType msgType){
	    MPI_Request   request;
	    MPI_Status    status;

	    MPI_Isend(data, count, MPIDatatypes<T>::type, dest, msgType, context, &request);

	    // Some calculations can be done here !
	    // So request should be returned and
	    // then queried for status (like std::future style)

	    MPI_Wait(&request, &status);

	}

	template <typename T>
	void recvData(T* data, const size_t count, const URI src, const Context context, const MsgType msgType){
	    std::cout << "[" << uri.at(context) << "] " << "MPI::Recv from " << src << std::endl;
	    MPI_Status  status;	    
	    MPI_Recv(data, count, MPIDatatypes<T>::type, src, msgType, context, &status);
    
	}

	template <typename T>
	void asyncRecvData(T* data, const size_t count, const URI dest, const Context context, const MsgType msgType){
	    MPI_Request   request;
	    MPI_Status    status;

	    MPI_Irecv(data, count, MPIDatatypes<T>::type, dest, msgType, context, &request);

	    // Some calculations can be done here !
	    // So request should be returned and
	    // then queried for status (like std::future)

	    MPI_Wait(&request, &status);

	}

	/**************************************************************************
	 *
	 * COLLECTIVE OPERATIONS
	 *
	 **************************************************************************/ 
	template <typename T>
	void reduce(T* sendData, T* recvData, const size_t count, BinaryOperation op, const URI root, const Context context){
	    MPI_Reduce(sendData, recvData, count, MPIDatatypes<T>::type, op , root, context);
	}

	template <typename T>
	void allReduce(T* sendData, T* recvData, const size_t count, BinaryOperation op, const Context context){
	    MPI_Allreduce(sendData, recvData, count, MPIDatatypes<T>::type, op , context);

	}

	template <typename T_Send, typename T_Recv>
	void gather(T_Send* sendData, const size_t sendCount, T_Recv* recvData, const size_t recvCount, const URI root, const Context context){
	    MPI_Gather(sendData, sendCount, MPIDatatypes<T_Send>::type, 
		       recvData, recvCount, MPIDatatypes<T_Recv>::type, 
		       root, context);
	}

	template <typename T_Send, typename T_Recv>
	void allGather(T_Send* sendData, const size_t sendCount, T_Recv* recvData, const size_t recvCount, const Context context){
	    MPI_Allgather(sendData, sendCount, MPIDatatypes<T_Send>::type, 
			  recvData, recvCount, MPIDatatypes<T_Recv>::type, 
			  context);
	}

	template <typename T_Send, typename T_Recv>
	void scatter(T_Send* sendData, const size_t sendCount, T_Recv* recvData, const size_t recvCount, const URI root, const Context context){
	    MPI_Scatter(sendData, sendCount, MPIDatatypes<T_Send>::type, 
			recvData, recvCount, MPIDatatypes<T_Recv>::type, 
			root, context);
	}

	template <typename T_Send, typename T_Recv>
	void allScatter(T_Send* sendData, const size_t sendCount, T_Recv* recvData, const size_t recvCount, const Context context){
	    MPI_Scatter(sendData, sendCount, MPIDatatypes<T_Send>::type, 
			recvData, recvCount, MPIDatatypes<T_Recv>::type, 
			context);
	}

	template <typename T_Send, typename T_Recv>
	void allToAll(T_Send* sendData, const size_t sendCount, T_Recv* recvData, const size_t recvCount, const Context context){
	    MPI_Alltoall(sendData, sendCount, MPIDatatypes<T_Send>::type, 
			 recvData, recvCount, MPIDatatypes<T_Recv>::type, 
			 context);
	}

	// TODO missing collective Operations :
	// barrier
	// broadcast
	// scan
	// allScan
	// Asyncron collcetives

	/***************************************************************************
	 *
	 * ORGANISATION
	 *
	 ***************************************************************************/
	Context createContext(std::vector<URI> ranks, const Context oldContext){
	    MPI_Comm  newContext;
	    MPI_Group oldGroup, newGroup;
	    MPI_Comm_group(oldContext, &oldGroup);
	      
	    MPI_Group_incl(oldGroup, ranks.size(), &(ranks[0]), &newGroup);
	    MPI_Comm_create(oldContext, newGroup, &newContext);
	    return newContext;
	    
	}

	size_t getContextSize(const Context context){
	    int n;
	    MPI_Comm_size(context, &n);
	    return (unsigned)n;
	}

    };

} // namespace CommunicationPolicy
