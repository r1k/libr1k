//////////////////////////////////////////////////////////////////////
//
// Log.h: interface for the Log class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "libr1k.h"

#include <string>
#include <fstream>

using namespace std;


namespace libr1k
{
    class Log : public ofstream
	{
	public:
		enum {MIN_LOG_LEVEL = 0, DEBUG_LOG_LEVEL = 3, DEFAULT_LOG_LEVEL = 5, MAX_LOG_LEVEL = 10};

		Log(int initLogLevel = 10);
		Log(string filename, int initLogLevel = 10);

		virtual void AddMessage ( const int errorLevel, const char *message, ... );
		virtual void AddMessage ( const int errorLevel, string message );

		void AddTime ( void );
		
		void SetLogLevel ( const int LogLevel );
		
		virtual ~Log();

	protected:

		bool logging;

	private:
		
		int currentLogLevel;
		string filename;

		Log(const Log&) = delete;
		Log& operator=(const Log&) = delete;
	};
}
