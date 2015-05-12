#pragma once


// TODO
// Replace call by value with
// call by reference
template <class T_Cage>
struct VertexTest {
    
    typedef T_Cage Cage;
    typedef typename Cage::GraphPolicy GraphPolicy;
    typedef typename GraphPolicy::VertexProperty VertexProperty;
    typedef typename GraphPolicy::VertexID VertexID;

    VertexID id;
    VertexProperty &vertexProperty;
    Cage &cage;


    VertexTest(const VertexID id, VertexProperty &vertexProperty, Cage &cage) :
	id(id),
	vertexProperty(vertexProperty),
	cage(cage){
	    
    }

    VertexProperty operator()(){
	return vertexProperty;
    }

    // VertexTest(const VertexTest &other) :
    // 	id(other.id),
    // 	vertexProperty(other.vertexProperty),
    // 	cage(other.cage){
    // }

    VertexTest& operator=(const VertexTest &other){
	id = other.id;
	vertexProperty = other.vertexProperty;
	
	return *this;
    }
	
};
