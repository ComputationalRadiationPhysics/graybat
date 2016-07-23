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

// CLIB
#include <time.h>

// STL
#include <tuple>

// GRAYBAT
#include <graybat/Cage.hpp>
#include <graybat/communicationPolicy/BMPI.hpp>
#include <graybat/graphPolicy/BGL.hpp>

#include <graybat/mapping/Roundrobin.hpp>
#include <graybat/pattern/OutStar.hpp>

/**
 * @brief Subscriber receive messages from publisher
 *
 * This example implements the publish and subscribe example of ZMQ[1]
 * in graybat style.
 *
 * [1] http://zguide.zeromq.org/page:all#Getting-the-Message-Out 
 *
 */

#define within(num) (int) ((float) num * random () / (RAND_MAX + 1.0))

int main() {
    /***************************************************************************
     * Configuration
     ****************************************************************************/

    // CommunicationPolicy
    typedef graybat::communicationPolicy::BMPI CP;
    
    // GraphPolicy
    typedef graybat::graphPolicy::BGL<>        GP;
    
    // Cage
    typedef graybat::Cage<CP, GP> Cage;
    typedef typename Cage::Vertex Vertex;
    typedef typename Cage::Edge   Edge;

    /***************************************************************************
     * Initialize Communication
     ****************************************************************************/
    // Create GoL Graph
    Cage cage;

    // Set communication pattern
    cage.setGraph(graybat::pattern::OutStar(cage.getPeers().size()));
    
    // Map vertices to peers
    cage.distribute(graybat::mapping::Roundrobin());

    /***************************************************************************
     * Run 
     ****************************************************************************/
    Vertex publisher = cage.getVertex(0);

    srandom ((unsigned) time (NULL));

    std::array<std::tuple<int,int,int>, 1> msg;

    for(Vertex v : cage.hostedVertices){


        // Publisher
        if(v == publisher){
            while(true){

                int zipcode, temperature, relhumidity;
                
                zipcode     = within (100000);
                temperature = within (215) - 80;
                relhumidity = within (50) + 10;

                msg[0] = std::make_tuple(zipcode, temperature, relhumidity);

                // Send weather data to all subscribers
                v.spread(msg);
                
            }
            
        }
        // Subscriber
        else {
            long total_temp = 0;
            for(unsigned nUpdates = 0; nUpdates < 100; ++nUpdates){

                int zipcode, temperature, relhumidity;
                
                cage.recv(msg);

                zipcode = std::get<0>(msg[0]);
                temperature = std::get<1>(msg[0]);
                temperature = std::get<2>(msg[0]);
                
                total_temp += temperature;
            }

            // Calculate average temperature value
            std::cout << (int) (total_temp / 100) << "F" << std::endl;
                
        }
        
    }
    

    
    
    return 0;
}
