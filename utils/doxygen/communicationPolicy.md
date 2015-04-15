Communication Policy
====================

The communication policy is a component of GrayBat that is responsible
for communication processes. By communication is meant the exchange of
arbitrary data between peers or even within one peer. Thus,
communication can mean sending a message over the internet, copying
data between two memories, or distributing data with the help of
MPI. It is provided to the communication and graph environment
([Cage](@ref Cage)) as an template argument such as
```graybat::Cage<CommunicationPolicy, ...>```.

**Define Peer**

The communication policy interface is separated into the parts
*point-to-point communication*, *collective communication*, and
*context management*. The advice is to implement all methods of the
three parts for completeness and run the unit test cases to verify
their correctness. Nevertheless, not all communication methods might
be necessary for a special use case and only a handful is really
necessary to initialize the communication network. Thus, it is
possible to provide a communication policy that does not implement all
interface methods.

The basic set of methods that need to be implemented is the
following:

* getGlobalContext()
* createContext()
* allGather()
* allReduce()

**Define Context**
**Define Event**
