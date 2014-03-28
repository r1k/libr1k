#include "SocketListeningGroup.h"

#include <iostream>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

using namespace std;

SocketListeningGroup::SocketListeningGroup():
    max_fd(0)
{
    FD_ZERO(&(this->listening_set));
}

SocketListeningGroup::~SocketListeningGroup()
{
    FD_ZERO(&(this->listening_set));
}

void SocketListeningGroup::addSocket( CSocket *sckt )
{
    this->listeningGroup.push_back(sckt);
    FD_SET(sckt->getSockDescriptor(), &(this->listening_set));
    if( sckt->getSockDescriptor() > this->max_fd)
    {
        this->max_fd = sckt->getSockDescriptor();
    }
}

void SocketListeningGroup::removeSocket( CSocket *sckt )
{
    for(list<CSocket *>::iterator iter = this->listeningGroup.begin(); iter != this->listeningGroup.end(); ++iter)
    {
        if (sckt->getSockDescriptor() == (*iter)->getSockDescriptor())
        {
            FD_CLR(sckt->getSockDescriptor(), &(this->listening_set));
            this->listeningGroup.erase(iter);
            break;
        }
    }

    if (this->max_fd == sckt->getSockDescriptor())
    {
        this->max_fd = 0;
        for(list<CSocket *>::iterator iter = this->listeningGroup.begin(); iter != this->listeningGroup.end(); ++iter)
        {
            if ((*iter)->getSockDescriptor() > this->max_fd)
                this->max_fd = (*iter)->getSockDescriptor();
        }
    }
}

int SocketListeningGroup::Select(list<CSocket *> &listref, int timeout)
{
    struct timeval tv;
    tv.tv_sec = timeout;
    tv.tv_usec = 0;

    fd_set readset;
    FD_ZERO(&readset);
    memcpy(&readset, &(this->listening_set), sizeof(fd_set));

    const int retval = select(this->max_fd+1, &readset, NULL, NULL, &tv);
    if (retval < 0)
    {
        perror("Error in select");
        return -1;
    }
    else if (retval == 0)
    {
        return 0;
    }

    for(int i = 0; i <= this->max_fd; i++)
    {
        if (FD_ISSET(i, &readset))
        {
            // Work out which AccessRawSocket is set and pass all of them back in the list
            for(list<CSocket *>::iterator iter = listeningGroup.begin(); iter != listeningGroup.end(); ++iter)
            {
                if (i == (*iter)->getSockDescriptor())
                {
                    listref.push_back(*iter);
                    break;
                }
            }
        }
    }


    return retval;
}
