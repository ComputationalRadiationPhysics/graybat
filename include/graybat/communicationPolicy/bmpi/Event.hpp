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

#include <boost/mpi/environment.hpp>

namespace graybat {
    
    namespace communicationPolicy {

        namespace bmpi {


	    /**
	     * @brief An event is returned by non-blocking 
	     *        communication operations and can be 
	     *        asked whether an operation has finished
	     *        or it can be waited for this operation to
	     *        be finished.
	     *
	     */
	    class Event {
                typedef unsigned Tag;                                            
                typedef unsigned VAddr;
                
	    public:
		Event(boost::mpi::request request) : request(request), async(true){

		}

                Event(boost::mpi::status status) : status(status), async(false){

                }


		~Event(){

		}

		void wait(){
                    if(async){
                        request.wait();
                    }
	
		}

		bool ready(){
                    if(async){
                        boost::optional<boost::mpi::status> status = request.test();

                        if(status){
                            return true;
                        }
                        else {
                            return false;
                        }
                    }
                    return true;

		}

                VAddr source(){
                    if(async){
                        status = request.wait();
                    }
                    return status.source();
                }

                Tag getTag(){
                    if(async){
                        status = request.wait();
                    }
                    return status.tag();

                }

	    private:
		boost::mpi::request request;
                boost::mpi::status  status;
                const bool async;

                
                
	    };

        } // namespace bmpi
        
    } // namespace communicationPolicy
	
} // namespace graybat

