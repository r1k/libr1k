#include "_runnable.h"

using namespace std;
namespace libr1k {

	_runnable::~_runnable()
	{
	}

	void _runnable::terminate()
	{
		thread *t = thrdHndl.release();
		delete t;
	}

	void _runnable::join()
	{
		if (thrdHndl != nullptr && thrdHndl->joinable())
		{
			thrdHndl->join();
		}
	}

	void _runnable::start()
	{
		thrdHndl = unique_ptr<thread>( new thread(&_runnable::run, this));
	}

}
