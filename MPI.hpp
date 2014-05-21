#pragma once
#include <types.hpp>    /* CommNode */

#include <stdlib.h>     /* srand, rand */
#include <time.h>       /* time */
#include <iostream>     /* cout */
#include <string>       /* string */
#include <map>          /* map */
#include <mpi.h>

namespace CommunicationPolicy {

    class MPI{

    private:
	typedef unsigned Description;

    protected:
	typedef int URI;
	typedef MPI_Comm Context;
	std::map<Context, URI> uri;

	Context initialContext;

	enum BinaryOperation {
	    MAX,
	    MIN,
	    SUM, 
	    PROD,
	};

	MPI(){

	    int        argc;
	    char       **argv;
	    initialContext = MPI_COMM_WORLD;
	    
	    // Get URI, which is rank in the MPI world
	    URI uriTmp;
	    MPI_Init(&argc, &argv);
	    MPI_Comm_rank(MPI_COMM_WORLD, &uriTmp);
	    uri[initialContext] = uriTmp;

	    std::cout << "Init MPI " << uri[initialContext]<< std::endl;
	    
	}

	~MPI(){
	    MPI_Finalize();
	}

	void sendData(char* data, const size_t count, const URI dest, const Context context, const Description descr, std::function<void()> callback){
	    //std::cout << "MPI::Send \"" << data << "\" to URI " << dest << " with descr: " << descr << "("<< count << " Bytes)" <<std::endl;
	    std::cout << "MPI::Send to " << dest << std::endl;

	    MPI_Send(data, count, MPI_CHAR, dest, descr, context);
	    callback();

	}

	void recvData(char* data, const size_t count, const URI src, const Context context, const Description descr, std::function<void()> callback){
	    std::cout << "MPI::Recv " << std::endl;
	    MPI_Status   status;
	    // MPI_Probe(src, MPI_ANY_TAG, context, &status);


	    // URI src = status.MPI_SOURCE;
	    // unsigned descr = status.MPI_TAG;
	    
	    MPI_Recv(data, count, MPI_CHAR, src, descr, context, &status);
	    callback();
    
	}

	// Need template for operation op and
	// maybe also for datatype of operation
	void reduce(const char* sendData, const size_t count, BinaryOperation op, URI root, const Context context){
	    /*
	      char recvData[count];
	    MPI_Reduce(sendData, recvData, count, MPI_CHAR, MPI_MIN , root, context);
	    */
	    
	}


	Context createContext(std::vector<URI> ranks, const Context oldContext){
	    MPI_Comm  newContext;
	    MPI_Group oldGroup, newGroup;
	    MPI_Comm_group(oldContext, &oldGroup);
	      
	    MPI_Group_incl(oldGroup, ranks.size(), &(ranks[0]), &newGroup);
	    MPI_Comm_create(oldContext, newGroup, &newContext);
	    return newContext;
	    
	}


    };

} // namespace CommunicationPolicy
