#pragma once
#include <string>
#include <utility>
#include <map>
#include <utility>
#include <functional>
#include <array>      /* array */
#include <memory>     /* unique_ptr */
#include <assert.h>   /* assert */


template <class T_CommunicationPolicy, typename T_Node>
class Communicator : public T_CommunicationPolicy {
private:
    typedef T_Node                                         Node;
    typedef T_CommunicationPolicy                          CommunicationPolicy;
    typedef typename Node::UUID                            NodeUUID;
    typedef typename CommunicationPolicy::ContextUUID      ContextUUID;
    typedef typename CommunicationPolicy::CommUUID         CommUUID;
    typedef typename CommunicationPolicy::BinaryOperation  BinaryOperation;


public:
    typedef typename CommunicationPolicy::Event            Event;
    typedef typename CommunicationPolicy::BinaryOperations  BinaryOperations;
    typedef typename CommunicationPolicy::Context           Context;

private:
    std::map<ContextUUID, std::map<NodeUUID, CommUUID>> contextMap;

public:

    /**************************
     * Inner Channel Class
     **************************/
    template <typename T_Container>
    class Channel {
    public:
	typedef typename T_Container::value_type value_type;

	Channel(const Node src, 
		const Node dest, 
		T_Container &container, 
		const unsigned channelType, 
		const Context context
		) :
	    src(src), dest(dest), channelType(channelType), context(context), container(container){
	
	}

	size_t size() const {
	    return container.size();
	}

	const value_type*  data() const {
	    return container.data();
	}

	const Node src;
	const Node dest;
	const unsigned channelType;
	const Context context;
    private:
	T_Container &container;


    };

    /*********************************
     * Inner CollectiveChannel Class
     *********************************/
    template <typename T_ContainerSend, typename T_ContainerRecv>
    class CollectiveChannel {
    public:
	typedef typename T_ContainerSend::value_type value_type_send;
	typedef typename T_ContainerRecv::value_type value_type_recv;

	CollectiveChannel( T_ContainerSend &sendContainer, 
			   T_ContainerRecv &recvContainer, 
			   const Node root, 
			   const Context context) :
	    root(root), context(context), sendContainer(sendContainer), recvContainer(recvContainer) {
	}

	size_t sendSize() const {
	    return sendContainer.size();
	}

	size_t recvSize() const {
	    return recvContainer.size();
	}

	const value_type_send*  sendData() const {
	    return sendContainer.data();
	}

	const value_type_recv*  recvData() const {
	    return recvContainer.data();
	}

	const Node root;
	const Context context;

    private:
	T_ContainerSend &sendContainer;
	T_ContainerRecv &recvContainer;

    };


    Communicator() : CommunicationPolicy(){
	contextMap.insert(std::make_pair(getInitialContext().getContextUUID(), std::map<NodeUUID, CommUUID>()));

    }



    /***************************************************************************
     *
     * POINT TO POINT COMMUNICATION
     *
     ***************************************************************************/
    template <typename T>
    void send(Channel<T> channel){
	Event e = asyncSend(channel);
	e.wait();
    }

    template <typename T>
    Event asyncSend(Channel<T> channel){
	CommUUID destURI = contextMap.at(channel.context.getContextUUID()).at(channel.dest.uuid);
	return CommunicationPolicy::asyncSendData(channel.data(), channel.size(), destURI, channel.context, channel.channelType);
    }

    template <typename T>
    void recv(Channel<T> channel){
    	Event e = asyncRecv(channel);
    	e.wait();
    }

    // TODO
    // Is an interface without channels better ?
    template <typename T>
    void recv(const Node srcNode, const unsigned tag, const Context context, T& data){
	CommUUID srcURI = contextMap.at(context.getContextUUID()).at(srcNode.uuid);
	Event e =  CommunicationPolicy::asyncRecvData(data.data(), data.size(), srcURI, context, tag);
	e.wait();

    }
    
    // TODO
    // Can there be a default context ?
    template <typename T>
    void recv(const Node srcNode, const unsigned tag, T& data){
	CommUUID srcURI = contextMap.at(getInitialContext().getContextUUID()).at(srcNode.uuid);
	Event e =  CommunicationPolicy::asyncRecvData(data.data(), data.size(), srcURI, getInitialContext(), tag);
	e.wait();

    }


    template <typename T>
    Event asyncRecv(Channel<T> channel){
	typedef typename T::value_type value_type;
	CommUUID srcURI = contextMap.at(channel.context.getContextUUID()).at(channel.src.uuid);
	return CommunicationPolicy::asyncRecvData(const_cast<value_type*>(channel.data()), channel.size(), srcURI, channel.context, channel.channelType);

    }

    /**************************************************************************
     *
     * COLLECTIVE OPERATIONS
     *
     **************************************************************************/ 
    // TODO 
    // Make collective interfaces more slim
    // ==> because some sizes, data not needed

    template <typename T_Send, typename T_Recv>
    void gather(const CollectiveChannel<T_Send, T_Recv> channel){
	typedef typename T_Recv::value_type recv_value_type;
     	CommUUID rootURI = contextMap.at(channel.context.getContextUUID()).at(channel.root.uuid);
    	CommunicationPolicy::gather(channel.sendData(), channel.sendSize(), const_cast<recv_value_type*>(channel.recvData()), channel.recvSize(), rootURI, channel.dest);
    }

    template <typename T_Send, typename T_Recv>
    void allGather(const CollectiveChannel<T_Send, T_Recv> channel){
	typedef typename T_Recv::value_type recv_value_type;
    	CommunicationPolicy::allGather(channel.sendData(), channel.sendSize(), const_cast<recv_value_type*>(channel.recvData()), channel.sendSize(), channel.context);

    }

    template <typename T_Send, typename T_Recv>
    void scatter(const CollectiveChannel<T_Send, T_Recv> channel){
	typedef typename T_Recv::value_type recv_value_type;
     	CommUUID rootURI = contextMap.at(channel.context.getContextUUID()).at(channel.root.uuid);
    	CommunicationPolicy::gather(channel.sendData(), channel.sendSize(), const_cast<recv_value_type*>(channel.recvData()), channel.recvSize(), rootURI, channel.dest);
    }

    template <typename T_Send, typename T_Recv>
    void allToAll(const CollectiveChannel<T_Send, T_Recv> channel){
	typedef typename T_Recv::value_type recv_value_type;
    	CommunicationPolicy::allToAll(channel.sendData(), channel.sendSize(), const_cast<recv_value_type*>(channel.recvData()), channel.recvSize(), channel.context);
    }

    template <typename T_Send, typename T_Recv>
    void reduce(const CollectiveChannel<T_Send, T_Recv> channel, const BinaryOperation op){
	typedef typename T_Recv::value_type recv_value_type;
     	CommUUID rootURI = contextMap.at(channel.context.getContextUUID()).at(channel.root.uuid);
     	CommunicationPolicy::reduce(channel.sendData(), const_cast<recv_value_type*>(channel.recvData()), channel.sendSize(), op, rootURI, channel.context);
    }

    template <typename T_Send, typename T_Recv>
    void allReduce(const CollectiveChannel<T_Send, T_Recv> &channel, const BinaryOperation op){
	typedef typename T_Recv::value_type recv_value_type;
	CommunicationPolicy::allReduce(channel.sendData(), const_cast<recv_value_type*>(channel.recvData()), channel.sendSize(), op, channel.context);
    }

    template <typename T_Send, typename T_Recv>
    void broadcast(const CollectiveChannel<T_Send, T_Recv> channel){
    	typedef typename T_Send::value_type send_value_type;
    	typedef typename T_Recv::value_type recv_value_type;
     	CommUUID rootUUID = contextMap.at(channel.context.getContextUUID()).at(channel.root.uuid);
    	CommUUID ownUUID  = channel.context.getCommUUID();
    	if(rootUUID == ownUUID){
    	    CommunicationPolicy::broadcast(const_cast<send_value_type*>(channel.sendData()), channel.sendSize(), rootUUID, channel.context);
    	}
    	else {
    	    CommunicationPolicy::broadcast(const_cast<recv_value_type*>(channel.recvData()), channel.recvSize(), rootUUID, channel.context);
    	}
    }

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

	// Each announces how many nodes it manages
	typedef std::array<unsigned, 1> reduceType;
	reduceType nodeCount {{(unsigned) nodes.size()}};
	reduceType maxNodes  {{0}};
    	CollectiveChannel<reduceType, reduceType> reduceChannel(nodeCount, maxNodes, nodes.at(0), context);
    	allReduce(reduceChannel, BinaryOperations::MAX);

	
    	for(unsigned i = 0; i < maxNodes[0]; ++i){
    	    const size_t contextSize = context.size();
    	    const size_t sendCount = 1;
	    typedef std::vector<int> channelType;
	    channelType sendData(sendCount);
	    channelType recvData(contextSize);

    	    CollectiveChannel<channelType, channelType> gatherChannel(sendData, recvData, nodes.at(0), context);
    	    if(i < nodes.size()){
    		sendData[0] = nodes.at(i).uuid;
    	    }
    	    else{
    		sendData[0] = -1;
    	    }

    	    allGather(gatherChannel);

    	    for(unsigned j = 0; j < contextSize; ++j){
    		if(recvData[j] != -1){
    		    contextMap.at(context.getContextUUID()).insert(std::make_pair(j, recvData[j]));
    		}
    	    }

    	}

    }

    void announce(const Node node, const Context context){
	std::vector<Node> nodes;
	nodes.push_back(node);
	announce(nodes, context);
    }

    Context getContext(std::vector<Node> nodes, Context oldContext){
	std::vector<CommUUID> uuids;
	for(Node node : nodes){
	    uuids.push_back(contextMap.at(oldContext.getContextUUID()).at(node.uuid));
	}

	Context newContext = CommunicationPolicy::createContext(uuids, oldContext);

	contextMap.insert(std::make_pair(newContext.getContextUUID(), std::map<NodeUUID, CommUUID>()));
	return newContext;
    }

    Context getInitialContext(){
	return CommunicationPolicy::initialContext;
    }


};

