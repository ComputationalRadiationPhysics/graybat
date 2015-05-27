#pragma once

template <class T_Cage>
struct EdgeTest {

    typedef T_Cage Cage;
    typedef typename Cage::GraphPolicy GraphPolicy;
    typedef typename Cage::Vertex Vertex;
    typedef typename GraphPolicy::EdgeProperty EdgeProperty;
    typedef typename GraphPolicy::VertexProperty VertexProperty;
    typedef typename Cage::Event Event;

	    
    unsigned id;
    Vertex edgeTarget;
    Vertex edgeSource;
    EdgeProperty &edgeProperty;
    Cage &cage;

    EdgeTest(const unsigned id,
	     Vertex edgeSource,
	     Vertex edgeTarget,
	     EdgeProperty &edgeProperty,
	     Cage &cage) :
	id(id),
	edgeTarget(edgeTarget),
	edgeSource(edgeSource),	
	edgeProperty(edgeProperty),
	cage(cage){
	    
    }

    EdgeProperty& operator()(){
	return edgeProperty;
    }

    template <class T_Send>
    Event operator<<(const T_Send &data){
	return cage.asyncSend(edgeTarget, *this, data);
    }

    VertexProperty& target(){
	return edgeTarget();
    }

    VertexProperty& source(){
	return edgeSource();
    }

    template <class T_Recv>
    void operator>>(T_Recv &data){
	cage.recv(edgeSource, *this, data);
    }
	
};
