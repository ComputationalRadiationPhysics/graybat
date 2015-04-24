Communication Policy
====================

[cage]:utils/doxygen/cage.md
[context]:utils/doxygen/context.md
[communication policy]:utils/doxygen/communicationPolicy.md

The communication policy is a class which implements the
communication interface of its host class ([cage]).

Communication in GrayBat is modeled in the way, that an instance that
takes part on whatever communication is called a *peer*. All peers
that want to communicate in some way with each other need to group up
in a [context]. Therefore, a [context] is a set of peers that are able
to communicate with each other.

By communication is meant the exchange of arbitrary data between peers
or even within one peer. Thus, communication can mean sending a
message over the internet, copying data between two memories, or
distributing data with the help of MPI. Therefore, a
communication policy need to implement the required interface but
can interpret the term communication on its own. See the code example
below or consider the preimplemented communication policies.

The communication policy interface is separated into the parts
**point-to-point communication**, **collective communication**, and
**context management**. The advice for completeness is to implement
all methods of these three parts and run the provided unit test cases
to verify their correctness. Nevertheless, not all communication
methods might be necessary for a special use case and only a handful
is really necessary to initialize the [cage]. Thus, it is possible to
provide a communication policy implementation that does not implement
all interface methods.

The basic set of methods that **need** to be implemented to initialize
the [cage] are the following:

* Context Management
 * getGlobalContext
 * createContext
* Collective Communication
 * allGather
 * allReduce
* Type definitions
 * Context 
 * Event
 
The following source code provides a basic skeleton of a communication
policy:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cc}
namespace graybat {

	namespace communicationPolicy {

		struct CommunicationPolicyI {
			typedef unsigned    VAddr;
		
			// Necessary type definitions
			typedef ContextImpl Context;
			typedef EventImpl   Event;

			CommunicationPolicyI(){
				...
			}
				
			/*********************************************************************************	
			 * Basic communication policy methods
			 * needed by the cage initialization
			 *********************************************************************************/
			template <typename T_Send, typename T_Recv>
			void allGather(const Context context, const T_Send& sendData, T_Recv& recvData){...}
				
			template <typename T_Send, typename T_Recv, typename T_Op>	
			void allReduce(const Context context, T_Op op, const T_Send& sendData, T_Recv& recvData){...}
				
			Context createContext(const std::vector<VAddr> peers, const Context oldContext){...}
				
			Context getGlobalContext(){...}
				
	    }
				
	} /* namespace communicationPolicy*/
				
} /* namespace graybat */
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The following source code provides the full skeleton of a
communication policy:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cc}
namespace graybat {

	namespace communicationPolicy {

	    struct CommunicationPolicyI {
			typedef unsigned    Tag;
			typedef unsigned    VAddr;
		
			// Necessary typedefinitions
			typedef ContextImpl Context;
			typedef EventImpl   Event;

			CommunicationPolicyI(){
				...
			}
				
			/*********************************************************************************	
		     * POINT TO POINT COMMUNICATION OPERATIONS
			 *********************************************************************************/
		    template <typename T_Send>
			Event asyncSend(const VAddr destVAddr, const Tag tag, const Context context, const T_Send& sendData){...}
			
			template <typename T_Send>
			void send(const VAddr destVAddr, const Tag tag, const Context context, const T_Send& sendData){...}
			
			template <typename T_Recv>
			Event asyncRecv(const VAddr srcVAddr, const Tag tag, const Context context, T_Recv& recvData){...}
			
			template <typename T_Recv>
			void recv(const VAddr srcVAddr, const Tag tag, const Context context, T_Recv& recvData){...}

		    /*********************************************************************************	
		     * COLLECTIVE COMMUNICATION OPERATIONS
			 *********************************************************************************/
		    template <typename T_Send, typename T_Recv>
			void gather(const VAddr rootVAddr, const Context context, const T_Send& sendData, T_Recv& recvData){...}
			
		    template <typename T_Send, typename T_Recv>
			void allGather(const Context context, const T_Send& sendData, T_Recv& recvData){...}
			
			template <typename T_Send, typename T_Recv>
			void gatherVar(const VAddr rootVAddr, const Context context, const T_Send& sendData, T_Recv& recvData, std::vector<unsigned>& recvCount){...}
			
			template <typename T_Send, typename T_Recv>
			void allGatherVar(const Context context, const T_Send& sendData, T_Recv& recvData, std::vector<unsigned>& recvCount){...}
			
			template <typename T_Send, typename T_Recv>
			void scatter(const VAddr rootVAddr, const Context context, const T_Send& sendData, T_Recv& recvData){...}
			
			template <typename T_Send, typename T_Recv>
			void allToAll(const Context context, const T_Send& sendData, T_Recv& recvData){...}
			
			template <typename T_Send, typename T_Recv, typename T_Op>
			void reduce(const VAddr rootVAddr, const Context context, const T_Op op, const T_Send& sendData, const T_Recv& recvData){...}
			
			template <typename T_Send, typename T_Recv, typename T_Op>	
			void allReduce(const Context context, T_Op op, const T_Send& sendData, T_Recv& recvData){...}
			
			template <typename T_SendRecv>
			void broadcast(const VAddr rootVAddr, const Context context, const T_SendRecv& data){...}
			
			void synchronize(const Context context){...}

		    /*********************************************************************************	
		     * CONTEXT MANAGEMENT OPERATIONS
			 *********************************************************************************/
			Context createContext(const std::vector<VAddr> peers, const Context oldContext){...}
				
			Context getGlobalContext(){...}
				
	    }
				
	} /* namespace communicationPolicy*/
				
} /* namespace graybat */
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
