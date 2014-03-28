#pragma once

#ifdef WIN32
#include <winsock2.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif


#include <string>


class CIpAddress : public IToString
{
public:

    // name Converter Helpers
    static bool ToSockAddr(struct ::sockaddr_in* sockAddr, const CIpAddress& address, Uint16_t  port);
    static bool FromSockAddr(const struct ::sockaddr_in* sockAddr, CIpAddress& address, Uint16_t& port);
 

    CIpAddress();
    explicit CIpAddress(const std::string&);
    CIpAddress(const CIpAddress& addr);
    virtual ~CIpAddress() {}

    void setAddress(U32 address);
    void setAddress(const std::string& addr);
    U32 getAddress() const;

    virtual std::string toString() const;


    // Multicast Range Checker
    bool isInMulticastRange() const;

    // Assignment
    const CIpAddress& operator=(const CIpAddress& rhs);

    // Boolean operators
    bool operator!=(const CIpAddress& rhs) const;
    bool operator==(const CIpAddress& rhs) const;

private:


    static const char IPADDRESS_SEPERATOR;
    static const Uint8_t MULTICAST_RANGE_MAX;
    static const Uint8_t MULTICAST_RANGE_MIN;

    // Member Variables
    U32 mAddress;   //< Binary IPv4 address
};


inline bool CIpAddress::operator==(const CIpAddress& rhs) const
{
	// Checks to see if two IP addresses match
    return (rhs.mAddress == mAddress);
}

inline bool CIpAddress::operator!=(const CIpAddress& rhs) const
{
	// Checks to see if two IP addresses do not match
    return (mAddress != rhs.mAddress);
}


inline void CIpAddress::setAddress(U32 address)
{
    // Binary address setter
    // raw write;
    mAddress = address;
}

inline U32 CIpAddress::getAddress() const
{
    // Binary address getter
    return mAddress;
}

inline const CIpAddress& CIpAddress::operator=(const CIpAddress& rhs)
{
    // Copy the address value
    mAddress = rhs.mAddress;

    return *this;
}


}
