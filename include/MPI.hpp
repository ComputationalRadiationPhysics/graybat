#pragma once
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
    protected:
	typedef MPI_Comm ContextUUID;
	typedef unsigned CommUUID;
	typedef MPI_Op   BinaryOperation;

	class Context {
	public:
	    Context(ContextUUID contextUUID) : 
	    contextUUID(contextUUID),
	    commUUID(0){}
	    
	    Context(ContextUUID contextUUID, CommUUID uuid) : 
		contextUUID(contextUUID),
		commUUID(uuid){}

	    size_t size() const{
		int n;
		MPI_Comm_size(contextUUID, &n);
		return n;
	    }

	    CommUUID uuid() const{
		return commUUID;
	    }

	    bool valid() const{
		return (contextUUID != MPI_COMM_NULL);
	    }

	    const ContextUUID contextUUID;
	private:
	    const CommUUID commUUID;
	};

    private:
	typedef unsigned MsgType;
	typedef int URI;

	std::map<ContextUUID, CommUUID>                 uuidMap;
	std::map<ContextUUID, std::map<CommUUID, URI> > uriMap;

    protected:
	struct BinaryOperations {
	    static constexpr BinaryOperation MAX  = MPI_MAX;
	    static constexpr BinaryOperation MIN  = MPI_MIN;
	    static constexpr BinaryOperation SUM  = MPI_SUM;
	    static constexpr BinaryOperation PROD = MPI_PROD;
	};

	// Member
	Context initialContext;

	MPI() : initialContext(MPI_COMM_WORLD, initialUUID()){
	    size_t contextSize = initialContext.size();

	    // Create Map uuid -> uri
	    uriMap.insert(std::make_pair(initialContext.contextUUID, std::map<CommUUID, URI>()));
	    for(unsigned i = 0; i < contextSize; ++i){
	    	uriMap.at(initialContext.contextUUID).insert(std::make_pair(CommUUID(i),URI(i)));
	    }

	    // Create Map context -> uuid
	    uuidMap.insert(std::make_pair(initialContext.contextUUID, initialContext.uuid()));
	    std::cout << "Init MPI " << uriMap.at(initialContext.contextUUID).at(initialContext.uuid()) << std::endl;
	    
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
	void sendData(T* data, const size_t count, const CommUUID dest, const Context context, const MsgType msgType){
	    URI destURI = uriMap.at(context.contextUUID).at(dest);
	    std::cout << "[" << uuidMap.at(context.contextUUID) << "] " << "MPI::Send to " << dest << std::endl;

	    MPI_Send(data, count, MPIDatatypes<T>::type, dest, msgType, context.contextUUID);

	}

	template <typename T>
	void asyncSendData(T* data, const size_t count, const CommUUID dest, const Context context, const MsgType msgType){
	    MPI_Request   request;
	    MPI_Status    status;
	    URI destURI = uriMap.at(context.contextUUID).at(dest);
	    MPI_Isend(data, count, MPIDatatypes<T>::type, destURI, msgType, context.contextUUID, &request);

	    // Some calculations can be done here !
	    // So request should be returned and
	    // then queried for status (like std::future style)

	    MPI_Wait(&request, &status);

	}

	template <typename T>
	void recvData(T* data, const size_t count, const CommUUID src, const Context context, const MsgType msgType){
	    std::cout << "[" << uuidMap.at(context.contextUUID) << "] " << "MPI::Recv from " << src << std::endl;
	    MPI_Status  status;	    
	    URI srcURI = uriMap.at(context.contextUUID).at(src);
	    MPI_Recv(data, count, MPIDatatypes<T>::type, srcURI, msgType, context.contextUUID, &status);
    
	}

	template <typename T>
	void asyncRecvData(T* data, const size_t count, const URI src, const Context context, const MsgType msgType){
	    MPI_Request   request;
	    MPI_Status    status;
	    URI srcURI = uriMap.at(context.contextUUID).at(src);
	    MPI_Irecv(data, count, MPIDatatypes<T>::type, srcURI, msgType, context.contextUUID, &request);

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
	void reduce(T* sendData, T* recvData, const size_t count, BinaryOperation op, const CommUUID root, const Context context){
	    URI rootURI = uriMap.at(context.contextUUID).at(root);
	    MPI_Reduce(sendData, recvData, count, MPIDatatypes<T>::type, op , rootURI, context.contextUUID);
	}

	template <typename T>
	void allReduce(T* sendData, T* recvData, const size_t count, BinaryOperation op, const Context context){
	    MPI_Allreduce(sendData, recvData, count, MPIDatatypes<T>::type, op , context.contextUUID);

	}

	template <typename T_Send, typename T_Recv>
	void gather(T_Send* sendData, const size_t sendCount, T_Recv* recvData, const size_t recvCount, const CommUUID root, const Context context){
	    URI rootURI = uriMap.at(context.contextUUID).at(root);
	    MPI_Gather(sendData, sendCount, MPIDatatypes<T_Send>::type, 
		       recvData, recvCount, MPIDatatypes<T_Recv>::type, 
		       rootURI, context.contextUUID);
	}

	template <typename T_Send, typename T_Recv>
	void allGather(T_Send* sendData, const size_t sendCount, T_Recv* recvData, const size_t recvCount, const Context context){
	    MPI_Allgather(sendData, sendCount, MPIDatatypes<T_Send>::type, 
			  recvData, recvCount, MPIDatatypes<T_Recv>::type, 
			  context.contextUUID);
	}

	template <typename T_Send, typename T_Recv>
	void scatter(T_Send* sendData, const size_t sendCount, T_Recv* recvData, const size_t recvCount, const CommUUID root, const Context context){
	    URI rootURI = uriMap.at(context.contextUUID).at(root);
	    MPI_Scatter(sendData, sendCount, MPIDatatypes<T_Send>::type, 
			recvData, recvCount, MPIDatatypes<T_Recv>::type, 
			rootURI, context.contextUUID);
	}

	template <typename T_Send, typename T_Recv>
	void allToAll(T_Send* sendData, const size_t sendCount, T_Recv* recvData, const size_t recvCount, const Context context){
	    MPI_Alltoall(sendData, sendCount, MPIDatatypes<T_Send>::type, 
			 recvData, recvCount, MPIDatatypes<T_Recv>::type, 
			 context.contextUUID);
	}
	
	template <typename T>
	void broadcast(T* data, const size_t count, const CommUUID root, const Context context){
	    URI rootURI = uriMap.at(context.contextUUID).at(root);
	    MPI_Bcast(data, count, MPIDatatypes<T>::type, rootURI, context.contextUUID);
	}


	void synchronize(Context context){
	    MPI_Barrier(context.contextUUID);
	}

	/***************************************************************************
	 *
	 * ORGANISATION
	 *
	 ***************************************************************************/
	Context createContext(std::vector<CommUUID> uuids, const Context oldContext){
	    assert(uuids.size() > 1);
	    ContextUUID  newContextUUID;
	    MPI_Group oldGroup, newGroup;

	    // Translate uuids to uris
	    std::vector<URI> ranks;
	    for(CommUUID u : uuids){
		ranks.push_back(uriMap.at(oldContext.contextUUID).at(u));
	    }

	    // Create new context	    
	    MPI_Comm_group(oldContext.contextUUID, &oldGroup);
	    MPI_Group_incl(oldGroup, ranks.size(), &(ranks[0]), &newGroup);
	    MPI_Comm_create(oldContext.contextUUID, newGroup, &newContextUUID);

	    if(newContextUUID != MPI_COMM_NULL){
	    	URI uri;
	    	MPI_Comm_rank(newContextUUID, &uri);
	    	Context newContext(newContextUUID, uri);

	    	// Update uuidMap
		uuidMap.insert(std::make_pair(newContext.contextUUID, uri));

		// Update UriMap
		uriMap.insert(std::make_pair(newContext.contextUUID, std::map<CommUUID, URI>()));
		URI otherUris[newContext.size()];
	    	for(unsigned i = 0; i < newContext.size(); ++i){
	    		allGather(&uri, 1, otherUris, 1, newContext);
	    		uriMap.at(newContext.contextUUID).insert(std::make_pair(i, otherUris[i]));
	    	}
	    	return newContext;

	    }
	    else {
	    	return Context(newContextUUID);
		
	    }
	    
	}

	URI initialUUID(){
	    URI uriTmp;
	    MPI_Comm_rank(MPI_COMM_WORLD, &uriTmp);
	    return uriTmp;

	}

    };


} // namespace CommunicationPolicy
