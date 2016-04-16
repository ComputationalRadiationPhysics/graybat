#pragma once

namespace graybat {
    
    namespace communicationPolicy {
    
        namespace zmq {

            struct Config {

                std::string masterUri;
                std::string peerUri;
                size_t contextSize;
                std::string contextName = "context";
                size_t maxBufferSize = 100 * 1000 * 1000;
            };

        } // zmq
        
    } // namespace communicationPolicy
	
} // namespace graybat
