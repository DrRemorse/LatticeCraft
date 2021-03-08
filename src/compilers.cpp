#include "compilers.hpp"

#include <library/log.hpp>
#include <library/timing/timer.hpp>
#include "columns.hpp"
#include "compiler_scheduler.hpp"
#include "precompiler.hpp"
#include "world.hpp"
#include <mutex>

using namespace library;

namespace cppcraft
{
  static std::mutex mtx_compiler;
	static std::vector<std::unique_ptr<Precomp>> cjobs;
  static std::deque<std::unique_ptr<Precomp>> compiler_queue;

	void CompilerScheduler::add(std::unique_ptr<Precomp> precomp)
	{
		std::lock_guard<std::mutex> lock(mtx_compiler);
		cjobs.push_back(std::move(precomp));
	}

	inline std::vector<std::unique_ptr<Precomp>> CompilerScheduler::get()
	{
    std::lock_guard<std::mutex> lock(mtx_compiler);
    return std::move(cjobs);
	}

	void CompilerScheduler::reset()
	{
    std::lock_guard<std::mutex> lock(mtx_compiler);
    cjobs.clear();
	}

	// initialize compiler data buffers
	void Compilers::init()
	{
		logger << Log::INFO << "* Initializing compilers" << Log::ENDL;
		columns.init();
	}

	// run compilers and try to clear queue, if theres enough time
	void Compilers::run(const int wx, const int wz, const int wdx, const int wdz)
	{
    auto scheduled = CompilerScheduler::get();
    // insert into main queue
    for (auto& precomp : scheduled) compiler_queue.push_back(std::move(precomp));

    library::Timer timer;
    // execute main queue
		while (!compiler_queue.empty())
		{
      auto precomp = std::move(compiler_queue.front());
      compiler_queue.pop_front();

      // TODO: find out why wx, wz doesnt work replacing world.getW[X/Z]
			const int x = precomp->getWX() - wx;
			const int z = precomp->getWZ() - wz;

			if (x >= 0 && z >= 0 && x < sectors.getXZ() && z < sectors.getXZ())
			{
				Column& cv = columns(x, z, wdx, wdz);
				cv.compile(x, 0, z, precomp.get());
			}

      const double time_spent = timer.getTime();
      // we only accept spending 2 millis here
      if (time_spent > 0.002) break;
		}
	} // Compilers::run()

}
