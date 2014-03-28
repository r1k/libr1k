
#include <stdexcept>
#include <sstream>
#include <cstring> //memset

//------------------------------------------------------------------------------
// Linux Configuration
//------------------------------------------------------------------------------
#ifndef WIN32
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <unistd.h>

#define INVALID_SOCKET (-1)
#define THEMELIOS_FD_SET(fd, set)   FD_SET(fd, set)

typedef unsigned int ThemeliosSockOptLen;

#endif 

//------------------------------------------------------------------------------
// Windows Includes
//------------------------------------------------------------------------------
#ifdef WIN32
#include <Winsock2.h>
#include <Ws2tcpip.h>

#define THEMELIOS_FD_SET(fd, set) \
        /* warning C4127: conditional expression is constant */ \
        __pragma( warning( suppress : 4127 )) \
        FD_SET(fd, set)

typedef int ThemeliosSockOptLen;

#endif


#include "common/Macros.h"
#include "net/CSocket.h"
#include "net/CSocketLayer.h"

const Uint16_t   CSocket::PORT_ANY      (0);
const U32        CSocket::TIMEOUT_NEVER (0xffffffff);
const CIpAddress CSocket::ADDRESS_ANY   ("0.0.0.0");
const CIpAddress CSocket::LOCALHOST     ("127.0.0.1");


CSocket::CSocket()
    : mTrace(__FUNCTION__)
    , mSocket(INVALID_SOCKET)
    , mNetworkType(NET_TYPE_IPv4) // dummy
    , mSocketType(SOCKET_TYPE_STREAM) // dummy
    , mProtocolType(PROTOCOL_TCP) // dummy
    , mLastError(0)
    , mBindAddress(ADDRESS_ANY)
    , mBindPort(PORT_ANY)
{
}


CSocket::CSocket(NetworkType networkType, SocketType socketType, ProtocolType protocol)

    : mTrace(__FUNCTION__)
    , mSocket(INVALID_SOCKET)
    , mNetworkType(networkType)
    , mSocketType(socketType)
    , mProtocolType(protocol)
    , mLastError(0)
    , mBindAddress(ADDRESS_ANY)
    , mBindPort(PORT_ANY)
{
	//
	// param networkType The network type of the new socket
	// param socketType The socket type of the new socket
	// param protocol The protocol type of the new socket
	//
	// Constructor for the socket object
	//
	// Basic Constructor for the socket object. This meythod sets up the socket with
	// basic information on the intended network type (IPv4 or IPv6), the basic
	// socket type (essentially connection orientated - TCP or connectionless - UDP)
	// and a protocol type/number
	//
	// throw std::runtime_error If the winsock layer cannot be initialized (windows only)
	// throw std::runtime_error If the socket can not be created
	//------------------------------------------------------------------------------
	
    // Check that the OS is capable of creating sockets
    if (!CSocketLayer::IsInitialized())
    {
        // could not initialize system internals
        THEMELIOS_TRACE_AND_THROW(mTrace, "Unable to initialize OS for networking support");
    }

    // Create the socket
    mSocket = ::socket(
        TranslateNetworkType(mNetworkType),
        TranslateSocketType(mSocketType),
        TranslateProtocolType(mProtocolType)
    );

    // Check the socket
    if (INVALID_SOCKET == mSocket)
    {
        THEMELIOS_TRACE_AND_THROW(mTrace, "Failed to create socket");
    }
}

CSocket::~CSocket()
{
	// Destructor
	//
	// Destructor for the socket object. If the socket is still open this destructor
	// will attempt to close the socket.
    // Check that there is a socket to close down
    if( INVALID_SOCKET != mSocket )
    {
#ifdef WIN32
        // Close the socket
        if (::closesocket(mSocket))
        {
            THEMELIOS_TRACE(mTrace, "Failed to close socket");
        }
#else
		if (::close(mSocket) < 0)
        {
            THEMELIOS_TRACE(mTrace, "Failed to close socket");
        }
#endif
        // Clear the handle there really isnt much we can do now.
        mSocket = INVALID_SOCKET;
    }
}

void CSocket::bind(const CIpAddress& ipAddress, Uint16_t port)
{
	// param ipAddress The ip address to bind the socket to
	// param port The port to bind to.
	//
	//  Bind Socket Method
	//
	// This function binds the socket to an ip address and port of an interface
	// on the host system. If you do care about the values here you can bind to
	// ADDRESS_ANY and PORT_ANY.
	//
	// If you intend to send or receive a multicast it is important that you
	// bind to the correct interface.
	//
	// throw std::runtime_error If the bind operation was unsuccessful.
	
    // Create socket address
    struct sockaddr_in bindAddress;
    CIpAddress::ToSockAddr(&bindAddress, ipAddress, port);

    // Attempt to bind
    int result = ::bind(mSocket, reinterpret_cast<sockaddr *>( &bindAddress ), sizeof(bindAddress));

    if (result)
	{
		throw Exception;
	}
	else
    {
        // Update bind information
        mBindAddress = ipAddress;
        mBindPort    = port;
    }
}

void CSocket::connect(const CIpAddress& ipAddress, Uint16_t port)
{
	// param ipAddress The IP address of the remote host
	// param port The port of the remote host
	//
	// Connect to a remote host
	//
	// This function is used when establishing a connection orientated link to a
	// remote host.
	//

    // Create socket address
    struct sockaddr_in connectAddress;
    CIpAddress::ToSockAddr(&connectAddress, ipAddress, port);

    // Attempt to connect
    int result = ::connect(mSocket, reinterpret_cast<sockaddr *>( &connectAddress ), sizeof(connectAddress));
}


void CSocket::listen(int maxConns)
{
    // default case maxConns = 0, meaning that the
    if (0 == maxConns)
    {
        // use the max number of connection supported
        maxConns = SOMAXCONN;
    }
    // Listen on the socket
    ::listen(mSocket, maxConns)
}

void CSocket::accept(CSocket& remoteSocket)
{
    // Assert remote_socket is invalid
    if (INVALID_SOCKET == remoteSocket.mSocket)
    {
        // Attempt to accept a connection
        remoteSocket.mSocket = ::accept(mSocket, NULL, NULL);

        // Check the socket
        if (INVALID_SOCKET == remoteSocket.mSocket)
        {
            throw Exception("Failed to accept incoming connection");
        }
    }
}

void CSocket::accept(CSocket& remoteSocket, unsigned int timeout, bool& timedOut)
{
    fd_set  fdSet;
    timeval timeoutInterval;

    // Clear the FD set
    FD_ZERO(&fdSet);

    // Add the socket to the FD set
    FD_SET(mSocket, &fdSet);

    // Create the timeout
    timeoutInterval.tv_sec  = (timeout - (timeout % 1000)) / 1000;
    timeoutInterval.tv_usec = (timeout % 1000) * 1000;

    // windows does not use the first parameter and it is ignored. Linux does
    // however, and it must the the highest number fd +1.
    const int result = ::select(ONE_MORE(mSocket), &fdSet, NULL, NULL, &timeoutInterval);

    // Clear the timeout flag
    timedOut = false;

    if (result > 0)
    {
        // do the actual call
        return accept(remoteSocket);
    }
    else if (result)
    {
        
    }
    else
    {
        timedOut = true;
    }
}

void CSocket::close()
{
#ifndef WIN32
    const int option = SHUT_RDWR;
#else
    const int option = SD_BOTH;
#endif

    // Shutdown the socket
    if (::shutdown(mSocket, option))
    {
        throw Exception("Failed to shutdown socket");
    }
}


bool CSocket::send(const void* buffer, unsigned int bufferLength)
{
    bool success(false);

    // send the packet
    const int bytesSent = ::send(mSocket, reinterpret_cast<const char*>(buffer), bufferLength, 0);

    if (bytesSent < 0)
    {
    }
    else
    {
        success = true;
    }

    return success;
}


bool CSocket::send(const void* buffer, unsigned int bufferLength, unsigned int timeout, bool& timedOut)
{

}


bool CSocket::receive(void* data, unsigned int dataLength, unsigned int& recvDataLength)
{
    bool success(false);

    // do the recv on the socket
    const int bytesRead = ::recv(mSocket, reinterpret_cast<char*>(data), dataLength, 0);

    if (bytesRead < 0)
    {
        // mark invalid read
        recvDataLength = 0;
    }
    else
    {
        // the good case
        recvDataLength = bytesRead;

        // signal success
        success = true;
    }

    return success;
}


bool CSocket::receive(void* data, unsigned int dataLength, unsigned int& recvDataLength, unsigned int timeout, bool& timedOut)
{
    fd_set  fdSet;
    timeval timeoutInterval;

    // Clear the FD set
    FD_ZERO(&fdSet);

    // Add the socket to the FD set
    FD_SET(mSocket, &fdSet);

    // Create the timeout
    timeoutInterval.tv_sec  = (timeout - (timeout % 1000)) / 1000;
    timeoutInterval.tv_usec = (timeout % 1000) * 1000;

    // windows does not use the first parameter and it is ignored. Linux does
    // however, and it must the the highest number fd +1.
    const int result = ::select(ONE_MORE(mSocket), &fdSet, NULL, NULL, &timeoutInterval);

    // Clear the timeout flag
    timedOut = false;

    if (result > 0)
    {
        // do the actual call
        return receive(data,dataLength,recvDataLength);
    }
    else if (result)
    {
        return false;
    }
    else
    {
        timedOut = true;
        return false;
    }
}


bool CSocket::send(const void* buffer, const unsigned int bufferLength, const CIpAddress& sendAddress, Uint16_t sendPort)
{
    bool success(false);

    struct sockaddr_in sendAddr;
    CIpAddress::ToSockAddr(&sendAddr, sendAddress, sendPort);

    // Send the data
    int bytes_sent = ::sendto(mSocket, reinterpret_cast<const char*>( buffer ), bufferLength, 0, reinterpret_cast<sockaddr*>( &sendAddr ), sizeof(sendAddr));

    if (bytes_sent < 0)
    {
        
    }
    else
    {
        // Send successful
        success = true;
    }

    return success;
}


bool CSocket::send(const void* buffer, const unsigned int bufferLength, const CIpAddress& sendAddress, Uint16_t sendPort, unsigned int timeout, bool& timedOut)
{
    fd_set  fdSet;
    timeval timeoutInterval;

    // Clear the FD set
    FD_ZERO(&fdSet);

    // Add the socket to the FD set
    FD_SET(mSocket, &fdSet);

    // Create the timeout
    timeoutInterval.tv_sec  = (timeout - (timeout % THOUSAND)) / THOUSAND;
    timeoutInterval.tv_usec = (timeout % THOUSAND) * THOUSAND;

    // windows does not use the first parameter and it is ignored. Linux does
    // however, and it must the the highest number fd +1.
    int result = ::select(ONE_MORE(mSocket), NULL, &fdSet, NULL, &timeoutInterval);

    // Clear the timeout flag
    timedOut = false;

    if( result > 0 )
    {
        return send(buffer,bufferLength,sendAddress,sendPort);
    }
    else if (result)
    {
        
        return false;
    }
    else
    {
        timedOut = true;
        return false;
    }
}


bool CSocket::receive(void* buffer, unsigned int bufferLength, unsigned int& recvDataLength, CIpAddress& recvAddress, Uint16_t& recvPort)
{
    bool success(false);

    sockaddr_in recvAddr;

#ifndef WIN32
    unsigned int szRecvAddr = sizeof(sockaddr_in);
#else
    int szRecvAddr = sizeof(sockaddr_in);
#endif

    // Receive packet from the socket
    int bytesRead = ::recvfrom(mSocket, reinterpret_cast<char*>(buffer), bufferLength, 0, reinterpret_cast<sockaddr*>(&recvAddr), &szRecvAddr);

    // Convert the sockaddr to IPaddress and port
    CIpAddress::FromSockAddr(&recvAddr, recvAddress, recvPort);

    // Check for errors etc.
    if (bytesRead < 0)
    {
        // Something bad has happened!
        
    }
    else if (bytesRead == 0)
    {
        // We have closed gracefully!
        recvDataLength = 0;

        // Signal success
        success = true;
    }
    else
    {
        // Normal (hopefully!) - We have received a datagram
        recvDataLength = bytesRead;

        // Signal success
        success = true;
    }

    return success;
}

bool CSocket::receive(void* buffer, unsigned int bufferLength, unsigned int& recvDataLength, CIpAddress& recvAddress, Uint16_t& recvPort, unsigned int timeout, bool& timedOut)
{
    fd_set  fdSet;
    timeval timeoutInterval;

    // Clear the FD set
    FD_ZERO(&fdSet);

    // Add the socket to the FD set
    FD_SET(mSocket, &fdSet);

    // Create the timeout
    timeoutInterval.tv_sec  = (timeout - (timeout % 1000)) / 1000;
    timeoutInterval.tv_usec = (timeout % 1000) * 1000;

    // windows does not use the first parameter and it is ignored. Linux does
    // however, and it must the the highest number fd +1.
    int result = ::select(ONE_MORE(mSocket), &fdSet, NULL, NULL, &timeoutInterval);

    // Clear the timeout flag
    timedOut = false;

    if (result > 0)
    {
        return receive(buffer,bufferLength,recvDataLength,recvAddress,recvPort);
    }
    else
    {
        timedOut = true;
        return false;
    }
}

void CSocket::joinMulticast(const CIpAddress& multicastAddress, const CIpAddress& interfaceAddress)

{
    //
	// @todo Need to add a check under linux to make sure that the rp_filter is
	//       set to 2:
	//       /proc/sys/net/ipv4/conf/*/rp_filter
	//       It is possible if this is not set then the multicast will fail to be
	//       received.
	
    // Multicast request structure (source specific)
    struct ip_mreq multicastRequest;
    ::memset(&multicastRequest,0, sizeof(multicastRequest));

    // Fill out interface and multicast address
    multicastRequest.imr_interface.s_addr  = interfaceAddress.getAddress();
    multicastRequest.imr_multiaddr.s_addr  = multicastAddress.getAddress();

    // Try and join the multicast
    const int result = ::setsockopt(mSocket, IPPROTO_IP, IP_ADD_MEMBERSHIP, reinterpret_cast<const char*>( &multicastRequest ), sizeof(multicastRequest));

    // Check the result
    if (result)
    {
        throw Exception("Failed to join multicast group " << multicastAddress << " on " << interfaceAddress);
    }
}


void CSocket::joinMulticast(const CIpAddress& multicastAddress, const CIpAddress& interfaceAddress, const CIpAddress& sourceAddress)
{
    throw Exception();
}

void CSocket::leaveMulticast(const CIpAddress& multicastAddress, const CIpAddress& interfaceAddress)
{
    // Multicast request structure (source specific)
    struct ip_mreq multicastRequest;
    ::memset(&multicastRequest,0, sizeof(multicastRequest));

    // Fill out interface and multicast address
    multicastRequest.imr_interface.s_addr  = interfaceAddress.getAddress();
    multicastRequest.imr_multiaddr.s_addr  = multicastAddress.getAddress();

    // Try and join the multicast
    const int result = ::setsockopt(mSocket, IPPROTO_IP, IP_DROP_MEMBERSHIP, reinterpret_cast<const char*>(&multicastRequest), sizeof(multicastRequest));

    // Check the result
    if (result)
    {
        throw Exception("Failed to leave multicast group " << multicastAddress << " on " << interfaceAddress);
    }
}

void CSocket::leaveMulticast(const CIpAddress& multicastAddress, const CIpAddress& interfaceAddress, const CIpAddress& sourceAddress)
{
    throw Exception("Method not implemented");
}


int CSocket::getReceiveBufferSize() const
{
    ThemeliosSockOptLen fieldLength(sizeof(int));
    unsigned int size(0);

#ifndef WIN32

    const int result = ::getsockopt(mSocket, SOL_SOCKET, SO_RCVBUF, &size, &fieldLength);

#else

    const int result = ::getsockopt(mSocket, SOL_SOCKET, SO_RCVBUF, reinterpret_cast<char*>(&size), &fieldLength);

#endif

    // check for error
    if (result)
    {
        throw Exception(mTrace, "Unable to get the socket recv buffer size");
    }

    return size;
}

void CSocket::setReceiveBufferSize(int size)
{
    const int result = ::setsockopt(mSocket, SOL_SOCKET, SO_RCVBUF,
#ifdef WIN32
        reinterpret_cast<char*>(&size),
#else
        &size,
#endif
        sizeof(int));

    if (0 != result)
    {
        throw Exception("Unable to set receive buffer size to: " << size);
    }
}


void CSocket::setReUseAddress(bool reuse)
{
    int opt = (reuse) ? 1 : 0;

    const int result = ::setsockopt(mSocket, SOL_SOCKET, SO_REUSEADDR,
#ifdef THEMELIOS_CONFIG_WINDOWS
        reinterpret_cast<char*>(&opt),
#else
        &opt,
#endif
        sizeof(int));

    if (0 != result)
    {
        throw Exception(mTrace, "Unable to " << ((reuse) ? "enable" : "disable") << " reuse of address on socket");
    }
}


bool CSocket::getMulticastLocalLoopback() const
{
    unsigned char loop(0);
    ThemeliosSockOptLen size = sizeof(loop);

#ifndef WIN32

    const int result = ::getsockopt(mSocket, IPPROTO_IP, IP_MULTICAST_LOOP, &loop, &size);

#else

    const int result = ::getsockopt(mSocket, IPPROTO_IP, IP_MULTICAST_LOOP, reinterpret_cast<char*>(&loop), &size);

#endif

    if (result)
    {
        throw Exception("failed to set multicast local loopback");
    }

    return ((loop) ? true : false);
}

void CSocket::setMulticastLocalLoopback(bool enabled)
{
    unsigned char loop = (enabled) ? 1 : 0;

    const int result = ::setsockopt(mSocket, IPPROTO_IP, IP_MULTICAST_LOOP,
#ifdef WIN32
        reinterpret_cast<char*>(&loop),
#else
        &loop,
#endif
        sizeof(loop));

    if (result)
    {
        throw Exception(mTrace, "failed to get multicast local loopback");
    }
}

int CSocket::getMulticastTTL() const
{
    throw Exception("getMulticastTTL not implemented");
}

void CSocket::setMulticastTTL(unsigned char ttl)
{
    const int result = ::setsockopt(mSocket, IPPROTO_IP, IP_MULTICAST_TTL,
#ifdef WIN32
        reinterpret_cast<char*>(&ttl),
#else
        &ttl,
#endif
        sizeof(ttl));

    if (result)
    {
        throw Exception("failed to set mutlicast ttl to "<< ttl);
    }
}

void CSocket::setMutlicastInterface(const CIpAddress& ipAddress)
{
    struct in_addr interface_addr;
    interface_addr.s_addr = ipAddress.getAddress();

    const int result = ::setsockopt(mSocket, IPPROTO_IP, IP_MULTICAST_IF, #
#ifdef WIN32
        reinterpret_cast<char*>(&interface_addr),
#else
        &interface_addr,
#endif
        sizeof(interface_addr));

    if (result)
    {
        throw Exception(mTrace, "failed to set multicast interface address to " << ipAddress);
    }
}

int CSocket::TranslateNetworkType(NetworkType type)
{
#ifndef WIN32

    switch (type)
    {
        case NET_TYPE_IPv4:     return AF_INET;
        case NET_TYPE_IPv6:     return AF_INET6;
        default:                return AF_INET;
    }

#else
    return static_cast<int>(type);
#endif
}

int CSocket::TranslateSocketType(SocketType type)
{
#ifndef WIN32

    switch (type)
    {
        case SOCKET_TYPE_STREAM:    return SOCK_STREAM;
        case SOCKET_TYPE_DATAGRAM:  return SOCK_DGRAM;
        case SOCKET_TYPE_RAW:       return SOCK_RAW;
        default:                    return SOCK_DGRAM;
    }

#else
    return static_cast<int>(type);

#endif
}

int CSocket::TranslateProtocolType(ProtocolType type)
{
    return static_cast<int>(type);
}

