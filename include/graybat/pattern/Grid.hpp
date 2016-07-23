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

# pragma once

// STL
#include <utility> /* std::make_pair */

// GRAYBAT
#include <graybat/graphPolicy/Traits.hpp>

namespace graybat {

    namespace pattern {

        template<typename T_GraphPolicy>
        struct Grid {

            using GraphPolicy       = T_GraphPolicy;
            using VertexDescription = graybat::graphPolicy::VertexDescription<GraphPolicy>;
            using EdgeDescription   = graybat::graphPolicy::EdgeDescription<GraphPolicy>;
            using GraphDescription  = graybat::graphPolicy::GraphDescription<GraphPolicy>;
            using EdgeProperty      = graybat::graphPolicy::EdgeProperty<GraphPolicy>;
            using VertexProperty    = graybat::graphPolicy::VertexProperty<GraphPolicy>;  
          
            const unsigned height;
            const unsigned width;

            Grid(const unsigned height, const unsigned width) :
                height(height),
                width(width){

            }
    
            GraphDescription operator()(){

                const unsigned verticesCount = height * width;
                std::vector<VertexDescription> vertices(verticesCount);
                std::vector<EdgeDescription> edges;

                for(unsigned i = 0; i < vertices.size(); ++i){
                    vertices.at(i) = std::make_pair(i, VertexProperty());
                }
                
                for(unsigned i = 0; i < vertices.size(); ++i){
                    if(i >= width){
                        unsigned up   = i - width;
                        edges.push_back(std::make_pair(std::make_pair(vertices[i].first, vertices[up].first), EdgeProperty()));
                    }

                    if(i < (verticesCount - width)){
                        unsigned down = i + width;
                        edges.push_back(std::make_pair(std::make_pair(vertices[i].first, vertices[down].first), EdgeProperty()));
                    }


                    if((i % width) != (width - 1)){
                        int right = i + 1;
                        edges.push_back(std::make_pair(std::make_pair(vertices[i].first, vertices[right].first), EdgeProperty()));
                    }

                    if((i % width) != 0){
                        int left = i - 1;
                        edges.push_back(std::make_pair(std::make_pair(vertices[i].first, vertices[left].first), EdgeProperty()));
                    }
	

                }

                return std::make_pair(vertices,edges);
            }

        };

    } /* pattern */

} /* graybat */
