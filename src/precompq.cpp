#include "precompq.hpp"

#include <library/log.hpp>
#include "compiler_scheduler.hpp"
#include "gameconf.hpp"
#include "lighting.hpp"
#include "minimap.hpp"
#include "precomp_thread.hpp"
#include "precompiler.hpp"
#include "sectors.hpp"
#include "threadpool.hpp"
#include <algorithm>
#include <mutex>
#include <cassert>
//#define TIMING

using namespace library;

namespace cppcraft
{
	PrecompQ precompq;

	void PrecompQ::add(Sector& sector)
	{
		assert(sector.generated() == true);
    // dont add if already added
		if (sector.isUpdatingMesh()) return;
    // dont add the edge to queue as its going to get auto-updated by seamless
    if (sector.getX() < 2 || sector.getZ() < 2 || sector.getX() >= sectors.getXZ()-2 || sector.getZ() >= sectors.getXZ()-2)
          return;

    // Unfortunately we can't do a full check to see if the sector
    // is completely ready for this queue.
    // On the edge there are sectors that will "never"
    // get their objects done, and so will never let their neighbors
    // qualify for this queue. This happens rarely, but it *DOES* happen.

    // We can at least check if the neighbors are generated:
    bool at_least_generated = sectors.onNxN(sector, 1, // 3x3
		[] (Sector& sect) { return sect.generated(); });
    if (at_least_generated == false) return;

		sector.meshgen = true;
		queue.push_back(&sector);
	}

	void PrecompQ::run()
	{
		if (!AsyncPool::available()) return;

    //std::sort(queue.begin(), queue.end(), GenerationOrder);

		// since we are the only ones that can take stuff
		// from the available queue, we should be good to just
		// check if there are any available, and thats it
		for (auto it = queue.begin(); it != queue.end(); ++it)
		{
			Sector* sector = *it;
      assert(sector != nullptr);

      // -= try to clear out old shite =-
  		// NOTE: there will always be sectors that cannot be finished
  		// due to objects begin scheduled and not enough room to build them
      if (sector->generated() == false ||
          sector->isUpdatingMesh() == false ||
          sector->getX() < 2 || sector->getZ() < 2 ||
          sector->getX() >= sectors.getXZ()-2 ||
          sector->getZ() >= sectors.getXZ()-2)
      {
        sector->meshgen = false;
        it = queue.erase(it);
        continue;
      }

			// make sure we have proper light
			bool is_ready = sectors.onNxN(*sector, 1, // 3x3
			[] (Sector& sect)
			{
				if (sect.atmospherics == false)
				{
					if (sect.isReadyForAtmos() == false) { return false; }
					#ifdef TIMING
						Timer timer;
					#endif
					Lighting::atmosphericFlood(sect);
					#ifdef TIMING
						printf("Time spent in that goddamn atm flood: %f\n",
							timer.getTime());
					#endif
          // flood takes some time, so lets not do more in one go
					return false;
				}
        return sect.objects == 0;
			});
      // if not ready yet,
			if (is_ready == false) {
        // try next in queue
        queue.splice(queue.end(), queue, it);
        continue;
      }

			// check again that there are available slots
			if (!AsyncPool::available()) break;

			// finally, we can start the job
			startJob(*sector);
      it = queue.erase(it);
		}
	}

	void PrecompQ::startJob(Sector& sector)
	{
		// create new Precomp
		//printf("Precompiler scheduling (%d, %d) size: %lu\n",
		//	sector->getX(), sector->getZ(), sizeof(Precomp));
		sector.meshgen = false;
    // sneak in a minimap update if scheduled
    if (sector.has_flag(Sector::MINIMAP)) {
      sector.rem_genflag(Sector::MINIMAP);
      minimap.addSector(sector);
      minimap.setUpdated();
    }

    auto precomp = std::make_unique<Precomp> (sector);

    // go go go!
    AsyncPool::sched(
      AsyncPool::job_t::make_packed(
      [pc = std::move(precomp)] () mutable
      {
        PrecompThread wset;
  			// first stage: mesh generation
  			wset.precompile(*pc);
  			// second stage: AO
  			wset.ambientOcclusion(*pc);

  			/////////////////////////
  			CompilerScheduler::add(std::move(pc));
  			/////////////////////////

  			AsyncPool::release();
      }));
	}

  bool PrecompQ::contains(Sector& sector) const
  {
    for (const auto* s : queue) {
      if (s->getX() == sector.getX() && s->getZ() == sector.getZ()) return true;
    }
    return false;
  }
}
