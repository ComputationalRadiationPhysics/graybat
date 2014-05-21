#pragma once
#include <string>
#include <utility>
#include <map>
#include <utility>
#include <functional>


template <class T_CommunicationPolicy, typename T_Node>
class Communicator : public T_CommunicationPolicy {
private:
    typedef T_Node                                         Node;
    typedef T_CommunicationPolicy                          CommunicationPolicy;
    typedef typename CommunicationPolicy::URI              URI;
    typedef typename Node::UUID                            UUID;

public:
    typedef typename CommunicationPolicy::BinaryOperation  BinaryOperation;
    typedef typename CommunicationPolicy::Context Context;

private:
    std::map<Context, std::map<UUID, URI>> contextMap;

public:
    template <typename T_Data = char>
    struct Channel {
	Channel(Node src, Node dest, size_t size, T_Data* data, Context context) :
	    src(src), dest(dest), size(size), data(data), context(context){}

	Node src;
	Node dest;
	size_t size;
	T_Data* data;
	Context context;

    };

    template <typename T_Data = char>
    struct MultiChannel {
	Context context;
	T_Data* sendData;
	T_Data* recvData;
	size_t size;
	
    };

    Communicator() : CommunicationPolicy(){

    }

    /***************************************************************************
     *
     * POINT TO POINT COMMUNICATION
     *
     ***************************************************************************/
    void send(Channel<char> channel, std::function<void()> callback){
	const unsigned descr = 0;
	URI destURI = contextMap[channel.context][channel.dest.uuid];
	CommunicationPolicy::sendData(channel.data, channel.size, destURI, channel.context, descr, callback);
    }

    void recv(Channel<char> channel, std::function<void()> callback){
	const unsigned descr = 0;
	URI srcURI = contextMap[channel.context][channel.src.uuid];
	CommunicationPolicy::recvData(channel.data, channel.size, srcURI, channel.context, descr, callback);
    }

    /**************************************************************************
     *
     * COLLECTIVE OPERATIONS
     *
     **************************************************************************/ 
    void broadcast(const MultiChannel<char> channel, const Node rootNode){

    }

    void gather(){

    }

    void scatter(){

    }

    //AllToAll

    void reduce(const MultiChannel<char> channel, const Node rootNode, const BinaryOperation op){
	URI rootURI = contextMap[channel.context][rootNode.uuid];
	CommunicationPolicy::reduce(channel.sendData, channel.recvData, channel.size, op, rootURI, channel.context);
    }

    void scan(){

    }
    /***************************************************************************
     *
     * ORGANISATION
     *
     ***************************************************************************/
    void announce(const Node node, const Context context){
	std::cout << "Node " << node.uuid << " available on " << CommunicationPolicy::uri[context] << std::endl;
	contextMap[context].insert(std::pair<UUID, URI>(node.uuid, CommunicationPolicy::uri[context]));

    }

    Context getContext(std::vector<Node> nodes, Context context){
	std::map<UUID, URI> uriMap = contextMap[context];
	std::vector<URI> uris;
	for(Node node : nodes){
	    uris.push_back(uriMap[node.uuid]);
	}

	return CommunicationPolicy::createContext(uris, context);
    }

    Context getContext(){
	return CommunicationPolicy::initialContext;
    }

    int getCommunicatorID(Context context){
	return CommunicationPolicy::uri[context];
    }


};

