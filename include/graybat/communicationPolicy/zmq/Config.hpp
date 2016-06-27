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

namespace graybat {
    
    namespace communicationPolicy {
    
        namespace zmq {

            struct Config {

                std::string masterUri;
                std::string peerUri;
                size_t contextSize;
                std::string contextName = "context";
                size_t maxBufferSize = 100 * 1000 * 1000;
            };

        } // zmq
        
    } // namespace communicationPolicy
	
} // namespace graybat
