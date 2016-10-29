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
#include <boost/core/ignore_unused.hpp> /* boost::ignore_unused */
#include <algorithm> /* std::sort*/
#include <vector>    /* std::vector */
#include <memory>    /* std::shared_memory */

namespace graybat {
    
    namespace mapping {
    
	struct Filter {

            const size_t vertexTag;
            
            Filter(size_t vertexTag):
                vertexTag(vertexTag){

            }
        
	    template<typename T_Cage>
	    std::vector<typename T_Cage::Vertex> operator()(const unsigned processID, const unsigned processCount, T_Cage &cage){

                using CommunicationPolicy = typename T_Cage::CommunicationPolicy;
                using Vertex              = typename T_Cage::Vertex;
                using Context             = typename CommunicationPolicy::Context;
                using VAddr               = typename CommunicationPolicy::VAddr;

                boost::ignore_unused(processID);
                boost::ignore_unused(processCount);

                std::vector<VAddr> peersWithSameTag;
                
                std::shared_ptr<CommunicationPolicy> comm = cage.getCommunicationPolicy();
                Context context = comm->getGlobalContext();

                // Get the information about who wants to
                // host vertices with the same tag
                std::array<size_t, 1> sendData{vertexTag};                
                for(auto const &vAddr : context){
                    comm->asyncSend(vAddr, 0, context, sendData);
                }

                for(auto const &vAddr : context){                    
                    std::array<size_t, 1> recvData{0};
                    comm->recv(vAddr, 0, context, recvData);
                    if(recvData[0] == vertexTag){
                        peersWithSameTag.push_back(vAddr);
                    }
                }

                // Distribute vertices to peers with same tag
                std::sort(peersWithSameTag.begin(), peersWithSameTag.end());

                const size_t nPeers = peersWithSameTag.size();
                size_t peer_i = 0;
                
                std::vector<Vertex> vertices = cage.getVertices();
                std::vector<Vertex> myVertices;
                
                for(size_t i = 0; i < vertices.size(); ++i){
                    if(vertices[i]().tag == vertexTag){
                        if(peersWithSameTag.at(peer_i) == context.getVAddr()){
                            myVertices.push_back(vertices[i]);
                            
                        }
                        peer_i = (peer_i + 1 % nPeers);
                        
                    }

                }
                
 		return myVertices;

	    }

	};

    } /* mapping */
    
} /* graybat */
