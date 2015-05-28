#pragma once

template <class T_Cage>
struct CommunicationVertex {

    typedef unsigned                             VertexID; 
    typedef T_Cage                               Cage;
    typedef typename Cage::GraphPolicy           GraphPolicy;
    typedef typename Cage::Edge                  Edge;
    typedef typename Cage::Event                 Event;
    typedef typename GraphPolicy::VertexProperty VertexProperty;

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

    /***************************************************************************
     * Communication Operations
     ****************************************************************************/

    template <typename T>
    std::vector<Event> broadcast(const T& data){
	return cage.broadcast(*this, data);
	
    }

	
};
