#pragma once

namespace graybat {
    
    namespace communicationPolicy {

        namespace traits {

            template <typename T_CommunicationPolicy>
            struct ContextType;

            template <typename T_CommunicationPolicy>
            struct EventType;

            template <typename T_CommunicationPolicy>
            struct ConfigType;
            
        } // namespace traits

        template <typename T_CommunicationPolicy>        
        using VAddr = unsigned;

        template <typename T_CommunicationPolicy>
        using Tag = unsigned;

        template <typename T_CommunicationPolicy>        
        using ContextID = unsigned;
        
        enum class MsgTypeType : std::int8_t { VADDR_REQUEST = 0,
                VADDR_LOOKUP = 1,
                DESTRUCT = 2,
                RETRY = 3,
                ACK = 4,
                CONTEXT_INIT = 5,
                CONTEXT_REQUEST = 6,
                PEER = 7,
                CONFIRM = 8,
                SPLIT = 9};
        
        template <typename T_CommunicationPolicy>
        using MsgType = MsgTypeType;

        template <typename T_CommunicationPolicy>        
        using MsgID = unsigned;

        template <typename T_CommunicationPolicy>
        using Context = typename traits::ContextType<T_CommunicationPolicy>::type;

        template <typename T_CommunicationPolicy>
        using Event = typename traits::EventType<T_CommunicationPolicy>::type;

        template <typename T_CommunicationPolicy>
        using Config = typename traits::ConfigType<T_CommunicationPolicy>::type;
        
    } // namespace communicationPolicy
    
} // namespace graybat
