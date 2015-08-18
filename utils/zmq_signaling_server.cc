// STL
#include <iostream> /* std::cout, std::endl */
#include <map>      /* std::map */
#include <sstream>  /* std::stringstream */

// CLIB
#include <string.h>   /* strup */

// ZMQ
#include <zmq.hpp>

// Type defs
typedef unsigned Tag;                                            
typedef unsigned ContextID;
typedef unsigned VAddr;
typedef unsigned MsgType;
typedef std::string Uri;

// Message tags
static const MsgType VADDR_REQUEST = 0;
static const MsgType VADDR_LOOKUP  = 1;
static const MsgType DESTRUCT      = 2;
static const MsgType RETRY         = 3;
static const MsgType ACK           = 4;

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

    std::map<VAddr, Uri> phoneBook;
    std::string masterUri = std::getenv("GRAYBAT_ZMQ_MASTER_URI");
    zmq::context_t context(1);
    
    zmq::message_t request;
    zmq::message_t reply;
    zmq::socket_t socket (context, ZMQ_REP);
    socket.bind(masterUri.c_str());

                
    VAddr maxVAddr = 0;
    unsigned nPeers = 0;

    while(true){
        std::stringstream ss;
        ss << s_recv(socket);

        Uri srcUri;
        MsgType type;
        ss >> type;

        switch(type){
                        
        case VADDR_REQUEST:
            {
                // Reply with correct information
                ss >> srcUri;
                phoneBook[maxVAddr] = srcUri;
                // Send requestet vAddr
                s_send(socket, std::to_string(maxVAddr++).c_str());
                nPeers++;
                break;
            }
                        
        case VADDR_LOOKUP:
            {
                VAddr remoteVAddr;
                ss >> remoteVAddr;

                std::stringstream sss;

                if(phoneBook.count(remoteVAddr) == 0){
                    sss << RETRY;
                    s_send(socket, sss.str().c_str());
                }
                else {
                    sss << ACK << " " << phoneBook[remoteVAddr];
                    s_send(socket, sss.str().c_str());
                }


                break;
            }


        case DESTRUCT:
            nPeers--;
            s_send(socket, "");
            break;
                        
        default:
            // Reply empty message
            s_send(socket, "");
            // TODO: throw exceptions since type should be specified
            break;

        };

        // All peers have destructed, so stop managing Peers
        if(nPeers == 0){
            return 0;
        }
                    
    }


}
