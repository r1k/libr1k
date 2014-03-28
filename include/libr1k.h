// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the CODELIBRARYRIK_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// CODELIBRARYRIK_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#pragma once

//#ifdef WIN32
//#ifdef LIBR1K_EXPORTS
//#define LIBR1K_API __declspec(dllexport)
//    #else
//#define LIBR1K_API __declspec(dllimport)
//    #endif
//#else
//#define LIBR1K_API
//#endif

//#define LIBR1K_API
// This class is exported from the codeLibraryRiK.dll

namespace libr1k {};

#define LIBR1K_VERSION 3
