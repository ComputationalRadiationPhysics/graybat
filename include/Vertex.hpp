#pragma once


// TODO
// Replace call by value with
// call by reference
template <class T_Cage>
struct VertexTest {
    
    typedef T_Cage Cage;
    typedef typename Cage::GraphPolicy GraphPolicy;
    typedef typename GraphPolicy::VertexProperty VertexProperty;

    unsigned id;
    VertexProperty vertexProperty;
    Cage &cage;


    VertexTest(const unsigned id, VertexProperty vertexProperty, Cage &cage) :
	id(id),
	vertexProperty(vertexProperty),
	cage(cage){
	    
    }

    VertexProperty operator()(){
	return vertexProperty;
    }
	
};
