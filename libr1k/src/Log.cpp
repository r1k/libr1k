//////////////////////////////////////////////////////////////////////
//
// Log.cpp: implementation of the Log class.
//
//////////////////////////////////////////////////////////////////////

#include "Log.h"
#include <fstream>
#include <time.h>
#include <stdarg.h>
#include <stdio.h>
#include <string>
#include <string.h>

using namespace std;
namespace libr1k
{
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

	Log::Log( int initLogLevel) : 
		currentLogLevel(initLogLevel)
	{
		char file_name[1024];
		time_t rawtime;
		
#ifdef WIN32
		time ( &rawtime );

		struct tm timeinfo;

		localtime_s(&timeinfo, &rawtime);
		sprintf_s( file_name, 1024, "%d_%d_%d.log", timeinfo.tm_hour,timeinfo.tm_min, timeinfo.tm_sec );
#else
		time( &rawtime );
		timeinfo = localtime(&rawtime);
		snprintf( file_name, 1024, "%d_%d_%d.log", timeinfo->tm_hour,timeinfo->tm_min, timeinfo->tm_sec );
#endif

		this->open(file_name);
		*this << "Logging Started" << endl;
		logging = true;
	}

	Log::Log(string filename, int initLogLevel) : 
		currentLogLevel(initLogLevel)
	{
		this->open(filename.c_str());
		
		*this  << "Logging Started" << endl;
		logging = true;
	}

	Log::~Log()
	{
		if ( logging ) 
		{
			logging = 0;
			this->flush();
			this->close();
		}
	}

	void Log::AddMessage ( const int errorLevel, const char *message, ... )
	{
		if (errorLevel >= currentLogLevel)
		{
			char	buffer[1024];
			char	timebuf[64];
			va_list ap;
			time_t rawtime;

			if ( !logging ) return;

			memset(buffer, '\0', 1023);
			memset(timebuf, '\0', 63);

			va_start ( ap, message );
#ifdef WIN32
			_vsnprintf_s( buffer, 1024, 1023, message, ap );
#else
			vsnprintf( buffer, 1023, message, ap );
#endif
			va_end ( ap );

			time( &rawtime );
#ifdef WIN32
			struct tm timeinfo;
			localtime_s(&timeinfo, &rawtime);
			sprintf_s ( timebuf, 64, "[%02d:%02d:%02d] > ", timeinfo.tm_hour,timeinfo.tm_min, timeinfo.tm_sec );
#else
			timeinfo = localtime(&rawtime);
			snprintf( timebuf, 64, "[%02d:%02d:%02d] > ", timeinfo->tm_hour,timeinfo->tm_min, timeinfo->tm_sec );
#endif


			*this << timebuf << buffer << endl;

			this->flush();
		}
	}

	void Log::AddMessage ( const int errorLevel, string message )
	{
		this->AddMessage ( errorLevel, "%s", message.c_str() );
	}

	void Log::SetLogLevel ( const int LogLevel )
	{

		this->currentLogLevel = LogLevel;
	}

	void Log::AddTime ( void )
	{
		
		if (this->logging == false)
		{
			return;
		}

		char	timebuf[64];
		time_t rawtime;

		memset(timebuf, '\0', 63);

		time( &rawtime );

#ifdef WIN32
		struct tm timeinfo;
		localtime_s(&timeinfo, &rawtime);
		sprintf_s ( timebuf, 64, "[%d:%d:%d] > ", timeinfo.tm_hour,timeinfo.tm_min, timeinfo.tm_sec );
#else
		timeinfo = localtime( &rawtime);
		snprintf ( timebuf,64, "[%d:%d:%d] > ", timeinfo->tm_hour,timeinfo->tm_min, timeinfo->tm_sec );
#endif
		*this << timebuf;
		return;
	}
}