#pragma once
#include "delegate.hpp"
//#include <functional>

namespace cppcraft
{
	struct AsyncPool
	{
    typedef delegate<void()> job_t;

		static void init();

		//! \brief Schedules a job
		static void sched(job_t job);

		//! \brief releases a job
		static void release(int count = 1);

		//! \brief returns true if there are free slots
		static bool available();

    static void stop();
	};
}
