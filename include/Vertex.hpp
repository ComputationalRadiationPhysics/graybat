#pragma once


template <class T_Cage>
struct CommunicationVertex {
    
    typedef T_Cage Cage;
    typedef typename Cage::GraphPolicy GraphPolicy;
    typedef typename GraphPolicy::VertexProperty VertexProperty;
    typedef typename GraphPolicy::VertexID VertexID;

    VertexID id;
    VertexProperty &vertexProperty;
    Cage &cage;


    CommunicationVertex(const VertexID id, VertexProperty &vertexProperty, Cage &cage) :
	id(id),
	vertexProperty(vertexProperty),
	cage(cage){
	    
    }

    VertexProperty& operator()(){
	return vertexProperty;
    }

    CommunicationVertex& operator=(const CommunicationVertex &other){
	id = other.id;
	vertexProperty = other.vertexProperty;

	return *this;
    }
	
};
