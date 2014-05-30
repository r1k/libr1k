#pragma once

#include "_no_copy.h"
#include <thread>
#include <memory>

namespace libr1k {
	class _runnable : public _no_copy
	{
	public:
		explicit _runnable();
		virtual ~_runnable();

		void start();
		void terminate();
		void join();

	protected:
		virtual int run () = 0;

	private:
		std::unique_ptr<std::thread> thrdHndl;
	};
}
