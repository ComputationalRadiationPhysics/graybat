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

// STL
#include <numeric>

// GRAYBAT
#include <graybat/communicationPolicy/Traits.hpp>
#include <graybat/communicationPolicy/zmq/VAddrIterator.hpp>

namespace graybat {
    
    namespace communicationPolicy {
    
        namespace zmq {

            /**
             * @brief A context represents a set of peers which are
             *        able to communicate with each other.
             *
             */
            template<typename T_CP>
            class Context {

                using ContextID = typename graybat::communicationPolicy::ContextID<T_CP>;
                using VAddr     = typename graybat::communicationPolicy::VAddr<T_CP>;
                using Tag       = typename graybat::communicationPolicy::Tag<T_CP>;                
                using MsgType   = typename graybat::communicationPolicy::MsgType<T_CP>;
                using MsgID     = typename graybat::communicationPolicy::MsgID<T_CP>;
	    
            public:
                Context() :
                    contextID(0),
                    vAddr(0),
                    nPeers(1),
                    isValid(false),
                    peers(0){

                }

                Context(ContextID contextID, VAddr vAddr, unsigned nPeers) :
                    contextID(contextID),
                    vAddr(vAddr),
                    nPeers(nPeers),
                    isValid(true),
                    peers(0){


                    peers.resize(nPeers);
                    std::iota(peers.begin(), peers.end(), 0);
                }

                Context(ContextID contextID, VAddr vAddr, std::vector<VAddr> peers) :
                    contextID(contextID),
                    vAddr(vAddr),
                    nPeers(peers.size()),
                    isValid(true),
                    peers(peers) {
                }

                size_t size() const{
                    return nPeers;
                }

                VAddr getVAddr() const {
                    return vAddr;
                }

                ContextID getID() const {
                    return contextID;
                }

                bool valid() const{
                    return isValid;
                }

                std::vector<VAddr>::iterator begin(){
                    return peers.begin();
                }

                std::vector<VAddr>::const_iterator begin() const {
                    return peers.cbegin();
                }

                std::vector<VAddr>::iterator end(){
                    return peers.end();
                }

                std::vector<VAddr>::const_iterator end() const {
                    return peers.cend();
                }
                
            private:	
                ContextID contextID;
                VAddr     vAddr;
                unsigned  nPeers;
                bool      isValid;
                std::vector<VAddr> peers;
            };


        } // zmq
        
    } // namespace communicationPolicy
	
} // namespace graybat
