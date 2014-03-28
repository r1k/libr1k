#ifndef SOCKETLISTENINGGROUP_H
#define SOCKETLISTENINGGROUP_H

#include <iostream>
#include <string>

#include "net/CIpAddress.h"
#include "net/CSocket.h"

#include <list>

using namespace themelios;

#define INVALID_SOCKET (-1)

class SocketListeningGroup
{
public:

    SocketListeningGroup();
    ~SocketListeningGroup();
    int Select(std::list<CSocket *> &listref, const int timeout=0);

    void addSocket( CSocket *sckt );
    void removeSocket( CSocket *sckt );

private:
    SocketListeningGroup(const SocketListeningGroup&);
    SocketListeningGroup& operator=(const SocketListeningGroup&);

    std::list<CSocket *> listeningGroup;
    int max_fd;
    fd_set listening_set;

};
#endif
