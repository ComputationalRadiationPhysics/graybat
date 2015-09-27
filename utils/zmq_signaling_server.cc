
// CLIB
#include <string.h>   /* strup */

// STL
#include <iostream> /* std::cout, std::endl */
#include <map>      /* std::map */
#include <sstream>  /* std::stringstream */

// ZMQ
#include <zmq.hpp>

// Type defs
typedef unsigned Tag;                                            
typedef unsigned ContextID;
typedef unsigned VAddr;
typedef unsigned MsgType;
typedef std::string Uri;

// Message tags
static const MsgType VADDR_REQUEST   = 0;
static const MsgType VADDR_LOOKUP    = 1;
static const MsgType DESTRUCT        = 2;
static const MsgType RETRY           = 3;
static const MsgType ACK             = 4;
static const MsgType CONTEXT_INIT    = 5;
static const MsgType CONTEXT_REQUEST = 6;

//  Receive 0MQ string from socket and convert into C string
//  Caller must free returned string. Returns NULL if the context
//  is being terminated.
static char * s_recv (void *socket) {
    char buffer [256];
    int size = zmq_recv (socket, buffer, 255, 0);
    if (size == -1)
        return NULL;
    if (size > 255)
        size = 255;
    buffer [size] = 0;
    return strdup (buffer);
}

//  Convert C string to 0MQ string and send to socket
static int s_send (void *socket, const char *string) {
    int size = zmq_send (socket, string, strlen (string), 0);
    return size;
}

int main(){
    std::cout << "Start zmq connection manager" << std::endl;

    std::map<ContextID, std::map<VAddr, Uri> > phoneBook;
    std::map<ContextID, VAddr> maxVAddr;
    std::string masterUri = std::getenv("GRAYBAT_ZMQ_MASTER_URI");
    zmq::context_t context(1);
    
    zmq::message_t request;
    zmq::message_t reply;
    zmq::socket_t socket (context, ZMQ_REP);
    socket.bind(masterUri.c_str());

    ContextID maxContextID = 0;
    ContextID maxInitialContextID = maxContextID;    
    unsigned nPeers = 0;

    while(true){
        std::stringstream ss;
        ss << s_recv(socket);

        Uri srcUri;
        MsgType type;
	ContextID contextID;
	unsigned size;
        ss >> type;

        switch(type){

	case CONTEXT_INIT:
	    {

		ss >> size;
		if(maxVAddr[maxInitialContextID]  == size){
		    maxInitialContextID = ++maxContextID;
		    maxVAddr[maxInitialContextID] = 0;
		}
		s_send(socket, std::to_string(maxInitialContextID).c_str());
		std::cout << "CONTEXT INIT [size:" << size << "]: " << maxInitialContextID << std::endl;		
		break;
	    }

	case CONTEXT_REQUEST:
	    {
		ss >> size;
		maxVAddr[maxContextID] = 0;
		s_send(socket, std::to_string(++maxContextID).c_str());
		break;

	    }
	    
        case VADDR_REQUEST:
            {
                // Reply with correct information
		ss >> contextID;
                ss >> srcUri;
		std::cout << "VADDR REQUEST [contextID:" << contextID << "][srcUri:" << srcUri << "]: " << maxVAddr[contextID] << std::endl;
                phoneBook[contextID][maxVAddr[contextID]] = srcUri;
                // Send requestet vAddr
		
                s_send(socket, std::to_string(maxVAddr[contextID]).c_str());
		maxVAddr[contextID]++;
                break;
            }
                        
        case VADDR_LOOKUP:
            {
                VAddr remoteVAddr;
		ss >> contextID;
                ss >> remoteVAddr;		


		
                std::stringstream sss;

                if(phoneBook[contextID].count(remoteVAddr) == 0){
                    sss << RETRY;
                    s_send(socket, sss.str().c_str());
                }
                else {
		    std::cout << "VADDR LOOKUP [contextID:" << contextID << "][remoteVAddr:" << remoteVAddr << "]: " << phoneBook[contextID][remoteVAddr] << std::endl;
                    sss << ACK << " " << phoneBook[contextID][remoteVAddr];
                    s_send(socket, sss.str().c_str());
                }


                break;
            }


        case DESTRUCT:
	    std::cout << "DESTRUCT" << std::endl;
            //nPeers--;
            s_send(socket, "");
            break;
                        
        default:
            // Reply empty message
            s_send(socket, "");
            // TODO: throw exceptions since type should be specified
            break;

        };

        // All peers have destructed, so stop managing Peers
        // if(nPeers == 0){
        //     return 0;
        // }
                    
    }


}
