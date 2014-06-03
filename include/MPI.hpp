#pragma once
#include <iostream>     /* cout */
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

    template<>
    struct MPIDatatypes<const char>{
    	static constexpr MPI_Datatype type = MPI_CHAR;
    };


    class MPI{
    protected:
	typedef MPI_Comm ContextUUID;
	typedef unsigned CommUUID;
	typedef MPI_Op   BinaryOperation;

	/**************************
	 * Inner Context class
	 **************************/
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

	    CommUUID getCommUUID() const {
		return commUUID;
	    }

	    ContextUUID getContextUUID() const {
		return contextUUID;
	    }

	    bool valid() const{
		return (contextUUID != MPI_COMM_NULL);
	    }
	private:
	    const ContextUUID contextUUID;
	    const CommUUID commUUID;
	};

	/**************************
	 * Inner Event class
	 **************************/
	class Event {
	public:
	    Event(MPI_Request request) : request(request){

	    }

	    void wait(){
		MPI_Status status;
		MPI_Wait(&request, &status);
	    }

	    bool ready(){
		int flag = 0;
		MPI_Status status;
		MPI_Test(&request, &flag, &status);

		return bool(flag);
	    }

	private:
	    MPI_Request request;
	};

    private:
	typedef unsigned MsgType;
	typedef int URI;

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
	    uriMap.insert(std::make_pair(initialContext.getContextUUID(), std::map<CommUUID, URI>()));
	    for(unsigned i = 0; i < contextSize; ++i){
	    	uriMap.at(initialContext.getContextUUID()).insert(std::make_pair(CommUUID(i),URI(i)));
	    }

	    // Create Map context -> uuid
	    std::cout << "Init MPI " << uriMap.at(initialContext.getContextUUID()).at(initialContext.getCommUUID()) << std::endl;
	    
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
	void sendData(const T* const data, const size_t count, const CommUUID dest, const Context context, const MsgType msgType){
	    URI destURI = uriMap.at(context.getContextUUID()).at(dest);
	    MPI_Send(const_cast<T*>(data), count, MPIDatatypes<T>::type, dest, msgType, context.getContextUUID());

	}

	template <typename T>
	Event asyncSendData(const T* const data, const size_t count, const CommUUID dest, const Context context, const MsgType msgType){
	    MPI_Request   request;
	    URI destURI = uriMap.at(context.getContextUUID()).at(dest);
	    MPI_Isend(const_cast<T*>(data), count, MPIDatatypes<T>::type, destURI, msgType, context.getContextUUID(), &request);
	    return Event(request);

	}

	template <typename T>
	void recvData(T*  data, const size_t count, const CommUUID src, const Context context, const MsgType msgType){
	    MPI_Status  status;	    
	    URI srcURI = uriMap.at(context.getContextUUID()).at(src);
	    MPI_Recv(const_cast<T*>(data), count, MPIDatatypes<T>::type, srcURI, msgType, context.getContextUUID(), &status);
    
	}

	template <typename T>
	Event asyncRecvData(T*  data, const size_t count, const URI src, const Context context, const MsgType msgType){
	    MPI_Request   request;
	    URI srcURI = uriMap.at(context.getContextUUID()).at(src);
	    MPI_Irecv(const_cast<T*>(data), count, MPIDatatypes<T>::type, srcURI, msgType, context.getContextUUID(), &request);
	    return Event(request);

	}

	/**************************************************************************
	 *
	 * COLLECTIVE OPERATIONS
	 *
	 **************************************************************************/ 
	template <typename T>
	void reduce(const T* sendData, T* recvData, const size_t count, BinaryOperation op, const CommUUID root, const Context context){
	    URI rootURI = uriMap.at(context.getContextUUID()).at(root);
	    MPI_Reduce(const_cast<T*>(sendData), const_cast<T*>(recvData), count, MPIDatatypes<T>::type, op , rootURI, context.getContextUUID());
	}

	template <typename T>
	void allReduce(const T* sendData, T* recvData, const size_t count, BinaryOperation op, const Context context){
	    MPI_Allreduce(const_cast<T*>(sendData), const_cast<T*>(recvData), count, MPIDatatypes<T>::type, op , context.getContextUUID());

	}

	template <typename T_Send, typename T_Recv>
	void gather(T_Send* sendData, const size_t sendCount, T_Recv* recvData, const size_t recvCount, const CommUUID root, const Context context){
	    URI rootURI = uriMap.at(context.getContextUUID()).at(root);
	    MPI_Gather(sendData, sendCount, MPIDatatypes<T_Send>::type, 
		       recvData, recvCount, MPIDatatypes<T_Recv>::type, 
		       rootURI, context.getContextUUID());
	}

	template <typename T_Send, typename T_Recv>
	void allGather(const T_Send* sendData, const size_t sendCount, T_Recv* recvData, const size_t recvCount, const Context context){
	    MPI_Allgather(const_cast<T_Send*>(sendData), sendCount, MPIDatatypes<T_Send>::type, 
			  const_cast<T_Recv*>(recvData), recvCount, MPIDatatypes<T_Recv>::type, 
			  context.getContextUUID());
	}

	template <typename T_Send, typename T_Recv>
	void scatter(T_Send* sendData, const size_t sendCount, T_Recv* recvData, const size_t recvCount, const CommUUID root, const Context context){
	    URI rootURI = uriMap.at(context.getContextUUID()).at(root);
	    MPI_Scatter(sendData, sendCount, MPIDatatypes<T_Send>::type, 
			recvData, recvCount, MPIDatatypes<T_Recv>::type, 
			rootURI, context.getContextUUID());
	}

	template <typename T_Send, typename T_Recv>
	void allToAll(T_Send* sendData, const size_t sendCount, T_Recv* recvData, const size_t recvCount, const Context context){
	    MPI_Alltoall(sendData, sendCount, MPIDatatypes<T_Send>::type, 
			 recvData, recvCount, MPIDatatypes<T_Recv>::type, 
			 context.getContextUUID());
	}
	
	template <typename T>
	void broadcast(T* data, const size_t count, const CommUUID root, const Context context){
	    URI rootURI = uriMap.at(context.getContextUUID()).at(root);
	    MPI_Bcast(const_cast<T*>(data), count, MPIDatatypes<T>::type, rootURI, context.getContextUUID());
	}


	void synchronize(Context context){
	    MPI_Barrier(context.getContextUUID());
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

	    // Translate commUUIDs to uris
	    std::vector<URI> ranks;
	    for(CommUUID u : uuids){
		ranks.push_back(uriMap.at(oldContext.getContextUUID()).at(u));
	    }

	    // Create new context	    
	    MPI_Comm_group(oldContext.getContextUUID(), &oldGroup);
	    MPI_Group_incl(oldGroup, ranks.size(), &(ranks[0]), &newGroup);
	    MPI_Comm_create(oldContext.getContextUUID(), newGroup, &newContextUUID);

	    if(newContextUUID != MPI_COMM_NULL){
	    	URI uri;
	    	MPI_Comm_rank(newContextUUID, &uri);
	    	Context newContext(newContextUUID, uri);

		// Update UriMap
		uriMap.insert(std::make_pair(newContext.getContextUUID(), std::map<CommUUID, URI>()));
		URI otherUris[newContext.size()];
	    	for(unsigned i = 0; i < newContext.size(); ++i){
	    		allGather(&uri, 1, otherUris, 1, newContext);
	    		uriMap.at(newContext.getContextUUID()).insert(std::make_pair(i, otherUris[i]));
	    	}
	    	return newContext;

	    }
	    else {
	    	return Context(newContextUUID);
		
	    }
	    
	}

    private:
	void initMPI(){
	    int flag;
	    MPI_Initialized(&flag);
	    if(!flag){
		int argc;
		char **argv;
		MPI_Init(&argc, &argv);
	    }
	}

	CommUUID initialUUID(){
	    initMPI();
	    URI uriTmp;
	    MPI_Comm_rank(MPI_COMM_WORLD, &uriTmp);
	    return (CommUUID)uriTmp;

	}

    };


} // namespace CommunicationPolicy
