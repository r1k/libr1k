
#include <stdexcept>
#include <sstream>

#ifdef WIN32
#include <winsock2.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif 

#include "net/CIpAddress.h"

static const CTrace gTrace("CIpAddress");

const char CIpAddress::IPADDRESS_SEPERATOR('.');
const U8   CIpAddress::MULTICAST_RANGE_MAX(239);
const U8   CIpAddress::MULTICAST_RANGE_MIN(224);

bool CIpAddress::ToSockAddr(struct ::sockaddr_in* sockAddr, const CIpAddress& address, Uint16_t port)
{
    // Convert CIpAddress + Port => sockaddr_in
    bool success(false);

    // Check the pointer
    if( sockAddr )
    {
        // Setup the structure
        sockAddr->sin_family      = AF_INET;
        sockAddr->sin_addr.s_addr = address.mAddress;
        sockAddr->sin_port        = htons( port );

        // Assume successful
        success = true;
    }

    return success;
}


bool CIpAddress::FromSockAddr(const struct ::sockaddr_in* sockAddr, CIpAddress& address, Uint16_t& port)
{
    // Convert sockaddr_in => CIpAddress + Port
    bool success(false);

    // Check for null pointer and correct address family
    if( sockAddr && (AF_INET == sockAddr->sin_family) )
    {
        // Extract the data
        address.mAddress = sockAddr->sin_addr.s_addr;
        port             = ntohs(sockAddr->sin_port);

        // Signal success
        success = true;
    }

    return success;
}


CIpAddress::CIpAddress()
    : mAddress(0)
{
}


CIpAddress::CIpAddress(const std::string& addr)
    : mAddress(0)
{
    // Create IP Address from string
    // Set the address - no checking possible to see if this has failed!
    setAddress(addr);
}

CIpAddress::CIpAddress(const CIpAddress& addr)
    : mAddress(addr.mAddress)
{
}


void CIpAddress::setAddress(const std::string& addr)
{
    // Convert the address
    const U32 address = ::inet_addr(addr.c_str());

    // Check for error
    if (INADDR_NONE != mAddress)
    {
        // Update the parameter
        mAddress = address;
    }
    else
    {
        throw Exception("address: " << addr << " is not a valid network address");
    }
}

std::string CIpAddress::toString() const
{
    std::ostringstream oss;
    const U8* binAddr = reinterpret_cast<const U8*>( &mAddress );

    // Build the String
    oss << static_cast<unsigned int>(binAddr[0])
        << IPADDRESS_SEPERATOR
        << static_cast<unsigned int>(binAddr[1])
        << IPADDRESS_SEPERATOR
        << static_cast<unsigned int>(binAddr[2])
        << IPADDRESS_SEPERATOR
        << static_cast<unsigned int>(binAddr[3]);

    return oss.str();
}


bool CIpAddress::isInMulticastRange() const
{
    bool isMulticast(false);

    // get the first byte
    const U8 firstByte = (reinterpret_cast<const U8*>(&mAddress))[0];

    if ( (firstByte >= MULTICAST_RANGE_MIN) &&
         (firstByte <= MULTICAST_RANGE_MAX) )
    {
        // it is a multicast
        isMulticast = true;
    }

    return isMulticast;
}

