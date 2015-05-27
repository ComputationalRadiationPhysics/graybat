#pragma once

template <class T_Cage>
struct CommunicationEdge {

    typedef T_Cage Cage;
    typedef typename Cage::GraphPolicy GraphPolicy;
    typedef typename Cage::Vertex Vertex;
    typedef typename GraphPolicy::EdgeProperty EdgeProperty;
    typedef typename GraphPolicy::VertexProperty VertexProperty;
    typedef typename Cage::Event Event;

	    
    unsigned id;
    Vertex target;
    Vertex source;
    EdgeProperty &edgeProperty;
    Cage &cage;

    CommunicationEdge(const unsigned id,
	     Vertex source,
	     Vertex target,
	     EdgeProperty &edgeProperty,
	     Cage &cage) :
	id(id),
	target(target),
	source(source),	
	edgeProperty(edgeProperty),
	cage(cage){
	    
    }

    EdgeProperty& operator()(){
	return edgeProperty;
    }

    template <class T_Send>
    Event operator<<(const T_Send &data){
	return cage.asyncSend(*this, data);
    }

    template <class T_Recv>
    void operator>>(T_Recv &data){
	cage.recv(*this, data);
    }
	
};
