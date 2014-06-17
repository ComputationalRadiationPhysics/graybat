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
	typedef unsigned ContextID;
	typedef unsigned CommID;
	// TODO
	// replace MPI binary operation
	// by more generic one
	typedef MPI_Op   BinaryOperation;

	/**************************
	 * Inner Context class
	 **************************/
	class Context {
	public:
	    Context() :
		contextID(0),
		commID(0),
		contextSize(0){}

	    Context(ContextID contextID, CommID id, size_t contextSize) : 
		contextID(contextID),
		commID(id),
		contextSize(contextSize){}

	    Context& operator=(const Context& otherContext){
		contextID = otherContext.getContextID();
		commID    = otherContext.getCommID();
		contextSize = otherContext.size();
		return *this;

	    }

	    size_t size() const{
		return contextSize;
	    }

	    CommID getCommID() const {
		return commID;
	    }

	    ContextID getContextID() const {
		return contextID;
	    }

	    bool valid() const{
		return (contextID != 0);
	    }

	private:
	    ContextID contextID;
	    CommID    commID;
	    size_t    contextSize;
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

	ContextID contextCount;
	std::map<ContextID, std::map<CommID, URI> > uriMap;
	std::map<ContextID, MPI_Comm>               contextMap;

    protected:
	struct BinaryOperations {
	    static constexpr BinaryOperation MAX  = MPI_MAX;
	    static constexpr BinaryOperation MIN  = MPI_MIN;
	    static constexpr BinaryOperation SUM  = MPI_SUM;
	    static constexpr BinaryOperation PROD = MPI_PROD;
	};

	// Member
	Context initialContext;

	MPI() : contextCount(1), initialContext(contextCount, initialID(), MPICommSize(MPI_COMM_WORLD)) {
	    size_t contextSize = initialContext.size();
	    contextMap[contextCount] = MPI_COMM_WORLD;

	    // Create Map id -> uri
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
	    MPI_Send(const_cast<T*>(data), count, MPIDatatypes<T>::type, dest, msgType, contextMap[context.getContextID()]);

	}

	template <typename T>
	Event asyncSendData(const T* const data, const size_t count, const CommID dest, const Context context, const MsgType msgType){
	    MPI_Request   request;
	    URI destURI = uriMap.at(context.getContextID()).at(dest);
	    MPI_Isend(const_cast<T*>(data), count, MPIDatatypes<T>::type, destURI, msgType, contextMap[context.getContextID()], &request);
	    return Event(request);

	}

	template <typename T>
	void recvData(const T*  data, const size_t count, const CommID src, const Context context, const MsgType msgType){
	    MPI_Status  status;	    
	    URI srcURI = uriMap.at(context.getContextID()).at(src);
	    MPI_Recv(const_cast<T*>(data), count, MPIDatatypes<T>::type, srcURI, msgType, contextMap[context.getContextID()], &status);
    
	}

	template <typename T>
	Event asyncRecvData(const T*  data, const size_t count, const URI src, const Context context, const MsgType msgType){
	    MPI_Request   request;
	    URI srcURI = uriMap.at(context.getContextID()).at(src);
	    MPI_Irecv(const_cast<T*>(data), count, MPIDatatypes<T>::type, srcURI, msgType, contextMap[context.getContextID()], &request);
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
	    MPI_Reduce(const_cast<T*>(sendData), const_cast<T*>(recvData), count, MPIDatatypes<T>::type, op , rootURI, contextMap[context.getContextID()]);
	}

	template <typename T>
	void allReduce(const T* sendData, const T* recvData, const size_t count, BinaryOperation op, const Context context){
	    MPI_Allreduce(const_cast<T*>(sendData), const_cast<T*>(recvData), count, MPIDatatypes<T>::type, op , contextMap[context.getContextID()]);

	}

	template <typename T_Send, typename T_Recv>
	void gather(const T_Send* sendData, const size_t sendCount, const T_Recv* recvData, const size_t recvCount, const CommID root, const Context context){
	    URI rootURI = uriMap.at(context.getContextID()).at(root);
	    MPI_Gather(const_cast<T_Send*>(sendData), sendCount, MPIDatatypes<T_Send>::type, 
		       const_cast<T_Recv*>(recvData), recvCount, MPIDatatypes<T_Recv>::type, 
		       rootURI, contextMap[context.getContextID()]);
	}

	template <typename T_Send, typename T_Recv>
	void allGather(const T_Send* sendData, const size_t sendCount, const T_Recv* recvData, const size_t recvCount, const Context context){
	    MPI_Allgather(const_cast<T_Send*>(sendData), sendCount, MPIDatatypes<T_Send>::type, 
			  const_cast<T_Recv*>(recvData), recvCount, MPIDatatypes<T_Recv>::type, 
			  contextMap[context.getContextID()]);
	}

	template <typename T_Send, typename T_Recv>
	void allGather2(const T_Send* sendData, const size_t sendCount, const T_Recv* recvData, const size_t recvCount, const Context context){

	    int rcounts[context.size()];
	    int rdispls[context.size()];

	    allGather(&sendCount, 1, rcounts, context.size(), context);


	    int offset  = 0;

	    for (unsigned i=0; i < context.size(); ++i) { 
		rdispls[i] = offset; 
		offset += rcounts[i];
		
	    } 
	    
	    MPI_Allgatherv(const_cast<T_Send*>(sendData), sendCount, MPIDatatypes<T_Send>::type, 
			   const_cast<T_Recv*>(recvData), rcounts, rdispls, MPIDatatypes<T_Recv>::type, 
			   contextMap[context.getContextID()]);

	}

	template <typename T_Send, typename T_Recv>
	void scatter(const T_Send* sendData, const size_t sendCount, T_Recv* recvData, const size_t recvCount, const CommID root, const Context context){
	    URI rootURI = uriMap.at(context.getContextID()).at(root);
	    MPI_Scatter(const_cast<T_Send*>(sendData), sendCount, MPIDatatypes<T_Send>::type, 
			const_cast<T_Recv*>(recvData), recvCount, MPIDatatypes<T_Recv>::type, 
			rootURI, contextMap[context.getContextID()]);
	}

	template <typename T_Send, typename T_Recv>
	void allToAll(const T_Send* sendData, const size_t sendCount, T_Recv* recvData, const size_t recvCount, const Context context){
	    MPI_Alltoall(const_cast<T_Send*>(sendData), sendCount, MPIDatatypes<T_Send>::type, 
			 const_cast<T_Recv*>(recvData), recvCount, MPIDatatypes<T_Recv>::type, 
			 contextMap[context.getContextID()]);
	}
	
	template <typename T>
	void broadcast(const T* data, const size_t count, const CommID root, const Context context){
	    URI rootURI = uriMap.at(context.getContextID()).at(root);
	    MPI_Bcast(const_cast<T*>(data), count, MPIDatatypes<T>::type, rootURI, contextMap[context.getContextID()]);
	}


	void synchronize(Context context){
	    MPI_Barrier(contextMap[context.getContextID()]);
	}

	/***************************************************************************
	 *
	 * ORGANISATION
	 *
	 ***************************************************************************/
	Context createContext(const std::vector<CommID> ids, const Context oldContext){
	    assert(ids.size() > 1);
	    MPI_Comm  newMPIContext;
	    MPI_Group oldGroup, newGroup;

	    // Translate commIDs to uris
	    std::vector<URI> ranks;
	    for(CommID u : ids){
	    	ranks.push_back(uriMap.at(oldContext.getContextID()).at(u));
	    }

	    // Create new context	    
	    MPI_Comm_group(contextMap[oldContext.getContextID()], &oldGroup);
	    MPI_Group_incl(oldGroup, ranks.size(), &(ranks[0]), &newGroup);
	    MPI_Comm_create(contextMap[oldContext.getContextID()], newGroup, &newMPIContext);

	    if(newMPIContext != MPI_COMM_NULL){
	    	URI uri;
	    	MPI_Comm_rank(newMPIContext, &uri);
	    	Context newContext(++contextCount, uri, MPICommSize(newMPIContext));
		contextMap[newContext.getContextID()] = newMPIContext;

	    	// Update UriMap
	    	uriMap.insert(std::make_pair(newContext.getContextID(), std::map<CommID, URI>()));
	    	URI otherUris[newContext.size()];
		allGather(&uri, 1, otherUris, 1, newContext);

	    	for(unsigned i = 0; i < newContext.size(); ++i){
	    		uriMap[newContext.getContextID()][i] =  otherUris[i];
	    	}
	    	return newContext;

	    }
	    else {
	    	return Context();
		
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

	CommID initialID(){
	    initMPI();
	    URI uriTmp;
	    MPI_Comm_rank(MPI_COMM_WORLD, &uriTmp);
	    return (CommID)uriTmp;

	}

	size_t MPICommSize(MPI_Comm comm){
	    initMPI();
	    int n;
	    MPI_Comm_size(comm, &n);
	    return n;
	}

    };


} // namespace CommunicationPolicy
