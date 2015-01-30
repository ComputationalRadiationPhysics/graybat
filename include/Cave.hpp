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
	
	typedef T_GraphPolicy         GraphPolicy;
	typedef T_CommunicationPolicy CommunicationPolicy;

	typedef graybat::CommunicationAbstractionLayer<CommunicationPolicy> CAL;
	typedef graybat::GraphBasedVirtualOverlayNetwork<GraphPolicy, CAL>  GVON;

	typedef std::pair<unsigned, unsigned> GraphDescription;
	

	// How to specify graph ?
	// * function pointer
	// * functor
	static void createGraph(std::function<GraphDescription()> foo){
	    std::cout << foo().first << " " << foo().second << std::endl;

	}

      template <typename FOO>
      static void createGraph0(FOO foo){
	std::cout << foo().first << " " << foo().second << std::endl;

      }
	
	
    };

}





