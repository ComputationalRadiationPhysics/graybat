#pragma once
#include <dout.hpp>

#include <iostream>     /* std::cout */
#include <map>          /* std::map */
#include <exception>    /* std::exception */
#include <sstream>      /* std::stringstream */
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


    // TODO
    // Remove C like interface
    // not usefull for anyone -.-
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
		id(0),
		commID(0),
		contextSize(0){}

	    Context(ContextID contextID, CommID id, size_t contextSize) : 
		id(contextID),
		commID(id),
		contextSize(contextSize){}

	    Context& operator=(const Context& otherContext){
		id = otherContext.id;
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
		return id;
	    }

	    bool valid() const{
		return (id != 0);
	    }

	    ContextID id;
	private:

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
	typedef int Uri;

	ContextID contextCount;
	std::map<ContextID, std::map<CommID, Uri> > uriMap;
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

	MPI() : contextCount(1), initialContext(contextCount, initialCommID(), MPICommSize(MPI_COMM_WORLD)) {
	    size_t contextSize = initialContext.size();
	    contextMap[contextCount] = MPI_COMM_WORLD;

	    // Create Map id -> uri
	    uriMap.insert(std::make_pair(initialContext.id, std::map<CommID, Uri>()));
	    for(unsigned i = 0; i < contextSize; ++i){
	    	uriMap.at(initialContext.id).insert(std::make_pair(CommID(i),Uri(i)));
	    }

	    std::cout << "Init MPI " << uriMap.at(initialContext.id).at(initialContext.getCommID()) << std::endl;
	    
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
	Event asyncSendData(const T* const data, const size_t count, const CommID destCommID, const Context context, const MsgType msgType){
	    MPI_Request request;
	    Uri destUri = getCommIDUri(context, destCommID);
	    MPI_Isend(const_cast<T*>(data), count, MPIDatatypes<T>::type, destUri, msgType, contextMap[context.id], &request);
	    return Event(request);


	}

	template <typename T>
	Event asyncRecvData(const T*  data, const size_t count, const CommID srcCommID, const Context context, const MsgType msgType){
	    MPI_Request request;
	    Uri srcUri = getCommIDUri(context, srcCommID);
	    MPI_Irecv(const_cast<T*>(data), count, MPIDatatypes<T>::type, srcUri, msgType, contextMap[context.id], &request);
	    return Event(request);

	}

	/**************************************************************************
	 *
	 * COLLECTIVE OPERATIONS
	 *
	 **************************************************************************/ 
	template <typename T>
	void reduce(const T* sendData, const T* recvData, const size_t count, BinaryOperation op, const CommID rootCommID, const Context context){
	    Uri rootUri = getCommIDUri(context, rootCommID);
	    MPI_Reduce(const_cast<T*>(sendData), const_cast<T*>(recvData), count, MPIDatatypes<T>::type, op , rootUri, contextMap[context.id]);
	}

	template <typename T>
	void allReduce(const T* sendData, const T* recvData, const size_t count, BinaryOperation op, const Context context){
	    MPI_Allreduce(const_cast<T*>(sendData), const_cast<T*>(recvData), count, MPIDatatypes<T>::type, op , contextMap[context.id]);

	}

	template <typename T_Send, typename T_Recv>
	void gather(const T_Send* sendData, const size_t sendCount, const T_Recv* recvData, const size_t recvCount, const CommID rootCommID, const Context context){
	    Uri rootUri = getCommIDUri(context, rootCommID);

	    MPI_Gather(const_cast<T_Send*>(sendData), sendCount, MPIDatatypes<T_Send>::type, 
		       const_cast<T_Recv*>(recvData), recvCount, MPIDatatypes<T_Recv>::type, 
		       rootUri, contextMap[context.id]);
	}

	template <typename T>
	void gather2(const T* sendData, const size_t sendCount, std::vector<T>& recvData, const CommID rootCommID, const Context context){
	    Uri rootUri = getCommIDUri(context, rootCommID);
	    int rcounts[context.size()];
	    int rdispls[context.size()];
	    
	    int sendCountTmp = (int)sendCount;
	    
	    allGather(&(sendCountTmp), 1, rcounts, context);

	    unsigned offset  = 0;

	    for (unsigned i=0; i < context.size(); ++i) { 
	    	rdispls[i] = offset; 
	    	offset += rcounts[i];
		
	    } 
	    
	    const T* recvDataCollective = (T*) malloc(sizeof(T) * offset);
	    
	    MPI_Gatherv(const_cast<T*>(sendData), sendCount, MPIDatatypes<T>::type, 
			const_cast<T*>(recvDataCollective), rcounts, rdispls, MPIDatatypes<T>::type, 
			rootUri, contextMap[context.id]);


	    std::vector<T> recvDataTmp;

	    if(rootCommID == context.getCommID()){
		for(unsigned i = 0; i < offset; ++i){
		    recvDataTmp.push_back(recvDataCollective[i]);
		}
	    }

	    recvData = recvDataTmp;

	}

	template <typename T_Send, typename T_Recv>
	void allGather(const T_Send* sendData, const size_t sendCount, const T_Recv* recvData, const Context context){
	    MPI_Allgather(const_cast<T_Send*>(sendData), sendCount, MPIDatatypes<T_Send>::type, 
			  const_cast<T_Recv*>(recvData), sendCount, MPIDatatypes<T_Recv>::type, 
			  contextMap[context.id]);
	}

      // TODO
      // Equivalent to allGatherV
      // beautify
	template <typename T>
	void allGather2(const T* sendData, const size_t sendCount, std::vector<T>& recvData, const Context context){

	    int rcounts[context.size()];
	    int rdispls[context.size()];
	    
	    int sendCountTmp = (int)sendCount;
	    
	    allGather(&(sendCountTmp), 1, rcounts, context);

	    unsigned offset  = 0;

	    for (unsigned i=0; i < context.size(); ++i) { 
	    	rdispls[i] = offset; 
	    	offset += rcounts[i];
		
	    } 
	    
	    const T* recvDataCollective = (T*) malloc(sizeof(T) * offset);
	    
	    MPI_Allgatherv(const_cast<T*>(sendData), sendCount, MPIDatatypes<T>::type, 
			   const_cast<T*>(recvDataCollective), rcounts, rdispls, MPIDatatypes<T>::type, 
			   contextMap[context.id]);


	    std::vector<T> recvDataTmp;

	    for(unsigned i = 0; i < offset; ++i){
		recvDataTmp.push_back(recvDataCollective[i]);
	    }

	    recvData = recvDataTmp;

	}

	template <typename T_Send, typename T_Recv>
	void scatter(const T_Send* sendData, const size_t sendCount, T_Recv* recvData, const size_t recvCount, const CommID root, const Context context){
	    Uri rootUri = uriMap.at(context.id).at(root);
	    MPI_Scatter(const_cast<T_Send*>(sendData), sendCount, MPIDatatypes<T_Send>::type, 
			const_cast<T_Recv*>(recvData), recvCount, MPIDatatypes<T_Recv>::type, 
			rootUri, contextMap[context.id]);
	}

	template <typename T_Send, typename T_Recv>
	void allToAll(const T_Send* sendData, const size_t sendCount, T_Recv* recvData, const size_t recvCount, const Context context){
	    MPI_Alltoall(const_cast<T_Send*>(sendData), sendCount, MPIDatatypes<T_Send>::type, 
			 const_cast<T_Recv*>(recvData), recvCount, MPIDatatypes<T_Recv>::type, 
			 contextMap[context.id]);
	}
	
	template <typename T>
	void broadcast(const T* data, const size_t count, const CommID root, const Context context){
	    Uri rootUri = uriMap.at(context.id).at(root);
	    MPI_Bcast(const_cast<T*>(data), count, MPIDatatypes<T>::type, rootUri, contextMap[context.id]);
	}


	void synchronize(Context context){
	    MPI_Barrier(contextMap[context.id]);
	}

	/***************************************************************************
	 *
	 * ORGANISATION
	 *
	 ***************************************************************************/
	Context createContext(const std::vector<CommID> commIDs, const Context oldContext){
	    assert(commIDs.size() > 0);
	    MPI_Comm  newMPIContext;
	    MPI_Group oldGroup, newGroup;

	    // Translate commIDs to uris
	    std::vector<Uri> ranks;
	    for(CommID commID : commIDs){
	    	ranks.push_back(uriMap[oldContext.id][commID]);
	    }

	    // Create new context	    
	    MPI_Comm_group(contextMap[oldContext.id], &oldGroup);
	    MPI_Group_incl(oldGroup, ranks.size(), &(ranks[0]), &newGroup);
	    MPI_Comm_create(contextMap[oldContext.id], newGroup, &newMPIContext);

	    if(newMPIContext != MPI_COMM_NULL){
	    	Uri uri;
	    	MPI_Comm_rank(newMPIContext, &uri);
	    	Context newContext(++contextCount, uri, MPICommSize(newMPIContext));
	    	contextMap[newContext.id] = newMPIContext;

	    	// Update UriMap
	    	uriMap.insert(std::make_pair(newContext.id, std::map<CommID, Uri>()));
	    	Uri otherUris[newContext.size()];
	    	allGather(&uri, 1, otherUris, newContext);

	    	for(unsigned i = 0; i < newContext.size(); ++i){
		    uriMap[newContext.id][i] =  otherUris[i];
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

	CommID initialCommID(){
	    initMPI();
	    Uri uriTmp;
	    MPI_Comm_rank(MPI_COMM_WORLD, &uriTmp);
	    return (CommID)uriTmp;

	}

	size_t MPICommSize(MPI_Comm comm){
	    initMPI();
	    int n;
	    MPI_Comm_size(comm, &n);
	    return n;
	}

	void error(CommID commID, std::string msg){
	    using namespace dout;
	    Dout dout = Dout::getInstance();
	    dout(Flags::ERROR) << "[" << commID << "] " << msg;

	}

	inline Uri getCommIDUri(Context context, CommID commID){
	    Uri uri;
	    try {
		uri = uriMap.at(context.id).at(commID);

	    } catch(const std::out_of_range& e){
		std::stringstream errorStream;
		errorStream << "MPI::" << e.what()<< " : Communicator with ID " << commID << " is not part of the context " << context.id << std::endl;
		error(context.id, errorStream.str());
		exit(1);
	    }

	    return uri;
	}

    };


} // namespace CommunicationPolicy
