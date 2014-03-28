#pragma once

#ifdef WIN32
#include <Winsock2.h>
#endif
#include "_no_copy.h"
	
	class CSocketLayer : public _no_copy
	{
		// OS Network Layer Representation
		//
		// This is class is needed because under windows we need to make specific calls
		// in order to initialize the underlying socket layer, this (under windows
		// configuration) means that we have to load the Winsock libraries and match
		// them against the ones requested by the user.
		//
		// In a linux configuration this essentially is not needed and will always
		// initialize correctly.
	public:

		// Static Methods
		static bool IsInitialised();

		// Destructor
		~CSocketLayer() {}

	private:

	#ifdef WIN32
		// Constants
		static const unsigned int WINSOCK_VERSION_MAJOR;    ///< Winsock Major Version (2)
		static const unsigned int WINSOCK_VERSION_MINOR;    ///< Winsock Minor Version (2)
	#endif 

		// Variables
		const CTrace    mTrace;                             ///< Trace object for debugging
		bool            mInitialised;                       ///< Initialized flag

	#ifdef WIN32
		WSADATA         mSocketData;                        ///< (Windows only) Winsock data
	#endif

		// Private Constructor
		CSocketLayer();

		// Hidden
		CSocketLayer(const CSocketLayer&);
		CSocketLayer& operator=(const CSocketLayer&);
	};


