/**
 * Copyright 2016 Erik Zenker
 *
 * This file is part of Graybat.
 *
 * Graybat is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Graybat is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with Graybat.
 * If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <graybat/communicationPolicy/Traits.hpp>

namespace graybat {
    
    namespace communicationPolicy {
    
        namespace zmq {

            template <typename T_CommunicationPolicy>
            struct Message {

                // Types
                using CommunicationPolicy = T_CommunicationPolicy;
                using ContextID           = typename graybat::communicationPolicy::ContextID<CommunicationPolicy>;
                using VAddr               = typename graybat::communicationPolicy::VAddr<CommunicationPolicy>;
                using Tag                 = typename graybat::communicationPolicy::Tag<CommunicationPolicy>;                
                using MsgType             = typename graybat::communicationPolicy::MsgType<CommunicationPolicy>;
                using MsgID               = typename graybat::communicationPolicy::MsgID<CommunicationPolicy>;

                // Members
                ::zmq::message_t message;

                // Methods
                Message(){

                }
                
                template <typename T_Data>
                Message(MsgType const msgType,  
                        MsgID const msgID,
                        ContextID const contextID,
                        VAddr const srcVAddr,
                        Tag const tag,      
                        T_Data & data) : message(sizeof(MsgType) +
                                                       sizeof(MsgID) +
                                                       sizeof(ContextID) +
                                                       sizeof(VAddr) +
                                                       sizeof(Tag) +
                                                       data.size() * sizeof(typename T_Data::value_type)){

                    size_t    msgOffset(0);
                    memcpy (static_cast<char*>(message.data()) + msgOffset, &msgType,    sizeof(MsgType));   msgOffset += sizeof(MsgType);
                    memcpy (static_cast<char*>(message.data()) + msgOffset, &msgID,      sizeof(MsgID));     msgOffset += sizeof(MsgID);		
                    memcpy (static_cast<char*>(message.data()) + msgOffset, &contextID,  sizeof(ContextID)); msgOffset += sizeof(ContextID);
                    memcpy (static_cast<char*>(message.data()) + msgOffset, &srcVAddr,   sizeof(VAddr));     msgOffset += sizeof(VAddr);
                    memcpy (static_cast<char*>(message.data()) + msgOffset, &tag,        sizeof(Tag));       msgOffset += sizeof(Tag);
                    memcpy (static_cast<char*>(message.data()) + msgOffset, data.data(), sizeof(typename T_Data::value_type) * data.size());

                }

                MsgType getMsgType(){
                    MsgType msgType;
                    memcpy (&msgType, static_cast<char*>(message.data()), sizeof(MsgType));
                    return msgType;

                    
                }

                MsgID getMsgID(){
                    MsgID   msgID;
                    memcpy (&msgID, static_cast<char*>(message.data()) + sizeof(MsgType), sizeof(MsgID));
                    return msgID;
                    
                }

                ContextID getContextID(){
                    ContextID contextID;
                    memcpy (&contextID, static_cast<char*>(message.data()) + sizeof(MsgType) + sizeof(MsgID), sizeof(ContextID));
                    return contextID;
                    
                }

                VAddr getVAddr(){
                    VAddr vAddr;
                    memcpy (&vAddr, static_cast<char*>(message.data()) + sizeof(MsgType) + sizeof(MsgID) + sizeof(ContextID), sizeof(VAddr));
                    return vAddr;

                }

                Tag getTag(){
                    Tag tag;
                    memcpy (&tag, static_cast<char*>(message.data()) + sizeof(MsgType) + sizeof(MsgID) + sizeof(ContextID) + sizeof(VAddr), sizeof(Tag));
                    return tag;

                }
		
		size_t size() {
		    return message.size();
		}

                std::int8_t* getData(){
                    return static_cast<std::int8_t*>(message.data()) + sizeof(MsgType) + sizeof(MsgID) + sizeof(ContextID) + sizeof(VAddr) + sizeof(Tag);
                    
                }
                
                ::zmq::message_t& getMessage(){
                    return message;
                }
                
            };

            
        } // zmq
        
    } // namespace communicationPolicy
	
} // namespace graybat
