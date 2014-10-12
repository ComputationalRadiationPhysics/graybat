#pragma once
#include <dout.hpp>

#include <iostream>     /* std::cout */
#include <map>          /* std::map */
#include <exception>    /* std::out_of_range */
#include <sstream>      /* std::stringstream */
#include <algorithm>    /* std::transform */
#include <mpi.h>        /* MPI_* */



namespace CommunicationPolicy {

    /**
     * @brief Construction of MPI_Op from binary operation *Op* with type *T*
     *
     * @todo is it possible to get rid of this static shit ?
     */
    template<typename T, typename Op>
    class userOperation
    {
    public:
	explicit userOperation(Op& op){
	    bool isCommutative = true;
	    MPI_Op_create(&userOperation<T, Op>::perform, isCommutative, &_mpiOp);
	    _op = &op;
	}

	~userOperation(){
	    MPI_Op_free(&_mpiOp);
	}

	MPI_Op& getMpiOp(){
	    return _mpiOp;
	}

	static void perform(void* vinvec, void* voutvec, int* plen, MPI_Datatype*)
	{
	    T* invec = static_cast<T*>(vinvec);
	    T* outvec = static_cast<T*>(voutvec);
	    std::transform(invec, invec + *plen, outvec, outvec, *_op);
	}

    private:
	MPI_Op _mpiOp;
	static Op* _op;

    };

    template<typename T, typename Op> Op* userOperation<T, Op>::_op = 0;


   

    // Traits for typeconversion to MPI Types
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
	typedef unsigned VAddr;

	/**
	 * @brief A Context is a set of Communicators which are
	 *        able to communicate with each other. This class
	 *        stores meta information about the Context.
	 *
	 */
	class Context {
	    typedef unsigned ContextID;
	    typedef unsigned VAddr;
	    
	public:
	    Context() :
		id(0),
		commID(0),
		contextSize(0){}

	    Context(ContextID contextID, VAddr id, size_t contextSize) : 
		id(contextID),
		commID(id),
		contextSize(contextSize){}

	    Context& operator=(const Context& otherContext){
		id          = otherContext.getID();
		commID      = otherContext.getVAddr();
		contextSize = otherContext.size();
		return *this;

	    }

	    size_t size() const{
		return contextSize;
	    }

	    VAddr getVAddr() const {
		return commID;
	    }

	    ContextID getID() const {
		return id;
	    }

	    bool valid() const{
		return (id != 0);
	    }


	private:
	    ContextID id;
	    VAddr    commID;
	    size_t    contextSize;
	};

	
	/**
	 * @brief Events are returned by non-blocking or asynchrous functions.
	 *        Thus, events can be checked weather the the function has
	 *        finished or not.
	 *
	 */					\
	class Event {
	public:
	    Event(MPI_Request request) : request(request){

	    }

	    ~Event(){

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
	std::map<ContextID, std::map<VAddr, Uri> > uriMap;
	std::map<ContextID, MPI_Comm>               contextMap;

    protected:

	// Member
	Context initialContext;

	MPI() : contextCount(1), initialContext(contextCount, initialVAddr(), MPICommSize(MPI_COMM_WORLD)) {
	    contextMap[contextCount] = MPI_COMM_WORLD;

	    // Create Map VAddr -> uri
	    for(unsigned i = 0; i < initialContext.size(); ++i){
	    	uriMap[initialContext.getID()][i] = i;
	    }

	    std::cout << "Init MPI " << initialContext.getVAddr() << std::endl;
	    
	}

	~MPI(){
	    MPI_Barrier(MPI_COMM_WORLD);
	    MPI_Finalize();
	}

	/**************************************************************************
	 *
	 * POINT TO POINT COMMUNICATION
	 *
	 **************************************************************************/ 
	template <typename T, typename T_Context>
	Event asyncSendData(const T* const data, const size_t count, const VAddr destVAddr, const T_Context context, const MsgType msgType){
	    MPI_Request request;
	    Uri destUri = getVAddrUri(context, destVAddr);
	    MPI_Issend(const_cast<T*>(data), count, MPIDatatypes<T>::type, destUri, msgType, contextMap[context.getID()], &request);
	    return Event(request);


	}

	template <typename T, typename T_Context>
	Event asyncRecvData(const T*  data, const size_t count, const VAddr srcVAddr, const T_Context context, const MsgType msgType){
	    MPI_Request request;
	    Uri srcUri = getVAddrUri(context, srcVAddr);
	    MPI_Irecv(const_cast<T*>(data), count, MPIDatatypes<T>::type, srcUri, msgType, contextMap[context.getID()], &request);
	    return Event(request);

	}

	/**************************************************************************
	 *
	 * COLLECTIVE OPERATIONS
	 *
	 **************************************************************************/ 

	template <typename T, typename Op, typename T_Context>
	void reduce(const T* sendData, const T* recvData, const size_t count, Op op, const VAddr rootVAddr, const T_Context context){
	    userOperation<T, Op> mpiOp(op);
	    Uri rootUri = getVAddrUri(context, rootVAddr);
	    MPI_Reduce(const_cast<T*>(sendData), const_cast<T*>(recvData), count, MPIDatatypes<T>::type, mpiOp.getMpiOp(), rootUri, contextMap[context.getID()]);
	}

	template <typename T, typename Op, typename T_Context>
	void allReduce(const T* sendData, const T* recvData, const size_t count, Op op, const T_Context context){
	    userOperation<T, Op> mpiOp(op);
	    MPI_Allreduce(const_cast<T*>(sendData), const_cast<T*>(recvData), count, MPIDatatypes<T>::type, mpiOp.getMpiOp() , contextMap[context.getID()]);

	}

	template <typename T_Send, typename T_Recv, typename T_Context>
	void gather(const T_Send* sendData, const size_t sendCount, const T_Recv* recvData, const size_t recvCount, const VAddr rootVAddr, const T_Context context){
	    Uri rootUri = getVAddrUri(context, rootVAddr);

	    MPI_Gather(const_cast<T_Send*>(sendData), sendCount, MPIDatatypes<T_Send>::type, 
		       const_cast<T_Recv*>(recvData), recvCount, MPIDatatypes<T_Recv>::type, 
		       rootUri, contextMap[context.getID()]);
	}

	template <typename T, typename T_Context>
	void gather2(const T* sendData, const size_t sendCount, std::vector<T>& recvData, std::vector<unsigned>& recvCount, const VAddr rootVAddr, const T_Context context){
	    Uri rootUri = getVAddrUri(context, rootVAddr);
	    int rcounts[context.size()];
	    int rdispls[context.size()];

	    // Create recv buffer with sendsize information of other ranks
	    int sendCountTmp = (int)sendCount;
	    allGather(&(sendCountTmp), 1, rcounts, context);
	    unsigned offset  = 0;
	    for (unsigned i=0; i < context.size(); ++i) { 
	    	rdispls[i] = offset; 
	    	offset += rcounts[i];
		
	    } 

	    const T* recvDataCollective = (T*) malloc(sizeof(T) * offset);

	    // Receive data with varying size
	    MPI_Gatherv(const_cast<T*>(sendData), sendCount, MPIDatatypes<T>::type, 
			const_cast<T*>(recvDataCollective), rcounts, rdispls, MPIDatatypes<T>::type, 
			rootUri, contextMap[context.getID()]);

	    if(rootVAddr == context.getVAddr()){
		for(unsigned i = 0; i < offset; ++i){
		    recvData.push_back(recvDataCollective[i]);
		}
		
		for(unsigned i = 0; i < context.size(); ++i){
		    recvCount.push_back(rcounts[i]);
		}
	    }

	}

	template <typename T_Send, typename T_Recv, typename T_Context>
	void allGather(const T_Send* sendData, const size_t sendCount, const T_Recv* recvData, const T_Context context){
	    MPI_Allgather(const_cast<T_Send*>(sendData), sendCount, MPIDatatypes<T_Send>::type, 
			  const_cast<T_Recv*>(recvData), sendCount, MPIDatatypes<T_Recv>::type, 
			  contextMap[context.getID()]);
	}

      // TODO
      // Equivalent to allGatherV
      // beautify
	template <typename T, typename T_Context>
	void allGather2(const T* sendData, const size_t sendCount, std::vector<T>& recvData, std::vector<unsigned>& recvCount ,const T_Context context){

	    int rcounts[context.size()];
	    int rdispls[context.size()];
	    
	    int sendCountTmp = (int)sendCount;
	    
	    allGather(&(sendCountTmp), 1, rcounts, context);

	    unsigned offset  = 0;

	    for (unsigned i=0; i < context.size(); ++i) { 
	    	rdispls[i] = offset; 
	    	offset += rcounts[i];
		
	    }

	    for(unsigned i = 0; i < context.size(); ++i){
		recvCount.push_back(rcounts[i]);
	    }
	    
	    const T* recvDataCollective = (T*) malloc(sizeof(T) * offset);
	    
	    MPI_Allgatherv(const_cast<T*>(sendData), sendCount, MPIDatatypes<T>::type, 
			   const_cast<T*>(recvDataCollective), rcounts, rdispls, MPIDatatypes<T>::type, 
			   contextMap[context.getID()]);


	    std::vector<T> recvDataTmp;

	    for(unsigned i = 0; i < offset; ++i){
		recvDataTmp.push_back(recvDataCollective[i]);
	    }

	    recvData = recvDataTmp;

	}

	template <typename T_Send, typename T_Recv, typename T_Context>
	void scatter(const T_Send* sendData, const size_t sendCount, T_Recv* recvData, const size_t recvCount, const VAddr root, const T_Context context){
	    Uri rootUri = uriMap.at(context.getID()).at(root);
	    MPI_Scatter(const_cast<T_Send*>(sendData), sendCount, MPIDatatypes<T_Send>::type, 
			const_cast<T_Recv*>(recvData), recvCount, MPIDatatypes<T_Recv>::type, 
			rootUri, contextMap[context.getID()]);
	}

	template <typename T_Send, typename T_Recv, typename T_Context>
	void allToAll(const T_Send* sendData, const size_t sendCount, T_Recv* recvData, const size_t recvCount, const T_Context context){
	    MPI_Alltoall(const_cast<T_Send*>(sendData), sendCount, MPIDatatypes<T_Send>::type, 
			 const_cast<T_Recv*>(recvData), recvCount, MPIDatatypes<T_Recv>::type, 
			 contextMap[context.getID()]);
	}
	
	template <typename T, typename T_Context>
	void broadcast(const T* data, const size_t count, const VAddr root, const T_Context context){
	    Uri rootUri = uriMap.at(context.getID()).at(root);
	    MPI_Bcast(const_cast<T*>(data), count, MPIDatatypes<T>::type, rootUri, contextMap[context.getID()]);
	}


	template <typename T_Context>
	void synchronize(T_Context context){
	    MPI_Barrier(contextMap[context.getID()]);
	}

	/***************************************************************************
	 *
	 * ORGANISATION
	 *
	 ***************************************************************************/
	template <typename T_Context>
	T_Context createContext(const std::vector<VAddr> commIDs, const T_Context oldContext){
	    assert(commIDs.size() > 0);
	    MPI_Comm  newMPIContext;
	    MPI_Group oldGroup, newGroup;

	    // Translate commIDs to uris
	    std::vector<Uri> ranks;
	    for(VAddr commID : commIDs){
	    	ranks.push_back(getVAddrUri(oldContext, commID));
	    }

	    // Create new context	    
	    MPI_Comm_group(contextMap[oldContext.getID()], &oldGroup);
	    MPI_Group_incl(oldGroup, ranks.size(), &(ranks[0]), &newGroup);
	    MPI_Comm_create(contextMap[oldContext.getID()], newGroup, &newMPIContext);

	    if(newMPIContext != MPI_COMM_NULL){
	    	Uri uri;
	    	MPI_Comm_rank(newMPIContext, &uri);
	    	T_Context newContext(Context(++contextCount, uri, MPICommSize(newMPIContext)));
	    	contextMap[newContext.getID()] = newMPIContext;

	    	// Update UriMap
	    	uriMap.insert(std::make_pair(newContext.getID(), std::map<VAddr, Uri>()));
	    	Uri otherUris[newContext.size()];
	    	allGather(&uri, 1, otherUris, newContext);

	    	for(unsigned i = 0; i < newContext.size(); ++i){
	    	    uriMap[newContext.getID()][i] =  otherUris[i];
	    	}
	    	return newContext;

	    }
	    else {
		// return invalid context
		// for peers not anymore included
	    	return T_Context(Context());
		
	    }
	    
	}

    private:
	void initMPI(){
	    int flag = 0;
	    MPI_Initialized(&flag);
	    if(!flag){
		int argc;
		char **argv;
		MPI_Init(&argc, &argv);
	    }
	}

	VAddr initialVAddr(){
	    initMPI();
	    Uri uriTmp;
	    MPI_Comm_rank(MPI_COMM_WORLD, &uriTmp);
	    return (VAddr)uriTmp;

	}

	size_t MPICommSize(MPI_Comm comm){
	    initMPI();
	    int n;
	    MPI_Comm_size(comm, &n);
	    return n;
	}

	void error(VAddr commID, std::string msg){
	    using namespace dout;
	    Dout dout = Dout::getInstance();
	    dout(Flags::ERROR) << "[" << commID << "] " << msg;

	}

	template <typename T_Context>
	inline Uri getVAddrUri(T_Context context, VAddr commID){
	    Uri uri;
	    try {
		uri = uriMap.at(context.getID()).at(commID);

	    } catch(const std::out_of_range& e){
		std::stringstream errorStream;
		errorStream << "MPI::getVAddrUri::" << e.what()<< " : Communicator with ID " << commID << " is not part of the context " << context.getID() << std::endl;
		error(context.getID(), errorStream.str());
		exit(1);
	    }

	    return uri;
	}

    };

} // namespace CommunicationPolicy
