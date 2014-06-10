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

    template<>
    struct MPIDatatypes<const bool>{
    	static constexpr MPI_Datatype type = MPI_CHAR;
    };


    class MPI{
    protected:
	//typedef MPI_Comm ContextID;
	typedef MPI_Comm ContextID;
	typedef unsigned CommID;
	typedef MPI_Op   BinaryOperation;

	/**************************
	 * Inner Context class
	 **************************/
	class Context {
	public:
	    Context(ContextID contextUUID) : 
	    contextUUID(contextUUID),
	    commUUID(0){}
	    
	    Context(ContextID contextUUID, CommID uuid) : 
		contextUUID(contextUUID),
		commUUID(uuid){}

	    size_t size() const{
		int n;
		MPI_Comm_size(contextUUID, &n);
		return n;
	    }

	    CommID getCommID() const {
		return commUUID;
	    }

	    ContextID getContextID() const {
		return contextUUID;
	    }

	    bool valid() const{
		return (contextUUID != MPI_COMM_NULL);
	    }
	private:
	    const ContextID contextUUID;
	    const CommID commUUID;
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

	std::map<ContextID, std::map<CommID, URI> > uriMap;

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
	    uriMap.insert(std::make_pair(initialContext.getContextID(), std::map<CommID, URI>()));
	    for(unsigned i = 0; i < contextSize; ++i){
	    	uriMap.at(initialContext.getContextID()).insert(std::make_pair(CommID(i),URI(i)));
	    }

	    std::cout << "Init MPI " << uriMap.at(initialContext.getContextID()).at(initialContext.getCommID()) << std::endl;
	    
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
	void sendData(const T* const data, const size_t count, const CommID dest, const Context context, const MsgType msgType){
	    URI destURI = uriMap.at(context.getContextID()).at(dest);
	    MPI_Send(const_cast<T*>(data), count, MPIDatatypes<T>::type, dest, msgType, context.getContextID());

	}

	template <typename T>
	Event asyncSendData(const T* const data, const size_t count, const CommID dest, const Context context, const MsgType msgType){
	    MPI_Request   request;
	    URI destURI = uriMap.at(context.getContextID()).at(dest);
	    MPI_Isend(const_cast<T*>(data), count, MPIDatatypes<T>::type, destURI, msgType, context.getContextID(), &request);
	    return Event(request);

	}

	template <typename T>
	void recvData(const T*  data, const size_t count, const CommID src, const Context context, const MsgType msgType){
	    MPI_Status  status;	    
	    URI srcURI = uriMap.at(context.getContextID()).at(src);
	    MPI_Recv(const_cast<T*>(data), count, MPIDatatypes<T>::type, srcURI, msgType, context.getContextID(), &status);
    
	}

	template <typename T>
	Event asyncRecvData(const T*  data, const size_t count, const URI src, const Context context, const MsgType msgType){
	    MPI_Request   request;
	    URI srcURI = uriMap.at(context.getContextID()).at(src);
	    MPI_Irecv(const_cast<T*>(data), count, MPIDatatypes<T>::type, srcURI, msgType, context.getContextID(), &request);
	    return Event(request);

	}

	/**************************************************************************
	 *
	 * COLLECTIVE OPERATIONS
	 *
	 **************************************************************************/ 
	template <typename T>
	void reduce(const T* sendData, const T* recvData, const size_t count, BinaryOperation op, const CommID root, const Context context){
	    URI rootURI = uriMap.at(context.getContextID()).at(root);
	    MPI_Reduce(const_cast<T*>(sendData), const_cast<T*>(recvData), count, MPIDatatypes<T>::type, op , rootURI, context.getContextID());
	}

	template <typename T>
	void allReduce(const T* sendData, const T* recvData, const size_t count, BinaryOperation op, const Context context){
	    MPI_Allreduce(const_cast<T*>(sendData), const_cast<T*>(recvData), count, MPIDatatypes<T>::type, op , context.getContextID());

	}

	template <typename T_Send, typename T_Recv>
	void gather(const T_Send* sendData, const size_t sendCount, const T_Recv* recvData, const size_t recvCount, const CommID root, const Context context){
	    URI rootURI = uriMap.at(context.getContextID()).at(root);
	    MPI_Gather(const_cast<T_Send*>(sendData), sendCount, MPIDatatypes<T_Send>::type, 
		       const_cast<T_Recv*>(recvData), recvCount, MPIDatatypes<T_Recv>::type, 
		       rootURI, context.getContextID());
	}

	template <typename T_Send, typename T_Recv>
	void allGather(const T_Send* sendData, const size_t sendCount, const T_Recv* recvData, const size_t recvCount, const Context context){
	    MPI_Allgather(const_cast<T_Send*>(sendData), sendCount, MPIDatatypes<T_Send>::type, 
			  const_cast<T_Recv*>(recvData), recvCount, MPIDatatypes<T_Recv>::type, 
			  context.getContextID());
	}

	template <typename T_Send, typename T_Recv>
	void scatter(const T_Send* sendData, const size_t sendCount, T_Recv* recvData, const size_t recvCount, const CommID root, const Context context){
	    URI rootURI = uriMap.at(context.getContextID()).at(root);
	    MPI_Scatter(const_cast<T_Send*>(sendData), sendCount, MPIDatatypes<T_Send>::type, 
			const_cast<T_Recv*>(recvData), recvCount, MPIDatatypes<T_Recv>::type, 
			rootURI, context.getContextID());
	}

	template <typename T_Send, typename T_Recv>
	void allToAll(const T_Send* sendData, const size_t sendCount, T_Recv* recvData, const size_t recvCount, const Context context){
	    MPI_Alltoall(const_cast<T_Send*>(sendData), sendCount, MPIDatatypes<T_Send>::type, 
			 const_cast<T_Recv*>(recvData), recvCount, MPIDatatypes<T_Recv>::type, 
			 context.getContextID());
	}
	
	template <typename T>
	void broadcast(const T* data, const size_t count, const CommID root, const Context context){
	    URI rootURI = uriMap.at(context.getContextID()).at(root);
	    MPI_Bcast(const_cast<T*>(data), count, MPIDatatypes<T>::type, rootURI, context.getContextID());
	}


	void synchronize(Context context){
	    MPI_Barrier(context.getContextID());
	}

	/***************************************************************************
	 *
	 * ORGANISATION
	 *
	 ***************************************************************************/
	Context createContext(const std::vector<CommID> uuids, const Context oldContext){
	    assert(uuids.size() > 1);
	    ContextID  newContextID;
	    MPI_Group oldGroup, newGroup;

	    // Translate commUUIDs to uris
	    std::vector<URI> ranks;
	    for(CommID u : uuids){
		ranks.push_back(uriMap.at(oldContext.getContextID()).at(u));
	    }

	    // Create new context	    
	    MPI_Comm_group(oldContext.getContextID(), &oldGroup);
	    MPI_Group_incl(oldGroup, ranks.size(), &(ranks[0]), &newGroup);
	    MPI_Comm_create(oldContext.getContextID(), newGroup, &newContextID);

	    if(newContextID != MPI_COMM_NULL){
	    	URI uri;
	    	MPI_Comm_rank(newContextID, &uri);
	    	Context newContext(newContextID, uri);

		// Update UriMap
		uriMap.insert(std::make_pair(newContext.getContextID(), std::map<CommID, URI>()));
		URI otherUris[newContext.size()];
	    	for(unsigned i = 0; i < newContext.size(); ++i){
	    		allGather(&uri, 1, otherUris, 1, newContext);
	    		uriMap[newContext.getContextID()][i] =  otherUris[i];
	    	}
	    	return newContext;

	    }
	    else {
	    	return Context(newContextID);
		
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

	CommID initialUUID(){
	    initMPI();
	    URI uriTmp;
	    MPI_Comm_rank(MPI_COMM_WORLD, &uriTmp);
	    return (CommID)uriTmp;

	}

    };


} // namespace CommunicationPolicy
