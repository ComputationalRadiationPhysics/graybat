#pragma once

template <class T_Cage>
struct EdgeTest {

    typedef T_Cage Cage;
    typedef typename Cage::GraphPolicy GraphPolicy;
    typedef typename Cage::Vertex Vertex;
    typedef typename GraphPolicy::EdgeProperty EdgeProperty;
    typedef typename Cage::Event Event;
	    
    unsigned id;
    EdgeProperty edgeProperty;
    Vertex srcVertex;
    Vertex destVertex;
    Cage &cage;
	    


    EdgeTest(const unsigned id, Vertex destVertex, EdgeProperty edgeProperty, Cage &cage) :
	id(id),
	edgeProperty(edgeProperty),
	cage(cage){
	    
    }

    EdgeProperty operator()(){
	return edgeProperty;
    }

    template <class T_Send>
    Event operator<<(const T_Send &data){
	return cage.asyncSend(destVertex, edgeProperty, data);
    }
	
};
