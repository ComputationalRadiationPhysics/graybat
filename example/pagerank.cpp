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

/**
 * @example chain.cpp
 *
 * @brief Data is send through a chain of compute 
 *        nodes and every node increments the value.
 *
 */

// STL
#include <iostream>   /* std::cout */
#include <vector>     /* std::vector */

// GRAYBAT
#include <graybat/Cage.hpp>
#include <graybat/communicationPolicy/BMPI.hpp>
#include <graybat/graphPolicy/BGL.hpp>
#include <graybat/serializationPolicy/ByteCast.hpp>

// GRAYBAT mappings
#include <graybat/graybat.hpp>

struct PageRank {
    PageRank() : pageRank({1}){ }
    std::vector<float> pageRank;
};

int exp() {
    /***************************************************************************
     * Configuration
     ****************************************************************************/

    // CommunicationPolicy
    typedef graybat::communicationPolicy::BMPI  CP;
    typedef CP::Config                          Config;    
    
    // GraphPolicy
    typedef graybat::graphPolicy::BGL<PageRank> GP;

    // SerializationPolicy
    using SP = graybat::serializationPolicy::ByteCast;

    // Cage
    typedef graybat::Cage<CP, GP, SP> Cage;
    typedef typename Cage::Event  Event;
    typedef typename Cage::Vertex Vertex;
    typedef typename Cage::Edge   Edge;    

    /***************************************************************************
     * Initialize Communication
     ****************************************************************************/
    size_t const n = 1000;    
    size_t const min = 1;
    size_t const max = 10;
    size_t const seed = 1234;
    Config config;
    Cage cage(config);

    cage.setGraph(graybat::pattern::Random<GP>(n, min, max, seed));    
    
    // Distribute vertices
    cage.distribute(graybat::mapping::Consecutive());

    /***************************************************************************
     * Run Simulation
     ****************************************************************************/
    std::vector<Event> events;
    size_t const nIterations = 20;
    float const dampingFactor = 0.5;

    for(size_t iteration = 0; iteration < nIterations; iteration++){

        // Calculate relative pagerank and spread it to
        // adjacent vertices
        for(Vertex &v : cage.getHostedVertices()){
            assert(cage.getAdjacentVertices(v).size() > 0);            
            std::vector<float> relativePageRank{1};
            relativePageRank[0] = v().pageRank[0] / static_cast<float>(cage.getAdjacentVertices(v).size());
            v.spread(relativePageRank, events);
        }

        // Calculate vertex page rank based on
        // relative page rank of adjacent vertices
        for(Vertex &v : cage.getHostedVertices()){
            std::vector<float> relativePageRank{1};            
            float relativePageRankSum = 0;
            
            for(Edge e : cage.getInEdges(v)){
                cage.recv(e, relativePageRank);
                relativePageRankSum += relativePageRank[0];                
            }
            v().pageRank[0] = (1 - dampingFactor) + dampingFactor * relativePageRankSum;
        }

        for(unsigned i = 0; i < events.size(); ++i){
            events.back().wait();
            events.pop_back();
        }
    }

    // Print page final page rank
    for(Vertex &v : cage.getHostedVertices()){
        std::cout << "Pagerank[" <<v.id << "] : " << v().pageRank[0] << std::endl;
    }
    
    return 0;

}

int main(){
    exp();
    return 0;
}
