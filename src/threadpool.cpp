#include "threadpool.hpp"

#include <ThreadPool/thread_pool.hpp>
#include "gameconf.hpp"
#include <memory>
#include <cassert>
#include <atomic>

namespace cppcraft
{
	static std::unique_ptr<ThreadPool> threadpool = nullptr;
	//static int free_jobs = 0;
	static std::atomic_int free_jobs = 0;

	void AsyncPool::init()
	{
		// create dormant thread pool
		free_jobs = config.get("world.threads", 2);
		threadpool.reset(new ThreadPool(free_jobs));
	}

	void AsyncPool::sched(job_t job)
	{
		//__sync_fetch_and_sub(&free_jobs, 1);
		std::atomic_fetch_sub(&free_jobs, 1);
		assert(free_jobs >= 0);

		threadpool->enqueue(job);
	}
	void AsyncPool::release(int count)
	{
		//__sync_fetch_and_add(&free_jobs, count);
		std::atomic_fetch_add(&free_jobs, count);
	}

	bool AsyncPool::available()
	{
		return free_jobs > 0;
	}

  void AsyncPool::stop()
  {
    threadpool = nullptr;
  }
}
