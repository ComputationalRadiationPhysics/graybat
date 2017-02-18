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

#include <graybat/graphPolicy/Traits.hpp>

namespace graybat {

namespace pattern {

template <typename T_GraphPolicy> struct None {
    using VertexDescription = graybat::graphPolicy::VertexDescription<T_GraphPolicy>;
    using EdgeDescription = graybat::graphPolicy::EdgeDescription<T_GraphPolicy>;
    using GraphDescription = graybat::graphPolicy::GraphDescription<T_GraphPolicy>;

    GraphDescription operator()()
    {
        std::vector<VertexDescription> vertices;
        std::vector<EdgeDescription> edges;

        return std::make_pair(vertices, edges);
    }
};

} /* pattern */

} /* graybat */
