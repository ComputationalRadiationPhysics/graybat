#pragma once

template <class T_Cage>
struct VertexTest {
    
    typedef T_Cage Cage;
    typedef typename Cage::GraphPolicy GraphPolicy;
    typedef typename GraphPolicy::VertexProperty VertexProperty;

    unsigned id;
    VertexProperty &vertexProperty;


    VertexTest(const unsigned id, VertexProperty &vertexProperty) :
	id(id),
	vertexProperty(vertexProperty){
	    
    }

    VertexProperty& operator()(){
	return vertexProperty;
    }
	
};
