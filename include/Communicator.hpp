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
    // template <typename T_Container>
    // class Channel {


    // public:
    // 	typedef typename T_Container::value_type value_type;

    // 	Channel(const Node contact, 
    // 		const unsigned channelType, 
    // 		const Context context,
    // 		T_Container &container
    // 		) :
    // 	    contact(contact), channelType(channelType), context(context), container(container){
	
    // 	}

    // 	size_t size() const {
    // 	    return container.size();
    // 	}

    // 	const value_type*  data() const {
    // 	    return container.data();
    // 	}

    // 	const Node contact;
    // 	const unsigned channelType;
    // 	const Context context;
    // private:
    // 	T_Container &container;


    // };

    /*********************************
     * Inner CollectiveChannel Class
     *********************************/
    // template <typename T_ContainerSend, typename T_ContainerRecv>
    // class CollectiveChannel {
    // public:
    // 	typedef typename T_ContainerSend::value_type value_type_send;
    // 	typedef typename T_ContainerRecv::value_type value_type_recv;

    // 	CollectiveChannel( T_ContainerSend &sendContainer, 
    // 			   T_ContainerRecv &recvContainer, 
    // 			   const Node root, 
    // 			   const Context context) :
    // 	    root(root), context(context), sendContainer(sendContainer), recvContainer(recvContainer) {
    // 	}

    // 	CollectiveChannel( T_ContainerSend &sendContainer, 
    // 			   T_ContainerRecv &recvContainer, 
    // 			   const Context context) :
    // 	    root(0), context(context), sendContainer(sendContainer), recvContainer(recvContainer) {
    // 	}


    // 	size_t sendSize() const {
    // 	    return sendContainer.size();
    // 	}

    // 	size_t recvSize() const {
    // 	    return recvContainer.size();
    // 	}

    // 	const value_type_send*  sendData() const {
    // 	    return sendContainer.data();
    // 	}

    // 	const value_type_recv*  recvData() const {
    // 	    return recvContainer.data();
    // 	}

    // 	const Node root;
    // 	const Context context;

    // private:
    // 	T_ContainerSend &sendContainer;
    // 	T_ContainerRecv &recvContainer;

    // };


    // Communicator() : CommunicationPolicy(){
    // 	contextMap.insert(std::make_pair(getInitialContext().getContextUUID(), std::map<NodeUUID, CommUUID>()));

    // }


    /***************************************************************************
     *
     * POINT TO POINT COMMUNICATION
     *
     ***************************************************************************/
    // TODO
    // Remove channels ?

    // template <typename T>
    // void send(Channel<T> channel){
    // 	Event e = asyncSend(channel);
    // 	e.wait();
    // }

    template <typename T>
    void send(const Node destNode, const unsigned tag, const Context context, T& sendData){
	Event e = asyncSend(destNode, tag, context, sendData);
	e.wait();
    }

    // template <typename T>
    // Event asyncSend(Channel<T> channel){
    // 	CommUUID destCommUUID = contextMap.at(channel.context.getContextUUID()).at(channel.contact.uuid);
    // 	return CommunicationPolicy::asyncSendData(channel.data(), channel.size(), destCommUUID, channel.context, channel.channelType);
    // }

    template <typename T>
    Event asyncSend(const Node destNode, const unsigned tag, const Context context, T& sendData){
	CommUUID destCommUUID = contextMap.at(context.getContextUUID()).at(destNode.uuid);
	return CommunicationPolicy::asyncSendData(sendData.data(), sendData.size(), destCommUUID, context, tag);

	//return asyncSend(Channel<T>(destNode, tag, context, sendData));
    }


    // template <typename T>
    // void recv(Channel<T> channel){
    // 	Event e = asyncRecv(channel);
    // 	e.wait();
    // }

    template <typename T>
    void recv(const Node srcNode, const unsigned tag, const Context context, T& recvData){
	Event e =  asyncRecv(srcNode, tag, context, recvData);
	e.wait();
    }

    // template <typename T>
    // Event asyncRecv(Channel<T> channel){
    // 	CommUUID srcCommUUID = contextMap.at(channel.context.getContextUUID()).at(channel.contact.uuid);
    // 	return CommunicationPolicy::asyncRecvData(channel.data(), channel.size(), srcCommUUID, channel.context, channel.channelType);

    // }

    template <typename T>
    Event asyncRecv(const Node srcNode, const unsigned tag, const Context context, const T& recvData){
	CommUUID srcCommUUID = contextMap.at(context.getContextUUID()).at(srcNode.uuid);
	return CommunicationPolicy::asyncRecvData(recvData.data(), recvData.size(), srcCommUUID, context, tag);
    }


    /**************************************************************************
     *
     * COLLECTIVE OPERATIONS
     *
     **************************************************************************/ 
    // TODO 
    // Make collective interfaces more slim
    template <typename T_Send, typename T_Recv>
    void gather(const CommUUID rootCommUUID, const Context context, const T_Send& sendData, T_Recv& recvData){
    	CommunicationPolicy::gather(sendData.data(), sendData.size(), recvData.data(), sendData.size(), rootCommUUID, context);
    }

    template <typename T_Send, typename T_Recv>
    void allGather(const Context context, const T_Send& sendData, const T_Recv& recvData){
    	CommunicationPolicy::allGather(sendData.data(), sendData.size(), recvData.data(), sendData.size(), context);
    }


    template <typename T_Send, typename T_Recv>
    void scatter(const CommUUID rootCommUUID, const Context context, const T_Send& sendData, const T_Recv& recvData){
    	CommunicationPolicy::gather(sendData.data(), sendData.size(), recvData.data(), recvData.size(), rootCommUUID, context);
    }

    template <typename T_Send, typename T_Recv>
    void allToAll(const Context context, const T_Send& sendData, const T_Recv& recvData){
    	CommunicationPolicy::allToAll(sendData.data(), sendData.size(), recvData.data(), recvData.size(), context);
    }

    // TODO 
    // Fill with various binary functions
    template <typename T>
    void reduce(const CommUUID rootCommUUID, const Context context, const BinaryOperation op, const T& sendData, const T& recvData){
     	CommunicationPolicy::reduce(sendData.data(), recvData.data(), sendData.size(), op, rootCommUUID, context);
    }

    template <typename T>
    void allReduce(const Context context, const BinaryOperation op, const T& sendData, T& recvData){
	CommunicationPolicy::allReduce(sendData.data(), recvData.data(), sendData.size(), op, context);
    }

    template <typename T>
    void broadcast(const NodeUUID rootCommUUID, const Context context, const T& data){
	CommunicationPolicy::broadcast(data.data(), data.size(), rootCommUUID, context);
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
    	//assert(nodes.size() > 0);

	// Each announces how many nodes it manages
	std::array<unsigned, 1> nodeCount {{(unsigned) nodes.size()}};
	std::array<unsigned, 1> maxNodes  {{0}};
	allReduce(context, BinaryOperations::MAX, nodeCount, maxNodes);
	 
	
    	for(unsigned i = 0; i < maxNodes[0]; ++i){
    	    const size_t contextSize = context.size();
	    std::array<int, 1> sendData{{-1}};
	    std::vector<int> recvData(contextSize);

    	    if(i < nodes.size()){
    	    	sendData[0] = nodes.at(i).uuid;
    	    }
    	    else{
    	    	sendData[0] = -1;
    	    }

    	    allGather(context, sendData, recvData);

    	    for(unsigned j = 0; j < contextSize; ++j){
    	    	if(recvData[j] != -1){
		    contextMap[context.getContextUUID()][recvData[j]] = j;
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

