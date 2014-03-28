#pragma once

#ifdef WIN32
#include <Winsock2.h>
#endif 
#include <stdint.h>
#include "CIPAddress.h"

class CSocket
{
public:
    // Network Type
    enum NetworkType
    {
        NET_TYPE_IPv4 = 2,        // IPv4 network type
        NET_TYPE_IPv6 = 23        // IPv6 network type
    };

    /// Socket Type
    enum SocketType
    {
        SOCKET_TYPE_STREAM   = 1,        // Stream type socket (TCP)
        SOCKET_TYPE_DATAGRAM = 2,        // Datagram type socket (UDP)
        SOCKET_TYPE_RAW      = 3         // Raw type socket
    };

    /// Protocol Type
    enum ProtocolType
    {
        PROTOCOL_TCP = 6,        // TCP protocol type
        PROTOCOL_UDP = 17        // UDP protocol type
    };

    //  Socket Constants
    static const Uint16_t   PORT_ANY;       // Any port constant
    static const Uint32_t   TIMEOUT_NEVER;  // Never timeout constant
    static const CIpAddress ADDRESS_ANY;    // Any network address constant
    static const CIpAddress LOCALHOST;      // Localhost (127.0.0.1) address constant
    
    CSocket();
    CSocket(NetworkType networkType, SocketType socketType, ProtocolType protocol);
    ~CSocket();
    
	// Socket Setup functions
    
    void bind(const CIpAddress& ipAddress = ADDRESS_ANY, Uint16_t port = PORT_ANY);
    void connect(const CIpAddress& ipAddress, Uint16_t port);
    void listen(int maxConns = 0);
    void accept(CSocket& remoteSocket);
    void accept(CSocket& remoteSocket, unsigned int timeout, bool& timedOut);
    void close();
    
	// I/O Connection-Orientated Methods
    bool send(const void* buffer, unsigned int bufferLength);
    bool send(const void* buffer, unsigned int bufferLength, unsigned int timeout, bool& timedOut);

    bool receive(void* data, unsigned int dataLength, unsigned int& recvDataLength);
    bool receive(void* data, unsigned int dataLength, unsigned int& recvDataLength, unsigned int timeout, bool& timedOut);
    
    // I/O Connectionless Methods
    bool send(const void* buffer, const unsigned int bufferLength, const CIpAddress& sendAddress, Uint16_t sendPort);

    bool send(const void* buffer, const unsigned int bufferLength, const CIpAddress& sendAddress, Uint16_t sendPort,
              unsigned int timeout, bool& timedOut);

    bool receive(void* buffer, unsigned int bufferLength, unsigned int& recvDataLength,
                 CIpAddress& recvAddress, Uint16_t& recvPort);

    bool receive(void* buffer, unsigned int bufferLength, unsigned int& recvDataLength,
                 CIpAddress& recvAddress, Uint16_t& recvPort, unsigned int timeout, bool& timedOut);
   
    // Multicast Support Methods
    void joinMulticast(const CIpAddress& multicastAddress, const CIpAddress& interfaceAddress);
    void joinMulticast(const CIpAddress& multicastAddress, const CIpAddress& interfaceAddress, const CIpAddress& sourceAddress);
    void leaveMulticast(const CIpAddress& multicastAddress, const CIpAddress& interfaceAddress);
    void leaveMulticast( const CIpAddress& multicastAddress, const CIpAddress& interfaceAddress, const CIpAddress& sourceAddress);

    // Socket Options Methods
    int getReceiveBufferSize() const;
    void setReceiveBufferSize(int size);
    void setReUseAddress(bool reuse = true);

    bool getMulticastLocalLoopback() const;
    void setMulticastLocalLoopback(bool enabled = true);
    int getMulticastTTL() const;
    void setMulticastTTL(unsigned char ttl);
    void setMutlicastInterface(const CIpAddress& ipAddress);

    int getSockDescriptor() const { return static_cast<int>(this->mSocket); }

protected:

    // Windows specific types
#ifdef WIN32
    typedef SOCKET  BaseSocketType;     ///< Base Socket Type
#else
    typedef int     BaseSocketType;     ///< Base Socket Type
#endif // THEMELIOS_CONFIG_UNIX

    const CTrace        mTrace;         ///< The trace object
    BaseSocketType      mSocket;        ///< The Handle to the socket
    const NetworkType   mNetworkType;   ///< The network type
    const SocketType    mSocketType;    ///< The socket type
    const ProtocolType  mProtocolType;  ///< The protocol type
    int                 mLastError;     ///< The last error reported by ReportLastError()
    CIpAddress          mBindAddress;   ///< The address bound by active socket
    Uint16_t            mBindPort;      ///< The port bound by the active socket

private:

    // Private Constants
    static const unsigned int THOUSAND = 1000;

    // Cross Platform Translation Methods
    static int TranslateNetworkType(NetworkType type);
    static int TranslateSocketType(SocketType type);
    static int TranslateProtocolType(ProtocolType type);
	
    // Hidden
    CSocket(const CSocket&);
    CSocket& operator=(const CSocket&);
};

