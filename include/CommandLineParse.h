#pragma once

#include "libr1k.h"
#include "Types.h"
#include <string>
using namespace std;

namespace libr1k
{
    class CommandLineParse
	{
	public:
		CommandLineParse(int argc, char* argv[]) : arg_num (argc), arg_array(argv){}
		virtual ~CommandLineParse();

		bool GetStringValue	( string key, string &retvalue );
		bool GetStringValue	( int argnum, string &retvalue );

		bool GetNumValue	( string key, int &retvalue );
		bool GetNumValue	( int argnum, int &retvalue );
		bool GetNumValue	( string key, int64_t *retvalue );
		bool GetNumValue	( string key, int *retvalue );
		bool GetNumValue	( int argnum, int *retvalue );

		bool Exists ( string key );

		int NumArguments ( void ) {return arg_num;}

	private:
		int arg_num;
		char **arg_array;
	};
}