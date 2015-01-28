#pragma once

#include <iostream>

namespace graybat {
    template <typename T_GraphPolicy, typename T_CommunicationPolicy>
    class Cave {
    public:
	
	typedef T_GraphPolicy         GraphPolicy;
	typedef T_CommunicationPolicy CommunicationPolicy;
	

	Cave(){
	    std::cout << "Welcome in the cave" << std::endl;
	}

    };

}





