#pragma once

// BOOST
#include <boost/mpi/environment.hpp>

// GRAYBAT
#include <graybat/communicationPolicy/Traits.hpp>
#include <graybat/communicationPolicy/bmpi/VAddrIterator.hpp>

namespace graybat {
    
    namespace communicationPolicy {

        namespace bmpi {

            
            /**
	     * @Brief A context represents a set of peers which are
	     *        able to communicate with each other.
	     *
	     */
            template<typename T_CP>
	    class Context {
                using ContextID = typename graybat::communicationPolicy::ContextID<T_CP>;
                using VAddr     = typename graybat::communicationPolicy::VAddr<T_CP>;
	    
	    public:
		Context() :
		    id(0),
		    isValid(false){

		}

		Context(ContextID contextID, boost::mpi::communicator comm) : 
		    comm(comm),
		    id(contextID),
		    isValid(true){
		
		}

		Context& operator=(const Context& otherContext){
		    id            = otherContext.getID();
		    isValid       = otherContext.valid();
		    comm          = otherContext.comm;
		    return *this;

		}

		size_t size() const{
		    return comm.size();
		}

		VAddr getVAddr() const {
		    return comm.rank();
		}

		ContextID getID() const {
		    return id;
		}

		bool valid() const{
		    return isValid;
		}

                VAddrIterator<T_CP> begin(){
                    return VAddrIterator<T_CP>(0);
                }

                VAddrIterator<T_CP> begin() const {
                    return VAddrIterator<T_CP>(0);
                }
                
                VAddrIterator<T_CP> end(){
                    return VAddrIterator<T_CP>(size());
                }

                VAddrIterator<T_CP> end() const {
                    return VAddrIterator<T_CP>(size());
                }
                
		boost::mpi::communicator comm;
	
	    private:	
		ContextID id;
		bool      isValid;
	    };

        } // namespace bmpi
        
    } // namespace communicationPolicy
	
} // namespace graybat
