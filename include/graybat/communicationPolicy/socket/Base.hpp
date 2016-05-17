#pragma once

// STL
#include <string> /* std::string */
#include <mutex>  /* std::mutex */
#include <map>    /* std::map */
#include <vector> /* std::vector */
#include <thread> /* std::thread */
#include <exception> /* std::runtime_error */

// BOOST
#include <boost/optional.hpp>
#include <boost/utility/in_place_factory.hpp>

// HANA
#include <boost/hana.hpp>
namespace hana = boost::hana;

// GrayBat
#include <graybat/communicationPolicy/Base.hpp>          /* graybat::communicationPolicy::Base */
#include <graybat/communicationPolicy/Traits.hpp>        /* cp related types */
#include <graybat/communicationPolicy/socket/Traits.hpp> /* socket related types */
#include <graybat/utils/MultiKeyMap.hpp>                 /* utils::MessageBox */

namespace graybat {

    namespace communicationPolicy {

        namespace socket {

            template <typename T_CommunicationPolicy>
            struct Base : graybat::communicationPolicy::Base<T_CommunicationPolicy>{

                // Types
                using CommunicationPolicy = T_CommunicationPolicy;
                using ContextID           = graybat::communicationPolicy::ContextID<CommunicationPolicy>;
                using Context             = graybat::communicationPolicy::Context<CommunicationPolicy>;
                using Tag                 = graybat::communicationPolicy::Tag<CommunicationPolicy>;
                using VAddr               = graybat::communicationPolicy::VAddr<CommunicationPolicy>;
                using MsgType             = graybat::communicationPolicy::MsgType<CommunicationPolicy>;
                using MsgID               = graybat::communicationPolicy::MsgID<CommunicationPolicy>;
                using Event               = graybat::communicationPolicy::Event<CommunicationPolicy>;
                using Config              = graybat::communicationPolicy::Config<CommunicationPolicy>;
                using Message             = graybat::communicationPolicy::socket::Message<CommunicationPolicy>;
                using Uri                 = graybat::communicationPolicy::socket::Uri<CommunicationPolicy>;
                using Socket              = graybat::communicationPolicy::socket::Socket<CommunicationPolicy>;
                using ContextName         = graybat::communicationPolicy::socket::ContextName<CommunicationPolicy>;

                // Members
                const Uri masterUri;
                const size_t contextSize;
                const ContextName contextName;
                unsigned maxMsgID;
                std::mutex sendMtx;
                std::map<ContextID, std::map<VAddr, std::size_t> > sendSocketMappings;
                utils::MessageBox<Message, MsgType, ContextID, VAddr, Tag> inBox;
                utils::MessageBox<Message, MsgType, ContextID, VAddr, Tag> ctrlBox;


                std::map<ContextID, Context> contexts;

                Context initialContext;
                std::map<ContextID, std::map<VAddr, Uri> > phoneBook;
                std::map<ContextID, std::map<VAddr, Uri> > ctrlPhoneBook;
                std::map<ContextID, std::map<Uri, VAddr> > inversePhoneBook;
                std::map<ContextID, std::map<Uri, VAddr> > inverseCtrlPhoneBook;
                std::map<ContextID, ContextName> contextNames;

                std::thread recvHandler;
                std::thread ctrlHandler;

                // Construct/Destruct Methods
                Base(Config const config);

                ~Base();

                void init();
                void deinit();


                // Socket Interface
                template <typename T_Socket>
                void connectToSocket(T_Socket& socket, std::string const signalingUri) = delete;

                template <typename T_Socket>
                void sendToSocket(T_Socket& socket, std::stringstream const ss) = delete;

                template <typename T_Socket, typename T_Data>
                void sendToSocket(T_Socket& socket, T_Data const data) = delete;

                template <typename T_Socket>
                void recvFromSocket(T_Socket& socket, std::stringstream ss) = delete;

                template <typename T_Socket>
                void recvFromSocket (T_Socket& socket, Message & message) = delete;

                void createSocketsToPeers() = delete;

                // P2P INTERFACE

                /**
                 * @brief Blocking transmission of a message sendData to peer with virtual address destVAddr.
                 * 
                 * @param[in] destVAddr  VAddr of peer that will receive the message
                 * @param[in] tag        Description of the message to better distinguish messages types
                 * @param[in] context    Context in which both sender and receiver are included
                 * @param[in] sendData   Data reference of template type T will be send to receiver peer.
                 *                       T need to provide the function data(), that returns the pointer
                 *                       to the data memory address. And the function size(), that
                 *                       return the amount of data elements to send. Notice, that
                 *                       std::vector and std::array implement this interface.
                 */
                template <typename T_Send>
                void send(const VAddr destVAddr, const Tag tag, const Context context, const T_Send& sendData);

                /** 
                 * @brief Non blocking transmission of a message sendData to peer with virtual address destVAddr.
                 * 
                 * @param[in] destVAddr  VAddr of peer that will receive the message
                 * @param[in] tag        Description of the message to better distinguish messages types
                 * @param[in] context    Context in which both sender and receiver are included
                 * @param[in] sendData   Data reference of template type T will be.
                 *                       T need to provide the function data(), that returns the pointer
                 *                       to the data memory address. And the function size(), that
                 *                       return the amount of data elements to send. Notice, that
                 *                       std::vector and std::array implement this interface.
                 *
                 * @return Event
                 */
                template <typename T_Send>
                Event asyncSend(const VAddr destVAddr, const Tag tag, const Context context, const T_Send& sendData);

                /**
                 * @brief Blocking receive of a message recvData from peer with virtual address srcVAddr.
                 * 
                 * @param[in]  srcVAddr   VAddr of peer that sended the message
                 * @param[in]  tag        Description of the message to better distinguish messages types
                 * @param[in]  context    Context in which both sender and receiver are included
                 * @param[out] recvData   Data reference of template type T will be received from sender peer.
                 *                        T need to provide the function data(), that returns the pointer
                 *                        to the data memory address. And the function size(), that
                 *                        return the amount of data elements to send. Notice, that
                 *                        std::vector and std::array implement this interface.
                 */
                template <typename T_Recv>
                void recv(const VAddr srcVAddr, const Tag tag, const Context context, T_Recv& recvData);

                template <typename T_Recv>
                Event recv(const Context context, T_Recv& recvData);

                template <typename T_Recv>
                Event asyncRecv(const VAddr srcVAddr, const Tag tag, const Context context, T_Recv& recvData);

                // EVENT INTERFACE
                bool ready(const MsgID msgID, const Context context, const VAddr vAddr, const Tag tag);

                // CONTEXT INTERFACE
                Context getGlobalContext();
                Context splitContext(const bool isMember, const Context oldContext);
                Context updateContext(const Context oldContext);

                // SIGNALING METHODS
                template <typename T_Socket>
                ContextID getInitialContextID(T_Socket& socket, size_t const contextSize);

                template <typename T_Socket>
                ContextID getContextID(T_Socket& socket, ContextName const contextName);

                template <typename T_Socket>
                VAddr getVAddr(T_Socket &socket, ContextID const contextID, Uri const uri, Uri const ctrlUri);

                template <typename T_Socket>
                std::pair<Uri,Uri> getUri(T_Socket& socket, ContextID const contextID, VAddr const vAddr);


                // Auxilary
                template <typename T_Send>
                void asyncSendImpl(MsgType const msgType, MsgID const msgID, Context const context,VAddr const destVAddr, Tag const tag, T_Send & sendData);

                template <typename T_Recv>
                void recvImpl(MsgType const msgType, Context const context,VAddr const destVAddr, Tag const tag, T_Recv & recvData);

                template <typename T_Recv>
                Event recvImpl(Context const context, T_Recv & recvData);

                template <typename T_Recv>
                bool asyncRecvImpl(MsgType const msgType, Context const context,VAddr const destVAddr, Tag const tag, T_Recv & recvData);

                template <typename T_Recv>
                bool asyncRecvImpl(MsgType const msgType, Context const context,VAddr const destVAddr, Tag const tag, T_Recv * recvData, size_t const size);


                MsgID getMsgID();

                void handleRecv();
                void handleCtrl();

            };


            template <typename T_CommunicationPolicy>
            Base<T_CommunicationPolicy>::Base(Config const config)  :
                    masterUri(config.masterUri),
                    contextSize(config.contextSize),
                    contextName(config.contextName),
                    maxMsgID(0),
                    inBox(config.maxBufferSize),
                    ctrlBox(config.maxBufferSize){

            }


            template <typename T_CommunicationPolicy>
            Base<T_CommunicationPolicy>::~Base() {

            }

            template <typename T_CommunicationPolicy>
            auto Base<T_CommunicationPolicy>::init()
            -> void {

                // Connect to signaling process
                static_cast<CommunicationPolicy*>(this)->connectToSocket(static_cast<CommunicationPolicy*>(this)->signalingSocket, masterUri);

                // Retrieve Context id for initial context from signaling process
                ContextID contextID = getContextID(static_cast<CommunicationPolicy*>(this)->signalingSocket, contextName);
                contextNames[contextID] = contextName;

                // Retrieve own vAddr from signaling process for initial context
                VAddr vAddr = getVAddr(static_cast<CommunicationPolicy*>(this)->signalingSocket, contextID, static_cast<CommunicationPolicy*>(this)->peerUri, static_cast<CommunicationPolicy*>(this)->ctrlUri);
                initialContext = Context(contextID, vAddr, contextSize);
                contexts[initialContext.getID()] = initialContext;

                // Retrieve for uris of other peers from signaling process for the initial context
                for(auto const &vAddr : initialContext){                    
                    Uri remoteUri;
                    Uri ctrlUri;
                    std::tie(remoteUri, ctrlUri) = getUri(static_cast<CommunicationPolicy*>(this)->signalingSocket, initialContext.getID(), vAddr);
                    phoneBook[initialContext.getID()][vAddr] = remoteUri;
                    ctrlPhoneBook[initialContext.getID()][vAddr] = ctrlUri;
                    inversePhoneBook[initialContext.getID()][remoteUri] = vAddr;
                    inverseCtrlPhoneBook[initialContext.getID()][ctrlUri] = vAddr;
                }

                // Create socket connection to other peers
                // Create socketmapping from initial context to sockets of VAddrs
                static_cast<CommunicationPolicy*>(this)->createSocketsToPeers();

                for(auto const &vAddr : initialContext){                    
                    sendSocketMappings[initialContext.getID()][vAddr] = vAddr;
                    static_cast<CommunicationPolicy*>(this)->connectToSocket(static_cast<CommunicationPolicy*>(this)->sendSockets.at(sendSocketMappings.at(initialContext.getID()).at(vAddr)), phoneBook.at(initialContext.getID()).at(vAddr).c_str());
                    static_cast<CommunicationPolicy*>(this)->connectToSocket(static_cast<CommunicationPolicy*>(this)->ctrlSendSockets.at(sendSocketMappings.at(initialContext.getID()).at(vAddr)), ctrlPhoneBook.at(initialContext.getID()).at(vAddr).c_str());

                    //std::cout << "sendSocket_i: " << vAddr << " --> " << phoneBook.at(initialContext.getID()).at(vAddr) << std::endl;

                }

                // Create thread which recv all messages to this peer
                recvHandler = std::thread(&Base<CommunicationPolicy>::handleRecv, this);
                ctrlHandler = std::thread(&Base<CommunicationPolicy>::handleCtrl, this);

            }

            template <typename T_CommunicationPolicy>
            auto Base<T_CommunicationPolicy>::deinit()
            -> void {
                std::stringstream ss;
                ss << static_cast<size_t>(MsgType::DESTRUCT) << " " << contextName;
                static_cast<CommunicationPolicy*>(this)->sendToSocket(static_cast<CommunicationPolicy*>(this)->signalingSocket, ss);

                std::array<unsigned, 1>  null;
                static_cast<CommunicationPolicy*>(this)->asyncSendImpl(MsgType::DESTRUCT, 0, initialContext, initialContext.getVAddr(), 0, null);
                recvHandler.join();
                ctrlHandler.join();

            }

            template <typename T_CommunicationPolicy>
            template <typename T_Send>
            auto Base<T_CommunicationPolicy>::send(const graybat::communicationPolicy::VAddr<T_CommunicationPolicy> destVAddr,
                                                   const graybat::communicationPolicy::Tag<T_CommunicationPolicy> tag,
                                                   const graybat::communicationPolicy::Context<T_CommunicationPolicy> context,
                                                   const T_Send& sendData)
            -> void
            {
                using CommunicationPolicy = T_CommunicationPolicy;
                using Event               = graybat::communicationPolicy::Event<CommunicationPolicy>;

                Event e = asyncSend(destVAddr, tag, context, sendData);
                e.wait();
            }

            template <typename T_CommunicationPolicy>
            template <typename T_Send>
            auto Base<T_CommunicationPolicy>::asyncSend(const graybat::communicationPolicy::VAddr<T_CommunicationPolicy> destVAddr,
                                                        const graybat::communicationPolicy::Tag<T_CommunicationPolicy> tag,
                                                        const graybat::communicationPolicy::Context<T_CommunicationPolicy> context,
                                                        const T_Send& sendData)
            -> graybat::communicationPolicy::Event<T_CommunicationPolicy>
            {
                using CommunicationPolicy = T_CommunicationPolicy;
                using MsgType             = graybat::communicationPolicy::MsgType<CommunicationPolicy>;
                using MsgID               = graybat::communicationPolicy::MsgID<CommunicationPolicy>;
                using Event               = graybat::communicationPolicy::Event<CommunicationPolicy>;

                //std::cout << "send method[" << context.getVAddr() << "]: " << context.getID() << " " << destVAddr << " " << tag << std::endl;
                MsgID msgID = getMsgID();
                asyncSendImpl(MsgType::PEER, msgID, context, destVAddr, tag, sendData);

                return Event(msgID, context, destVAddr, tag, *static_cast<CommunicationPolicy*>(this));

            }

            template <typename T_CommunicationPolicy>
            template <typename T_Recv>
            auto Base<T_CommunicationPolicy>::recv(const graybat::communicationPolicy::VAddr<T_CommunicationPolicy> srcVAddr,
                                                   const graybat::communicationPolicy::Tag<T_CommunicationPolicy> tag,
                                                   const graybat::communicationPolicy::Context<T_CommunicationPolicy> context,
                                                   T_Recv& recvData)
            -> void
            {
                using CommunicationPolicy = T_CommunicationPolicy;
                using MsgType             = graybat::communicationPolicy::MsgType<CommunicationPolicy>;

                //std::cout << "recv method[" << context.getVAddr() << "]:" << context.getID() << " " << srcVAddr << " " << tag << std::endl;
                recvImpl(MsgType::PEER, context, srcVAddr, tag, recvData);

            }


            template <typename T_CommunicationPolicy>
            template <typename T_Recv>
            auto Base<T_CommunicationPolicy>::recv(const graybat::communicationPolicy::Context<T_CommunicationPolicy> context,
                                                   T_Recv& recvData)
            -> graybat::communicationPolicy::Event<T_CommunicationPolicy>
            {
                return recvImpl(context, recvData);
            }


            template <typename T_CommunicationPolicy>
            template <typename T_Recv>
            auto Base<T_CommunicationPolicy>::asyncRecv(const graybat::communicationPolicy::VAddr<T_CommunicationPolicy> srcVAddr,
                                                        const graybat::communicationPolicy::Tag<T_CommunicationPolicy> tag,
                                                        const graybat::communicationPolicy::Context<T_CommunicationPolicy> context,
                                                        T_Recv& recvData)
            -> graybat::communicationPolicy::Event<T_CommunicationPolicy>
            {
                bool result = asyncRecvImpl(MsgType::PEER, context, srcVAddr, tag, recvData);
                //std::cout << "Size before event creation: " << sizeof(typename T_Recv::value_type) * recvData.size() << std::endl;
                return Event(getMsgID(), context, srcVAddr, tag, recvData, result, *(static_cast<CommunicationPolicy*>(this)));
            }

            template <typename T_CommunicationPolicy>
            auto Base<T_CommunicationPolicy>::ready(const graybat::communicationPolicy::MsgID<T_CommunicationPolicy> msgID,
                                                    const graybat::communicationPolicy::Context<T_CommunicationPolicy> context,
                                                    const graybat::communicationPolicy::VAddr<T_CommunicationPolicy> vAddr,
                                                    const graybat::communicationPolicy::Tag<T_CommunicationPolicy> tag)
            -> bool
            {
                using CommunicationPolicy = T_CommunicationPolicy;
                using Message             = graybat::communicationPolicy::socket::Message<CommunicationPolicy>;
                using MsgType             = graybat::communicationPolicy::MsgType<CommunicationPolicy>;

                Message message(std::move(ctrlBox.waitDequeue(MsgType::CONFIRM, context.getID(), vAddr, tag)));

                // size_t    msgOffset = 0;
                // MsgType   remoteMsgType;
                // MsgID     remoteMsgID;


                if(message.getMsgID() == msgID){
                    return true;
                }
                else {
                    ctrlBox.enqueue(std::move(message), MsgType::CONFIRM, context.getID(), vAddr, tag);
                }

                return false;

            }

            /*******************************************************************
             *
             ******************************************************************/
            template <typename T_CommunicationPolicy>
            auto Base<T_CommunicationPolicy>::getGlobalContext()
            -> graybat::communicationPolicy::Context<T_CommunicationPolicy> {
                return initialContext;
            }


            template <typename T_CommunicationPolicy>
            auto Base<T_CommunicationPolicy>::splitContext(const bool isMember,
                                                           const graybat::communicationPolicy::Context<T_CommunicationPolicy> oldContext)
            -> graybat::communicationPolicy::Context<T_CommunicationPolicy> {

                using CommunicationPolicy = T_CommunicationPolicy;
                using Context             = graybat::communicationPolicy::Context<CommunicationPolicy>;

                //std::cout  << oldContext.getVAddr() << " splitcontext entry" << std::endl;

                Context newContext;
                VAddr masterVAddr = 0;

                // Request old master for new context
                std::array<unsigned, 1> member {{ isMember }};
                static_cast<CommunicationPolicy*>(this)->asyncSendImpl(MsgType::SPLIT, getMsgID(), oldContext, masterVAddr, 0, member);

                // Peer with VAddr 0 collects new members
                if( oldContext.getVAddr() == masterVAddr){
                    std::array<ContextID, 1> newContextID {{ 0 }};
                    std::array<unsigned, 1> newContextSize {{ 0 }};
                    std::vector<VAddr> newContextWhiteList(0, 0);

                    for(auto const &vAddr : oldContext){                        
                        std::array<unsigned, 1> remoteIsMember {{ 0 }};
                        //std::cout << "Recv remoteIsMember: " <<  vAddr << std::endl;
                        static_cast<CommunicationPolicy*>(this)->recvImpl(MsgType::SPLIT, oldContext, vAddr, 0, remoteIsMember);

                        if(remoteIsMember[0]) {
                            newContextWhiteList.push_back(vAddr);

                        }

                    }

                    ContextName newContextName = contextName + "_" + std::to_string(std::rand());
                    newContextID[0]   = getContextID(static_cast<CommunicationPolicy*>(this)->signalingSocket, newContextName);
                    contextNames[newContextID[0]] = newContextName;
                    newContextSize[0] = newContextWhiteList.size();

                    for(VAddr vAddr : newContextWhiteList){
                        static_cast<CommunicationPolicy*>(this)->asyncSendImpl(MsgType::SPLIT, getMsgID(), oldContext, vAddr, 0, newContextID);
                        static_cast<CommunicationPolicy*>(this)->asyncSendImpl(MsgType::SPLIT, getMsgID(), oldContext, vAddr, 0, newContextSize);
                        static_cast<CommunicationPolicy*>(this)->asyncSendImpl(MsgType::SPLIT, getMsgID(), oldContext, vAddr, 0, newContextWhiteList);

                    }

                }

                //std::cout << oldContext.getVAddr() << " check 0" << std::endl;

                if(isMember){
                    std::array<ContextID, 1> newContextID {{  0 }};
                    std::array<unsigned, 1> newContextSize {{ 0 }};

                    static_cast<CommunicationPolicy*>(this)->recvImpl(MsgType::SPLIT, oldContext, 0, 0, newContextID);
                    static_cast<CommunicationPolicy*>(this)->recvImpl(MsgType::SPLIT, oldContext, 0, 0, newContextSize);

                    std::vector<VAddr> newContextWhiteList(newContextSize[0], 0);

                    static_cast<CommunicationPolicy*>(this)->recvImpl(MsgType::SPLIT, oldContext, 0, 0, newContextWhiteList);

                    newContext = Context(newContextID[0], oldContext.getVAddr(), newContextWhiteList);
                    contexts[newContext.getID()] = newContext;

                    //std::cout  << oldContext.getVAddr() << " check 1" << std::endl;
                    // Update phonebook for new context
                    for(auto const &vAddr : newContext){                        
                        Uri remoteUri = phoneBook[oldContext.getID()][vAddr];
                        Uri ctrlUri   = ctrlPhoneBook[oldContext.getID()][vAddr];
                        phoneBook[newContext.getID()][vAddr] = remoteUri;
                        ctrlPhoneBook[newContext.getID()][vAddr] = ctrlUri;
                        inversePhoneBook[newContext.getID()][remoteUri]   = inversePhoneBook[oldContext.getID()][remoteUri];
                        inverseCtrlPhoneBook[newContext.getID()][ctrlUri] = inverseCtrlPhoneBook[oldContext.getID()][ctrlUri];

                    }

                    //std::cout  << oldContext.getVAddr() << " check 2" << std::endl;
                    // Create mappings to sockets for new context
                    for(auto const &vAddr : newContext){                        
                        Uri uri = phoneBook.at(oldContext.getID()).at(vAddr);
                        VAddr oldVAddr = inversePhoneBook.at(oldContext.getID()).at(uri);
                        sendSocketMappings[newContext.getID()][vAddr] = sendSocketMappings.at(oldContext.getID()).at(oldVAddr);

                    }

                }
                else{
                    // Invalid context for "not members"
                    newContext = Context();
                }

                //static_cast<CommunicationPolicy*>(this)->synchronize(oldContext);

                //std::cout  << oldContext.getVAddr() << " check 3" << std::endl;

                // Barrier thus recvHandler is up to date with sendSocketMappings
                // Necessary in environment with multiple zmq objects
                std::array<unsigned, 0> null;
                for(auto const &vAddr : oldContext){
                    static_cast<CommunicationPolicy*>(this)->asyncSendImpl(MsgType::SPLIT, getMsgID(), oldContext, vAddr, 0, null);
                }
                for(auto const &vAddr : oldContext){
                    static_cast<CommunicationPolicy*>(this)->recvImpl(MsgType::SPLIT, oldContext, vAddr, 0, null);
                }

                //std::cout  << oldContext.getVAddr() << " splitContext end" << std::endl;		 
                return newContext;

            }

            template <typename T_CommunicationPolicy>
            template <typename T_Socket>
            auto Base<T_CommunicationPolicy>::getInitialContextID(T_Socket& socket, size_t const contextSize)
            -> graybat::communicationPolicy::ContextID<T_CommunicationPolicy> {
                ContextID contextID = 0;
                // Send vAddr request
                std::stringstream ss;
                ss << static_cast<size_t>(MsgType::CONTEXT_INIT) << " " << contextSize;
                static_cast<CommunicationPolicy*>(this)->sendToSocket(socket, ss);

                // Recv vAddr
                std::stringstream sss;
                static_cast<CommunicationPolicy*>(this)->recvFromSocket(socket, sss);
                sss >> contextID;
                return contextID;

            }


            template <typename T_CommunicationPolicy>
            template <typename T_Socket>
            auto Base<T_CommunicationPolicy>::getContextID(T_Socket& socket, ContextName const contextName)
            -> graybat::communicationPolicy::ContextID<T_CommunicationPolicy> {
                using ContextID = graybat::communicationPolicy::ContextID<T_CommunicationPolicy>;

                ContextID contextID = 0;

                // Send vAddr request
                std::stringstream ss;
                ss << static_cast<size_t>(MsgType::CONTEXT_REQUEST) << " " << contextName << " ";

                static_cast<CommunicationPolicy*>(this)->sendToSocket(socket, ss);

                // Recv vAddr
                std::stringstream sss;
                static_cast<CommunicationPolicy*>(this)->recvFromSocket(socket, sss);
                sss >> contextID;
                return contextID;

            }


            template <typename T_CommunicationPolicy>
            template <typename T_Socket>
            auto Base<T_CommunicationPolicy>::getVAddr(T_Socket &socket, ContextID const contextID, Uri const uri, Uri const ctrlUri)
            -> graybat::communicationPolicy::VAddr<T_CommunicationPolicy> {

                using VAddr = graybat::communicationPolicy::VAddr<T_CommunicationPolicy>;

                VAddr vAddr(0);
                // Send vAddr request
                std::stringstream ss;
                ss << static_cast<size_t>(MsgType::VADDR_REQUEST) << " " << contextID << " " << uri << " " << ctrlUri << " ";
                static_cast<CommunicationPolicy*>(this)->sendToSocket(socket, ss);

                // Recv vAddr
                std::stringstream sss;
                static_cast<CommunicationPolicy*>(this)->recvFromSocket(socket, sss);
                sss >> vAddr;

                return vAddr;

            }


            template <typename T_CommunicationPolicy>
            template <typename T_Socket>
            auto Base<T_CommunicationPolicy>::getUri(T_Socket& socket,
                                                     graybat::communicationPolicy::ContextID<T_CommunicationPolicy> const contextID,
                                                     graybat::communicationPolicy::VAddr<T_CommunicationPolicy> const vAddr)
            -> std::pair<graybat::communicationPolicy::socket::Uri<T_CommunicationPolicy>,
                    graybat::communicationPolicy::socket::Uri<T_CommunicationPolicy>
            > {

                MsgType type = MsgType::RETRY;

                while(type == MsgType::RETRY){
                    // Send vAddr lookup
                    std::stringstream ss;
                    ss << static_cast<size_t>(MsgType::VADDR_LOOKUP) << " " << contextID << " " << vAddr;
                    static_cast<CommunicationPolicy*>(this)->sendToSocket(socket, ss);

                    // Recv uri
                    std::string msgTypeStr;
                    std::string remoteUri;
                    std::string ctrlUri;
                    std::stringstream sss;
                    static_cast<CommunicationPolicy*>(this)->recvFromSocket(socket, sss);
                    sss >> msgTypeStr;
                    type = static_cast<MsgType>(std::stoi(msgTypeStr));
                    if(type == MsgType::ACK){
                        sss >> remoteUri;
                        sss >> ctrlUri;
                        return std::make_pair(remoteUri, ctrlUri);

                    }

                }
                return std::make_pair(std::string(""), std::string(""));

            }


            template <typename T_CommunicationPolicy>
            template <typename T_Send>
            auto Base<T_CommunicationPolicy>::asyncSendImpl(graybat::communicationPolicy::MsgType<T_CommunicationPolicy> const msgType,
                                                            graybat::communicationPolicy::MsgID<T_CommunicationPolicy> const msgID,
                                                            graybat::communicationPolicy::Context<T_CommunicationPolicy> const context,
                                                            graybat::communicationPolicy::VAddr<T_CommunicationPolicy> const destVAddr,
                                                            graybat::communicationPolicy::Tag<T_CommunicationPolicy> const tag,
                                                            T_Send & sendData)
            -> void {

                using Message   = graybat::communicationPolicy::socket::Message<T_CommunicationPolicy>;

                //std::cout << "send msg: " << static_cast<int>(msgType) << " " << msgID << " " << context.getID() << " " << destVAddr << "(socket_i " <<  sendSocketMappings.at(context.getID()).at(destVAddr)<< ") " << tag << std::endl;

                // Create message
                Message message(msgType, msgID, context.getID(), context.getVAddr(), tag, sendData);

                std::size_t sendSocket_i  = sendSocketMappings.at(context.getID()).at(destVAddr);
                Socket &sendSocket = static_cast<CommunicationPolicy*>(this)->sendSockets.at(sendSocket_i);
                Socket &ctrlSendSocket = static_cast<CommunicationPolicy*>(this)->ctrlSendSockets.at(sendSocket_i);


                sendMtx.lock();
                if(msgType == MsgType::CONFIRM){
                    static_cast<CommunicationPolicy*>(this)->sendToSocket(ctrlSendSocket, message.getMessage());
                }
                else {

                    if(msgType == MsgType::DESTRUCT){
                        Message message2(msgType, msgID, context.getID(), context.getVAddr(), tag, sendData);
                        static_cast<CommunicationPolicy*>(this)->sendToSocket(sendSocket, message.getMessage());
                        static_cast<CommunicationPolicy*>(this)->sendToSocket(ctrlSendSocket, message2.getMessage());

                    }
                    else {
                        static_cast<CommunicationPolicy*>(this)->sendToSocket(sendSocket, message.getMessage());
                    }
                }
                sendMtx.unlock();

            }

            template <typename T_CommunicationPolicy>
            template <typename T_Recv>
            auto Base<T_CommunicationPolicy>::recvImpl(graybat::communicationPolicy::MsgType<T_CommunicationPolicy> const msgType,
                                                       graybat::communicationPolicy::Context<T_CommunicationPolicy> const context,
                                                       graybat::communicationPolicy::VAddr<T_CommunicationPolicy> const srcVAddr,
                                                       graybat::communicationPolicy::Tag<T_CommunicationPolicy> const tag,
                                                       T_Recv& recvData)
            -> void {

                using Message   = graybat::communicationPolicy::socket::Message<T_CommunicationPolicy>;

                Message message(std::move(inBox.waitDequeue(msgType, context.getID(), srcVAddr, tag)));
                memcpy (static_cast<void*>(recvData.data()),
                        static_cast<std::int8_t*>(message.getData()),
                        sizeof(typename T_Recv::value_type) * recvData.size());

            }


            template <typename T_CommunicationPolicy>
            template <typename T_Recv>
            auto Base<T_CommunicationPolicy>::recvImpl(graybat::communicationPolicy::Context<T_CommunicationPolicy> const context,
                                                       T_Recv& recvData)
            -> graybat::communicationPolicy::Event<T_CommunicationPolicy> {

                using CommunicationPolicy = T_CommunicationPolicy;
                using Event   = graybat::communicationPolicy::Event<CommunicationPolicy>;
                using Message = graybat::communicationPolicy::socket::Message<CommunicationPolicy>;

                hana::tuple<MsgType, ContextID, VAddr, Tag> keys;
                VAddr destVAddr;
                Tag tag;

                Message message(std::move(inBox.waitDequeue(keys, MsgType::PEER, context.getID())));
                destVAddr = hana::at(keys, hana::size_c<2>);
                tag = hana::at(keys, hana::size_c<3>);

                memcpy (static_cast<void*>(recvData.data()),
                        static_cast<std::int8_t*>(message.getData()),
                        sizeof(typename T_Recv::value_type) * recvData.size());

                return Event(getMsgID(), context, destVAddr, tag, *(static_cast<CommunicationPolicy*>(this)));

            }


            template <typename T_CommunicationPolicy>
            template <typename T_Recv>
            auto Base<T_CommunicationPolicy>::asyncRecvImpl(graybat::communicationPolicy::MsgType<T_CommunicationPolicy> const msgType,
                                                            graybat::communicationPolicy::Context<T_CommunicationPolicy> const context,
                                                            graybat::communicationPolicy::VAddr<T_CommunicationPolicy> const srcVAddr,
                                                            graybat::communicationPolicy::Tag<T_CommunicationPolicy> const tag,
                                                            T_Recv& recvData)
            -> bool {

                using Message = graybat::communicationPolicy::socket::Message<T_CommunicationPolicy>;
                bool result = false;
                Message message = std::move(inBox.tryDequeue(result, msgType, context.getID(), srcVAddr, tag));
                if(result) {
                    memcpy (static_cast<void*>(recvData.data()),
                            static_cast<std::int8_t*>(message.getData()),
                            sizeof(typename T_Recv::value_type) * recvData.size());
                    return true;
                }
                return false;
            }

            template <typename T_CommunicationPolicy>
            template <typename T_Recv>
            auto Base<T_CommunicationPolicy>::asyncRecvImpl(graybat::communicationPolicy::MsgType<T_CommunicationPolicy> const msgType,
                                                            graybat::communicationPolicy::Context<T_CommunicationPolicy> const context,
                                                            graybat::communicationPolicy::VAddr<T_CommunicationPolicy> const srcVAddr,
                                                            graybat::communicationPolicy::Tag<T_CommunicationPolicy> const tag,
                                                            T_Recv * recvData,
                                                            size_t const size)
            -> bool {

                using Message = graybat::communicationPolicy::socket::Message<T_CommunicationPolicy>;
                bool result = false;
                Message message = std::move(inBox.tryDequeue(result, msgType, context.getID(), srcVAddr, tag));
                if(result) {
                    //std::cout << "Copy data: " << "bytes: " << size << " " << "0: " << reinterpret_cast<unsigned*>(message.getData())[0] << " 1: " << reinterpret_cast<unsigned*>(message.getData())[1]<< std::endl;
                     memcpy (recvData,
                             static_cast<std::int8_t*>(message.getData()),
                             size);
                    return true;
                }
                else {
                    //std::cout << "No message received" << std::endl;
                }
                return false;
            }


            template <typename T_CommunicationPolicy>
            auto Base<T_CommunicationPolicy>::getMsgID()
            -> graybat::communicationPolicy::MsgID<T_CommunicationPolicy> {
                return maxMsgID++;
            }


            template <typename T_CommunicationPolicy>
            auto Base<T_CommunicationPolicy>::handleRecv()
            -> void {

                using CommunicationPolicy = T_CommunicationPolicy;
                using Message = graybat::communicationPolicy::socket::Message<CommunicationPolicy>;

                while(true){

                    Message message;
                    static_cast<CommunicationPolicy*>(this)->recvFromSocket(static_cast<CommunicationPolicy*>(this)->recvSocket, message);

                    //std::cout << "recv handler: " << static_cast<int>(message.getMsgType()) << " " << message.getMsgID() << " " << message.getContextID() << " " << message.getVAddr() << " " << message.getTag() << std::endl;

                    if(message.getMsgType() == MsgType::DESTRUCT){
                        return;
                    }

                    if(message.getMsgType() == MsgType::PEER){
                        std::array<unsigned,0>  null;
                        Context context = contexts.at(message.getContextID());
                        static_cast<CommunicationPolicy*>(this)->asyncSendImpl(MsgType::CONFIRM, message.getMsgID(), context, message.getVAddr(), message.getTag(), null);
                    }

                    inBox.enqueue(std::move(message), message.getMsgType(), message.getContextID(), message.getVAddr(), message.getTag());

                }

            }

            template <typename T_CommunicationPolicy>
            auto Base<T_CommunicationPolicy>::handleCtrl()
            -> void {

                using CommunicationPolicy = T_CommunicationPolicy;
                using Message = graybat::communicationPolicy::socket::Message<CommunicationPolicy>;

                while(true){

                    Message message;
                    static_cast<CommunicationPolicy*>(this)->recvFromSocket(static_cast<CommunicationPolicy*>(this)->ctrlSocket, message);

                    //std::cout << "recv handler: " << static_cast<int>(message.getMsgType()) << " " << message.getMsgID() << " " << message.getContextID() << " " << message.getVAddr() << " " << message.getTag() << std::endl;

                    if(message.getMsgType() == MsgType::DESTRUCT){
                        return;
                    }

                    if(message.getMsgType() == MsgType::CONFIRM){
                        ctrlBox.enqueue(std::move(message), message.getMsgType(), message.getContextID(), message.getVAddr(), message.getTag());
                    }
                    else {
                        // Throw exception
                        throw std::runtime_error("Received wrong message type on ctrl socket (not confirm).");
                    }

                }

            }

            template <typename T_CommunicationPolicy>
            auto Base<T_CommunicationPolicy>::updateContext(Context oldContext)
            -> graybat::communicationPolicy::Context<T_CommunicationPolicy> {
                using VAddr = graybat::communicationPolicy::VAddr<T_CommunicationPolicy>;

                size_t newContextSize(0);
                // Send vAddr request
                std::stringstream ss;
                ss << static_cast<size_t>(MsgType::CONTEXT_STATE) << " " << oldContext.getID() << " ";
                static_cast<CommunicationPolicy*>(this)->sendToSocket(static_cast<CommunicationPolicy*>(this)->signalingSocket, ss);

                // Recv vAddr
                std::stringstream sss;
                static_cast<CommunicationPolicy*>(this)->recvFromSocket(static_cast<CommunicationPolicy*>(this)->signalingSocket, sss);
                sss >> newContextSize;

                Context updatedCotenxt = Context(oldContext.getID(), oldContext.getVAddr(), newContextSize);

                // TODO: now create the new sockets
                for(auto const &vAddr : initialContext){
                    Uri remoteUri;
                    Uri ctrlUri;
                    std::tie(remoteUri, ctrlUri) = getUri(static_cast<CommunicationPolicy*>(this)->signalingSocket, initialContext.getID(), vAddr);
                    phoneBook[updatedCotenxt.getID()][vAddr] = remoteUri;
                    ctrlPhoneBook[updatedCotenxt.getID()][vAddr] = ctrlUri;
                    inversePhoneBook[updatedCotenxt.getID()][remoteUri] = vAddr;
                    inverseCtrlPhoneBook[updatedCotenxt.getID()][ctrlUri] = vAddr;
                }

                // Create socket connection to other peers
                // Create socketmapping from initial context to sockets of VAddrs
//                static_cast<CommunicationPolicy*>(this)->createSocketsToPeers();
//
//                for(auto const &vAddr : initialContext){
//                    sendSocketMappings[initialContext.getID()][vAddr] = vAddr;
//                    static_cast<CommunicationPolicy*>(this)->connectToSocket(static_cast<CommunicationPolicy*>(this)->sendSockets.at(sendSocketMappings.at(initialContext.getID()).at(vAddr)), phoneBook.at(initialContext.getID()).at(vAddr).c_str());
//                    static_cast<CommunicationPolicy*>(this)->connectToSocket(static_cast<CommunicationPolicy*>(this)->ctrlSendSockets.at(sendSocketMappings.at(initialContext.getID()).at(vAddr)), ctrlPhoneBook.at(initialContext.getID()).at(vAddr).c_str());
//
//                    //std::cout << "sendSocket_i: " << vAddr << " --> " << phoneBook.at(initialContext.getID()).at(vAddr) << std::endl;
//
//                }


                return updatedCotenxt;
            }


        } // socket

    } // communicationPolicy

} // graybat
