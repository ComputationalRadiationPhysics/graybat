#pragma once

// GrayBat
#include <graybat.hpp>

// STL
#include <iostream>
#include <functional> /* std::functional */

namespace graybat {
    template <typename T_GraphPolicy, typename T_CommunicationPolicy>
    class Cave {
    public:

	// Typedefs
	typedef T_GraphPolicy         GraphPolicy;
	typedef T_CommunicationPolicy CommunicationPolicy;
      
	typedef graybat::CommunicationAbstractionLayer<CommunicationPolicy> CAL;
	typedef graybat::GraphBasedVirtualOverlayNetwork<GraphPolicy, CAL>  GVON;
      
	typedef unsigned                                  Vertex;
	typedef std::vector<Vertex>                       VertexContainer;
	typedef std::vector<std::pair<Vertex, Vertex> >   EdgeContainer;
	typedef std::pair<VertexContainer, EdgeContainer> GraphDescription;
	
	// Members
	GraphPolicy graph;
	CAL  cal;
	GVON gvon;


	// Constructor / Destructor
	Cave(std::function<GraphDescription()> graphFunctor) : gvon(cal){
	    std::cout << cal.getGlobalContext().getVAddr() << std::endl;
	    
	}

	~Cave(){

	}


	// Management Member Methods
	std::vector<Vertex> distribute(std::function<void()> distributionFunctor){
	    std::vector<Vertex> hostedVertices;

	    gvon.announce(graph, hostedVertices);

	}    
	
    };

}





