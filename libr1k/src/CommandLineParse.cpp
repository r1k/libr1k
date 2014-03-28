//////////////////////////////////////////////////////////////////////
//
// ommandLineParse.cpp: implementation of the CommandLineParse class.
//
//////////////////////////////////////////////////////////////////////

#include "CommandLineParse.h"
#include <stdlib.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif
namespace libr1k
{
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

	bool CommandLineParse::GetStringValue ( int argnum, string &retvalue )
	{
		if (argnum >= this->arg_num)
		{
			retvalue = "";
			return false;
		}
		retvalue = arg_array[argnum];
		return true;
	}

	bool CommandLineParse::GetStringValue ( string option, string &retvalue )
	{
		retvalue = "";
		for(int i = 0; i < this->arg_num; i++)
		{
			if (option.compare( this->arg_array[i] ) == 0)
			{
				if ( (i + 1) < this->arg_num )
				{
					retvalue = arg_array[i+1];
					return true;
				}
			}
		}
		return false;
	}

	bool CommandLineParse::GetNumValue ( int argnum, int *retvalue )
	{
		if (argnum >= this->arg_num)
		{
			*retvalue = -1;
			return false;
		}
		*retvalue = atoi( arg_array[argnum] );
		return true;
	}

	bool CommandLineParse::GetNumValue ( string key, int *retvalue )
	{
		for(int i = 0; i < this->arg_num; i++)
		{
			if (key.compare( this->arg_array[i] ) == 0)
			{
				if ( (i + 1) < this->arg_num )
				{
					*retvalue = atoi( arg_array[i+1] );
					return true;
				}
			}
		}
		
		*retvalue = 0;
		return false;
	}

	bool CommandLineParse::GetNumValue ( string key, int64_t *retvalue )
	{
		for(int i = 0; i < this->arg_num; i++)
		{
			if (key.compare( this->arg_array[i] ) == 0)
			{
				if ( (i + 1) < this->arg_num )
				{
#ifdef WIN32
					*retvalue = _atoi64( arg_array[i+1] );
#else
					*retvalue = atoll( arg_array[i+1] );
#endif
					return true;
				}
			}
		}
		
		*retvalue = 0;
		return false;
	}

	bool CommandLineParse::GetNumValue ( int argnum, int &retvalue )
	{
		if (argnum >= this->arg_num)
		{
			retvalue = -1;
			return false;
		}
		retvalue = atoi( arg_array[argnum] );
		return true;
	}

	bool CommandLineParse::GetNumValue ( string key, int &retvalue )
	{
		for(int i = 0; i < this->arg_num; i++)
		{
			if (key.compare( this->arg_array[i] ) == 0)
			{
				if ( (i + 1) < this->arg_num )
				{
					retvalue = atoi( arg_array[i+1] );
					return true;
				}
			}
		}
		
		retvalue = 0;
		return false;
	}

	bool CommandLineParse::Exists ( string key )
	{
		for(int i = 0; i < this->arg_num; i++)
		{
			if (key.compare( this->arg_array[i] ) == 0)
			{
				return true;
			}
		}
		return false;
	}

	CommandLineParse::~CommandLineParse()
	{

	}
}