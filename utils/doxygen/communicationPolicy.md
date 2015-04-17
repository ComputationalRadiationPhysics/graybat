Communication Policy
====================

The communication policy is a class which implements the
communication interface of its host class ([cage](utils/doxygen/cage.md)).

Communication is modeled in GrayBat in the way, that an instance
that takes part on whatever communication is called a peer. All
peers that want to communicate in some way with each other need
to group up in a Context. Therefore, a Context is a set of peers
that are able to communicate with each other.

By communication is meant the exchange of arbitrary data between peers
or even within one peer. Thus, communication can mean sending a
message over the internet, copying data between two memories, or
distributing data with the help of MPI. Therefore, a communication policy
does only need to implement the required interface but can
interpret the term communication on its own.

The communication policy interface is separated into the parts
**point-to-point communication**, **collective communication**, and
**context management**. The advice for completeness is to implement all
methods of these three parts and run the provided unit test cases to
verify their correctness. Nevertheless, not all communication methods
might be necessary for a special use case and only a handful is really
necessary to initialize the [cage](utils/doxygen/cage.md). Thus, it is possible to
provide a communication policy implementation that does not implement
all interface methods.

The basic set of methods that **need** to be implemented is the
following:

* Context Management
 * getGlobalContext()
 * createContext()
* Collective Communication
 * allGather()
 * allReduce()

