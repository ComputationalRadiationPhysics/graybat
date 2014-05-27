#pragma once
#include <string>
#include <utility>
#include <map>
#include <utility>
#include <functional>
#include <memory> /* unique_ptr */
#include <assert.h> /* assert */


template <class T_CommunicationPolicy, typename T_Node>
class Communicator : public T_CommunicationPolicy {
private:
    typedef T_Node                                         Node;
    typedef T_CommunicationPolicy                          CommunicationPolicy;
    //typedef typename CommunicationPolicy::URI              URI;
    typedef unsigned                                       URI;
    typedef typename Node::UUID                            UUID;
    typedef typename CommunicationPolicy::BinaryOperation  BinaryOperation;

public:
    typedef typename CommunicationPolicy::BinaryOperations  BinaryOperations;
    typedef typename CommunicationPolicy::Context           Context;

private:
    std::map<Context, std::map<UUID, URI>> contextMap;

public:
    template <typename T = char>
    struct Channel {
	Channel(const Node src, const Node dest, T* data, const size_t size, const unsigned channelType, const Context context) :
	    src(src), dest(dest), data(data), size(size), channelType(channelType), context(context){}


	const Node src;
	const Node dest;
	T* data;
	const size_t size;
	const unsigned channelType;
	const Context context;

    };

    template <typename T = char>
    struct CollectiveChannel {
	CollectiveChannel( T* sendData, T* recvData, const size_t size, const Node root, const Context context) :
	    sendData(sendData), recvData(recvData), size(size),  root(root), context(context){}

	T* sendData;
	T* recvData;
	const size_t size;
	const Node root;
	const Context context;

    };

    Communicator() : CommunicationPolicy(){
	contextMap.insert(std::make_pair(getInitialContext(), std::map<UUID, URI>()));

    }

    /***************************************************************************
     *
     * POINT TO POINT COMMUNICATION
     *
     ***************************************************************************/
    void send(Channel<char> channel){
	URI destURI = contextMap.at(channel.context).at(channel.dest.uuid);
	CommunicationPolicy::asyncSendData(channel.data, channel.size, destURI, channel.context, channel.channelType);
    }

    void recv(Channel<char> channel){
	URI srcURI = contextMap.at(channel.context).at(channel.src.uuid);
	CommunicationPolicy::recvData(channel.data, channel.size, srcURI, channel.context, channel.channelType);
    }

    /**************************************************************************
     *
     * COLLECTIVE OPERATIONS
     *
     **************************************************************************/ 
    template <typename T>
    void gather(const CollectiveChannel<T> channel){
     	URI rootURI = contextMap[channel.context][channel.root.uuid];
	CommunicationPolicy::gather(channel.sendData, channel.size, channel.recvData, channel.size, rootURI, channel.dest);
    }

    template <typename T>
    void allGather(const CollectiveChannel<T> channel){
	CommunicationPolicy::allGather(channel.sendData, channel.size, channel.recvData, channel.size, channel.context);

    }

    template <typename T>
    void scatter(const CollectiveChannel<T> channel){
     	URI rootURI = contextMap[channel.context][channel.root.uuid];
	CommunicationPolicy::gather(channel.sendData, channel.size, channel.recvData, channel.size, rootURI, channel.dest);
    }

    template <typename T>
    void allToAll(const CollectiveChannel<T> channel){
	CommunicationPolicy::allToAll(channel.sendData, channel.size, channel.recvData, channel.size, channel.context);
    }

    template <typename T>
    void reduce(const CollectiveChannel<T> channel, const BinaryOperation op){
     	URI rootURI = contextMap[channel.context][channel.root.uuid];
     	CommunicationPolicy::reduce(channel.sendData, channel.recvData, channel.size, op, rootURI, channel.context);
     }

    template <typename T>
    void allReduce(const CollectiveChannel<T> channel, const BinaryOperation op){
	CommunicationPolicy::allReduce(channel.sendData, channel.recvData, channel.size, op, channel.context);

    }

    template <typename T>
    void broadcast(const CollectiveChannel<T> channel){
     	URI rootURI = contextMap[channel.context][channel.root.uuid];
	CommunicationPolicy::broadcast(channel.sendData, channel.size, channel.recvData, channel.size, rootURI, channel.context);
    }

    template <typename T>
    void synchronize(const Context context){
	CommunicationPolicy::synchronize(context);
    }




    /***************************************************************************
     *
     * ORGANISATION
     *
     ***************************************************************************/
    void announce(const std::vector<Node> nodes, const Context context){
	assert(nodes.size() > 0);
	unsigned count = nodes.size();
	unsigned maxNodes = 0;
	CollectiveChannel<unsigned> reduceChannel(&count, &maxNodes, 1, nodes.at(0), context);
	allReduce(reduceChannel, BinaryOperations::MAX);

	for(unsigned i = 0; i < maxNodes; ++i){
	    const size_t contextSize = CommunicationPolicy::getContextSize(context);
	    const size_t sendCount = 1;
	    int sendData;
	    int recvData[contextSize];

	    CollectiveChannel<int> gatherChannel(&sendData, recvData, sendCount, nodes.at(0), context);
	    if(i < nodes.size()){
		gatherChannel.sendData[0] = nodes.at(i).uuid;
	    }
	    else{
		gatherChannel.sendData[0] = -1;
	    }

	    allGather(gatherChannel);

	    for(unsigned j = 0; j < contextSize; ++j){
		if(gatherChannel.recvData[j] != -1){
		    contextMap.at(context).insert(std::make_pair(j, gatherChannel.recvData[j]));
		}
	    }

	}

    }

    Context getContext(std::vector<Node> nodes, Context oldContext){
	std::map<UUID, URI> uriMap = contextMap.at(oldContext);
	std::vector<URI> uris;
	for(Node node : nodes){
	    uris.push_back(uriMap[node.uuid]);
	}

	Context newContext = CommunicationPolicy::createContext(uris, oldContext);
	contextMap.insert(std::make_pair(newContext, std::map<UUID, URI>()));
	return newContext;
    }

    Context getInitialContext(){
	return CommunicationPolicy::initialContext;
    }

    URI getCommunicatorID(Context context){
	return CommunicationPolicy::uri.at(context);
    }


};

