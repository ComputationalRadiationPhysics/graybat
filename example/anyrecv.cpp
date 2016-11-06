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
 * @example anyrecv.cpp
 *
 * @brief A master recv messages from all slaves and answers.
 *
 */

// STL
#include <iostream>   /* std::cout */
#include <vector>     /* std::vector */
#include <array>      /* std::array */
#include <functional> /* std::bind */

// GRAYBAT
#include <graybat/Cage.hpp>
#include <graybat/communicationPolicy/BMPI.hpp>
#include <graybat/graphPolicy/BGL.hpp>
#include <graybat/serializationPolicy/ByteCast.hpp>
// GRAYBAT mappings
#include <graybat/mapping/Consecutive.hpp>
#include <graybat/mapping/Random.hpp>
#include <graybat/mapping/Roundrobin.hpp>
// GRAYBAT pattern
#include <graybat/pattern/GridDiagonal.hpp>
#include <graybat/pattern/Chain.hpp>
#include <graybat/pattern/BiStar.hpp>

struct Function {
    
    void process(std::array<unsigned, 1> &data){
	foo(std::get<0>(data));
	
    }

    std::function<void (unsigned&)> foo;
};


int exp() {
    /***************************************************************************
     * Configuration
     ****************************************************************************/

    // CommunicationPolicy
    typedef graybat::communicationPolicy::BMPI CP;
    typedef CP::Config                         Config;    
    
    // GraphPolicy
    typedef graybat::graphPolicy::BGL<Function>    GP;

    // SerializationPolicy
    using SP = graybat::serializationPolicy::ByteCast;

    // Cage
    typedef graybat::Cage<CP, GP, SP> Cage;
    typedef typename Cage::Vertex Vertex;
    typedef typename Cage::Edge Edge;

    /***************************************************************************
     * Initialize Communication
     ****************************************************************************/
    // Create GoL Graph
    Config config;
    Cage cage(config);

    // Set communication pattern
    cage.setGraph(graybat::pattern::BiStar<GP>(cage.getPeers().size()));

    // Distribute vertices
    cage.distribute(graybat::mapping::Consecutive());
        

    /***************************************************************************
     * Run Simulation
     ****************************************************************************/
    //std::vector<Event> events;

    std::array<unsigned, 1> input {{0}};
    std::array<unsigned, 1> output {{0}};

    const Vertex reply = cage.getVertex(0);
    
    for(Vertex &v : cage.getHostedVertices()){


        if(v == reply){
            while(true){
                Edge e = cage.recv(output);
                std::cout << "Got msg from " << e.source.id << ": "<< output[0] << std::endl;
                output[0]++;
                cage.send(e.inverse(), output);
            }
            
        }
        else {
            input[0] = v.id;
            v.spread(input);
            v.collect(input);
            std::cout << " Got input from master:" << input[0] << std::endl;
        }
 
	
    }

    return 0;

}

int main(){
    exp();
    return 0;
}
