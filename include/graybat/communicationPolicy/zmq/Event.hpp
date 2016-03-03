#pragma once

// STL
#include <memory> /* std::unique_ptr */

// graybat
#include <graybat/communicationPolicy/Traits.hpp>

namespace graybat {

    namespace communicationPolicy {

        namespace zmq {

	    /**
	     * @brief An event is returned by non-blocking
	     *        communication operations and can be
	     *        asked whether an operation has finished
	     *        or it can be waited for this operation to
	     *        be finished.
	     *
	     */
            template <typename T_CP>
	    class Event {
	    public:

                using ContextID = typename graybat::communicationPolicy::ContextID<T_CP>;
                using VAddr     = typename graybat::communicationPolicy::VAddr<T_CP>;
                using Tag       = typename graybat::communicationPolicy::Tag<T_CP>;
                using MsgType   = typename graybat::communicationPolicy::MsgType<T_CP>;
                using MsgID     = typename graybat::communicationPolicy::MsgID<T_CP>;
                using Context   = typename graybat::communicationPolicy::Context<T_CP>;

		Event(MsgID msgID, Context context, VAddr vAddr, Tag tag, T_CP& comm) :
                    msgID(msgID),
                    context(context),
                    vAddr(vAddr),
                    tag(tag),
                    buf(nullptr),
                    size(0),
                    done(false),
                    comm(comm) {

		}

            template<typename T_Buf>
		Event(MsgID msgID, Context context, VAddr vAddr, Tag tag, T_Buf & buf, bool done, T_CP& comm) :
                    msgID(msgID),
                    context(context),
                    vAddr(vAddr),
                    tag(tag),
                    buf(reinterpret_cast<std::int8_t*>(buf.data())),
                    size(sizeof(typename T_Buf::value_type) * buf.size()),
                    done(done),
                    comm(comm) {

		}

		void wait(){
                    while(!ready());

		}

                bool ready(){
                    //std::cout << "ready? size: " << size << std::endl;
                    if(done == true){
                        return true;
                    }
                    else {
                        if(buf == nullptr){
                            // asyncSend Event
                            done = comm.ready(msgID, context, vAddr, tag);
                        }
                        else {
                            // asyncRecv Event
                            done = comm.asyncRecvImpl(MsgType::PEER, context, vAddr, tag, buf, size);
                        }
                    }
                    return done;
                }

                VAddr source(){
		    return vAddr;
		}

                Tag getTag(){
                    return tag;

                }

                MsgID      msgID;
                Context    context;
                VAddr      vAddr;
                Tag        tag;
                std::int8_t * buf;
                const size_t size;
                bool       done;
                T_CP&      comm;




	    };

        } // zmq
        
    } // namespace communicationPolicy
	
} // namespace graybat
