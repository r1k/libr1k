#include <iostream>
#include <string>
#include <sstream>
#include <signal.h>
#include "mpeg/inputs/CTransportStreamMulticast.h"
#include "mpeg/inputs/CTransportStreamFile.h"
#include "mpeg/engines/CTransportStreamPreProcessor.h"
#include "mpeg/data/CTransportStreamPacketBuffer.h"
#include "mpeg/events/CTransportStreamEvent.h"
#include "mpeg/events/CBitrateListEvent.h"
#include "mpeg/events/CBitrateEvent.h"
#include "mpeg/events/CPidEvent.h"
#include "net/CIpAddress.h"
#include "event/IEventRegister.h"
#include "event/CAbsEventHandler.h"
#include "event/CEventDispatcher.h"
#include "console/CFileTraceListener.h"

#include <list>

#include "SocketListeningGroup.h"

using namespace std;
using namespace themelios;

themelios::CFileTraceListener fileListener("console.log");

class appEngine : public CAbsEventHandler<CTransportStreamEvent>
{
public:

    // multicast details
    string         source_ipMulticast;
    string         source_ipInterface;
    unsigned short source_port;

    // listening socket details
    unsigned short listening_port;
    CSocket*       listening_socket;     // this owns the master listening port

    // group of monitors
    list<CSocket *> eventReceivers;      // this owns the list of connections interested in receiving updates
    SocketListeningGroup listeningGroup; // this is the group of connections we monitor for incoming requests

    // transport stream source
    ITransportStreamSource*         source;
    CTransportStreamPreProcessor*   preProc;

    ~appEngine()
    {
        while (this->eventReceivers.size())
        {
            CSocket* pSock = this->eventReceivers.front();
            this->eventReceivers.pop_front();
            delete pSock;
        }

        if (this->listening_socket) delete this->listening_socket;
        if (this->source)           delete this->source;
        if (this->preProc)          delete this->preProc;
    }

    void openListener()
    {
        try{
            cout << "*****************************************************" << endl;
            cout << "Create main listening socket" << endl;

            this->listening_socket = new CSocket(CSocket::NET_TYPE_IPv4, CSocket::SOCKET_TYPE_STREAM, CSocket::PROTOCOL_TCP);
            this->listening_socket->bind(CIpAddress(source_ipInterface), this->listening_port);
            this->listening_socket->setReUseAddress();
            this->listening_socket->listen();
            this->listeningGroup.addSocket(this->listening_socket);

            cout << "*****************************************************" << endl;
        }
        catch(...)
        {
            cout << "Error:openListener:creating socket" << endl;
            exit (-1);
        }
    }

    virtual bool eventFilter(const CTransportStreamEvent & ev)
    {
        bool wanted = false;
        const CTransportStreamEvent *pEv = &ev;

        const CPidEvent * pe;
        const CBitrateEvent * bre;
        const CBitrateListEvent * brle;

        if (
            (pe = dynamic_cast<const CPidEvent *>(pEv))
            ||
            (bre = dynamic_cast<const CBitrateEvent *>(pEv))
            ||
            (brle = dynamic_cast<const CBitrateListEvent *>(pEv))
            )
        {
            if (pe)
            {
                if(pe->isDisappearance())
                     wanted = true;
            }
            else if (bre)
            {
                // cout << "Bitrate message for PID:" << bre->Pid << endl;
                wanted = true;
            }
            else if (brle)
            {
                // cout << "Sending bitrate list message" << endl;
                wanted = true;
            }
        }

        return wanted;
    }

    virtual void onEvent(const CTransportStreamEvent & ev)
    {
        if (eventFilter(ev))
        {
            sendEvent(ev);
        }
    }

    virtual void sendEvent(const CTransportStreamEvent& event)
    {
        for(list<CSocket *>::iterator iter = this->eventReceivers.begin(); iter != this->eventReceivers.end(); ++iter)
        {
            string json = event.toJSON();
            (*iter)->send(json.c_str(), json.length());
        }
    }

    void addClient(CSocket *newSocket)
    {
        // New connection on main port - add it to the list of sockets interested in receiving updates.
        // Add it to the group we listen for messages on.

        CSocket *remoteSock = new CSocket();
        this->listening_socket->accept(*remoteSock);
        this->eventReceivers.push_back(remoteSock);
        this->listeningGroup.addSocket(remoteSock);
    }

    void removeClient(CSocket *deadSocket)
    {
        // connection closed
        // remove from listening group
        this->listeningGroup.removeSocket(deadSocket);

        // also remove from client list
        for(list<CSocket *>::iterator iter = this->eventReceivers.begin(); iter != this->eventReceivers.end(); ++iter)
        {
            if ((*iter)->getSockDescriptor() == deadSocket->getSockDescriptor())
            {
                this->eventReceivers.erase(iter);
                break;
            }
        }

        // finally delete the memory created when the socket was originally created
        delete deadSocket;
    }
};

static CEventDispatcher eventDispatcher;
static appEngine server;
static bool sigEnd = false;

void signalHandler (int signum)
{
    sigEnd = true;
    throw ("test");
}

int generateConfig(int argc, char** argv)
{
    if (argc == 5)
    {
        istringstream sourceMulticast(argv[1]);
        sourceMulticast >> server.source_ipMulticast;

        istringstream sourcePort(argv[2]);
        sourcePort >> server.source_port;

        istringstream sourceInterface(argv[3]);
        sourceInterface >> server.source_ipInterface;

        istringstream listeningPort(argv[4]);
        listeningPort >> server.listening_port;
    }
    else
    {
        cout << "Incorrect usage:" << endl;
        cout << " eventServer <multicast IP Address> <multicast port> <server listening IP address> <server port>" << endl;
        return -1;
    }

    return 0;
}

int createSource()
{
    CTransportStreamMulticast *pMulticast = new CTransportStreamMulticast(CIpAddress(server.source_ipMulticast), server.source_port, CIpAddress(server.source_ipInterface));
    pMulticast->setRetryConnection(true);
    server.source = pMulticast;

    // create the preprocessor
    server.preProc = new CTransportStreamPreProcessor(eventDispatcher);

    // also register the "config" structure to the dispatcher
    server.registerHandler(eventDispatcher);

    return 0;
}

int main(int argc, char** argv)
{
    signal( SIGINT, signalHandler);
    signal( SIGQUIT, signalHandler);
    signal( SIGHUP, signalHandler);
    signal( SIGTERM, signalHandler);

    // generate the config
    if (generateConfig(argc, argv))
    {
        cout << "Error: Failed to generate config..." << endl;
        return 1;
    }

    int status = 1;

    if (createSource())
    {
        cout << "Error: Failed to create source" << endl;
        return 1;
    }

    try
    {
        server.openListener();

        // run the main loop
        for (;;)
        {
            if (sigEnd) break;

            try
            {
                // get the packet
                const CTransportStreamPacketBuffer& pkt = server.source->getNextPacket();

                // push to the preproc
                server.preProc->pushNextPacket(pkt);
            }
            catch (underflow_error& ue)
            {
                cout << "Error:" << __FILE__ << ":" << __LINE__ << ":Exception detected - " << ue.what() << endl;
                // let's try to carry on from this
            }


            // check for any client activity
            list<CSocket *> incomingConnections;

            if (server.listeningGroup.Select(incomingConnections, 0))
            {
                for(list<CSocket *>::iterator iter = incomingConnections.begin(); iter != incomingConnections.end(); ++iter)
                {
                    // cout << "Incoming connection received" << endl;
                    if ((*iter)->getSockDescriptor() == server.listening_socket->getSockDescriptor())
                    {
                        cout << "New client connection - adding" << endl;
                        server.addClient(*iter);
                    }
                    else
                    {
                        const int buf_size = 4096;
                        char buf[buf_size];
                        unsigned int readin = 0;
                        // cout << "Existing client connection" << endl;
                        if ((*iter)->receive((void *)buf, buf_size, readin))
                        {
                            // cout << "Client sent " << readin << " bytes" << endl;

                            if (readin == 0)
                            {
                                cout << "Removing client connection" << endl;
                                server.removeClient(*iter);
                                break;
                            }

    //						client_requests_if *req = NULL;
    //						json_parser jsonp;
    //						jsonp.from_buffer(buf, readin, req);
    //
    //						if (req)
    //						{
    //							// deal with request
    //
    //							delete req;
    //						}
                        }
                    }
                }
            }
        }
    }
    catch (exception& ex)
    {
        cout << "Error:" << __FILE__ << ":" << __LINE__ << ":Exception detected - " << ex.what() << endl;
    }
    catch (...)
    {
        cout << "Error:" << __FILE__ << ":" << __LINE__ << ":Unknown exception" << endl;
        exit(-1);
    }

    return status;
}
