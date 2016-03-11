#pragma once

// CLIB
#include <assert.h>

// STL
#include <utility> /* std::make_pair */
#include <cstdlib> /* std::rand */

// GRAYBAT
#include <graybat/graphPolicy/Traits.hpp>

namespace graybat {

    namespace pattern {

        template<typename T_GraphPolicy>
        struct Random {

            using GraphPolicy       = T_GraphPolicy;
            using VertexDescription = graybat::graphPolicy::VertexDescription<GraphPolicy>;
            using EdgeDescription   = graybat::graphPolicy::EdgeDescription<GraphPolicy>;
            using GraphDescription  = graybat::graphPolicy::GraphDescription<GraphPolicy>;
            using EdgeProperty      = graybat::graphPolicy::EdgeProperty<GraphPolicy>;
            using VertexProperty    = graybat::graphPolicy::VertexProperty<GraphPolicy>;            

            size_t const nVertices;
            size_t const minEdgesPerVertex;
            size_t const maxEdgesPerVertex;
            
            Random(size_t const nVertices, size_t const minEdgesPerVertex, size_t const maxEdgesPerVertex, size_t const seed) :
                nVertices(nVertices),
                minEdgesPerVertex(minEdgesPerVertex),
                maxEdgesPerVertex(maxEdgesPerVertex)
            {
                assert(minEdgesPerVertex <= maxEdgesPerVertex);
                std::srand(seed);
            }
            
            GraphDescription operator()(){
                std::vector<VertexDescription> vertices;
                std::vector<EdgeDescription> edges;
                
                for(size_t vertex_i = 0; vertex_i < nVertices; ++vertex_i){
                    vertices.push_back(std::make_pair(vertex_i, VertexProperty()));
                }

                for(size_t vertex_i = 0; vertex_i < nVertices; ++vertex_i){
                    size_t nEdges = std::max(minEdgesPerVertex, (std::rand() % maxEdgesPerVertex));
                    for(size_t j = 0; j < nEdges; ++j){
                        size_t adjacentVertex_i = std::rand() % nVertices;
                        edges.push_back(std::make_pair(std::make_pair(vertices[vertex_i].first, vertices[adjacentVertex_i].first), EdgeProperty()));
                    }
                }
                
                return std::make_pair(vertices, edges);
            }

        };

    } /* pattern */

} /* graybat */
