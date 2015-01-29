#pragma once

#include <iostream>
#include <graybat.hpp>

namespace graybat {
    template <typename T_GraphPolicy, typename T_CommunicationPolicy>
    class Cave {
    public:
	
	typedef T_GraphPolicy         GraphPolicy;
	typedef T_CommunicationPolicy CommunicationPolicy;

	typedef graybat::CommunicationAbstractionLayer<CommunicationPolicy> CAL;
	typedef graybat::GraphBasedVirtualOverlayNetwork<GraphPolicy, CAL>  Graph;
	

	static Graph createGraph(){
	    CAL cal;
	    Graph graph(cal);
	    return graph;
	}
	
	
    };

}





