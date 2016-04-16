#pragma once

namespace graybat {
    
    namespace communicationPolicy {

        namespace socket {
        
            namespace traits {

                template <typename T_CommunicationPolicy>
                struct UriType;

                template <typename T_CommunicationPolicy>
                struct SocketType;

                template <typename T_CommunicationPolicy>
                struct MessageType;
            
            
            } // namespace traits

            template <typename T_CommunicationPolicy>
            using Uri = typename traits::UriType<T_CommunicationPolicy>::type;        

            template <typename T_CommunicationPolicy>
            using Socket = typename traits::SocketType<T_CommunicationPolicy>::type;        
        
            template <typename T_CommunicationPolicy>
            using Message = typename traits::MessageType<T_CommunicationPolicy>::type;

            template <typename T_CommunicationPolicy>
            using ContextName = std::string;
            
        } // namespace socket
            
    } // namespace communicationPolicy
    
} // namespace graybat
