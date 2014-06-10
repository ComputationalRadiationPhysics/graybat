#pragma once
#include <string>
#include <utility>
#include <map>
#include <utility>
#include <functional>
#include <array>      /* array */
#include <assert.h>   /* assert */


template <class T_CommunicationPolicy, typename T_Node>
class Communicator : public T_CommunicationPolicy {
private:
    typedef T_Node                                         Node;
    typedef T_CommunicationPolicy                          CommunicationPolicy;
    typedef unsigned                                       NodeID;
    typedef typename CommunicationPolicy::ContextID      ContextID;
    typedef unsigned                                       CommID;
    typedef typename CommunicationPolicy::BinaryOperation  BinaryOperation;


public:
    typedef typename CommunicationPolicy::Event             Event;
    typedef typename CommunicationPolicy::BinaryOperations  BinaryOperations;
    typedef typename CommunicationPolicy::Context           Context;

private:
    // TODO
    // Remove ContextMap from Communicator
    // Put into own class
    std::map<ContextID, std::map<NodeID, CommID>> contextMap;

public:

    /***************************************************************************
     *
     * POINT TO POINT COMMUNICATION
     *
     ***************************************************************************/
    template <typename T>
    void send(const Node destNode, const unsigned tag, const Context context, T& sendData){
	Event e = asyncSend(destNode, tag, context, sendData);
	e.wait();
    }

    template <typename T>
    Event asyncSend(const Node destNode, const unsigned tag, const Context context, T& sendData){
	CommID destCommID = contextMap.at(context.getContextID()).at(destNode.uuid);
	return CommunicationPolicy::asyncSendData(sendData.data(), sendData.size(), destCommID, context, tag);
    }


    template <typename T>
    void recv(const Node srcNode, const unsigned tag, const Context context, T& recvData){
	Event e =  asyncRecv(srcNode, tag, context, recvData);
	e.wait();
    }

    template <typename T>
    Event asyncRecv(const Node srcNode, const unsigned tag, const Context context, const T& recvData){
	CommID srcCommID = contextMap.at(context.getContextID()).at(srcNode.uuid);
	return CommunicationPolicy::asyncRecvData(recvData.data(), recvData.size(), srcCommID, context, tag);
    }


    /**************************************************************************
     *
     * COLLECTIVE OPERATIONS
     *
     **************************************************************************/ 
    // TODO 
    // Make collective interfaces more slim
    template <typename T_Send, typename T_Recv>
    void gather(const CommID rootCommID, const Context context, const T_Send& sendData, T_Recv& recvData){
    	CommunicationPolicy::gather(sendData.data(), sendData.size(), recvData.data(), sendData.size(), rootCommID, context);
    }

    template <typename T_Send, typename T_Recv>
    void allGather(const Context context, const T_Send& sendData, const T_Recv& recvData){
    	CommunicationPolicy::allGather(sendData.data(), sendData.size(), recvData.data(), sendData.size(), context);
    }

    template <typename T_Send, typename T_Recv>
    void scatter(const CommID rootCommID, const Context context, const T_Send& sendData, const T_Recv& recvData){
    	CommunicationPolicy::gather(sendData.data(), sendData.size(), recvData.data(), recvData.size(), rootCommID, context);
    }

    template <typename T_Send, typename T_Recv>
    void allToAll(const Context context, const T_Send& sendData, const T_Recv& recvData){
    	CommunicationPolicy::allToAll(sendData.data(), sendData.size(), recvData.data(), recvData.size(), context);
    }

    // TODO 
    // Fill with various binary functions
    template <typename T>
    void reduce(const CommID rootCommID, const Context context, const BinaryOperation op, const T& sendData, const T& recvData){
     	CommunicationPolicy::reduce(sendData.data(), recvData.data(), sendData.size(), op, rootCommID, context);
    }

    template <typename T>
    void allReduce(const Context context, const BinaryOperation op, const T& sendData, T& recvData){
	CommunicationPolicy::allReduce(sendData.data(), recvData.data(), sendData.size(), op, context);
    }

    template <typename T>
    void broadcast(const NodeID rootCommID, const Context context, const T& data){
	CommunicationPolicy::broadcast(data.data(), data.size(), rootCommID, context);
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

	// Each announces how many nodes it manages
	std::array<unsigned, 1> myNodesCount {{(unsigned) nodes.size()}};
	std::array<unsigned, 1> maxNodesCount  {{0}};
	allReduce(context, BinaryOperations::MAX, myNodesCount, maxNodesCount);
	 
	
    	for(unsigned i = 0; i < maxNodesCount[0]; ++i){
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
		    contextMap[context.getContextID()][recvData[j]] = j;
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
	std::vector<CommID> uuids;
	for(Node node : nodes){
	    uuids.push_back(contextMap.at(oldContext.getContextID()).at(node.uuid));
	}

	Context newContext = CommunicationPolicy::createContext(uuids, oldContext);

	contextMap.insert(std::make_pair(newContext.getContextID(), std::map<NodeID, CommID>()));
	return newContext;
    }

    Context getInitialContext(){
	return CommunicationPolicy::initialContext;
    }


};

