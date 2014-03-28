
#ifdef WIN32
#include <Winsock2.h>
#endif

#include "CSocketLayer.h"

#ifdef THEMELIOS_CONFIG_WINDOWS
const unsigned int CSocketLayer::WINSOCK_VERSION_MAJOR = 2;
const unsigned int CSocketLayer::WINSOCK_VERSION_MINOR = 2;
#endif // THEMELIOS_CONFIG_WINDOWS

bool CSocketLayer::IsInitialised()
{
	// Creates a single instance of the CSocketLayer object and checks to see if
	// it is initialised.
    static const CSocketLayer operatingSystemLayer;
    return operatingSystemLayer.mInitialised;
}

CSocketLayer::CSocketLayer()
    : mTrace(__FUNCTION__),
      mInitialised(false)
{
	// This object should only be created once and it will initialize the OS
	// networking support. For some configurations i.e. linux this is not required
	// and the OS network layer will always be initialized.
	//
	// However, under a windows configuration the Winsock DLL needs to be loaded
	// and so this is always called.
#ifdef WIN32
    // Start up win sock
    int result = ::WSAStartup(MAKEWORD(WINSOCK_VERSION_MAJOR,WINSOCK_VERSION_MINOR), &mSocketData);

    // Check the result
    if (result)
    {
        mTrace.write("Failed to initialise OS socket subsystem");
    }
    else
    {
        // Winsock has initialized
        mInitialised = true;
    }
#else
    // the socket layer is always ready to go in linux/unix
    mInitialised = true;
#endif
}
