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
#include <utility> /* std::pair */
#include <vector>

namespace graybat {

    namespace graphPolicy {

        namespace traits {


        } // namespace traits

        /***********************************************************************
         * Iterators
         **********************************************************************/
        template <typename T_GraphPolicy>
        using InEdgeIter = typename T_GraphPolicy::InEdgeIter;
        
        template <typename T_GraphPolicy>
        using OutEdgeIter = typename T_GraphPolicy::OutEdgeIter;

        template <typename T_GraphPolicy>        
        using AdjacentVertexIter = typename T_GraphPolicy::AdjacentVertexIter;

        template <typename T_GraphPolicy>        
        using AllVertexIter = typename T_GraphPolicy::AllVertexIter;

        /***********************************************************************
         * Properties
         **********************************************************************/
        template <typename T_GraphPolicy>
        using VertexProperty = typename T_GraphPolicy::VertexProperty;

        template <typename T_GraphPolicy>
        using EdgeProperty = typename T_GraphPolicy::EdgeProperty;

        /***********************************************************************
         * Constants Types
         **********************************************************************/
        using VertexID = size_t;
        using EdgeID   = size_t;        
        using GraphID  = size_t;
        
        template <typename T_GraphPolicy>        
        using VertexDescription = std::pair<VertexID, VertexProperty<T_GraphPolicy> >;

        template <typename T_GraphPolicy>        
        using EdgeDescription = std::pair< std::pair<
                                               VertexID
                                               ,VertexID >
                                           ,EdgeProperty<T_GraphPolicy> >;

        template <typename T_GraphPolicy>
        using GraphDescription = std::pair<
            std::vector<VertexDescription<T_GraphPolicy> >,
            std::vector<EdgeDescription<T_GraphPolicy> >
            >;

        
    } // namespace graphPolicy
    
} // namespace graybat
