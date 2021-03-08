#include "generator.hpp"

#include <library/config.hpp>
#include <library/log.hpp>
#include "chunks.hpp"
#include "minimap.hpp"
#include "player.hpp"
#include "lighting.hpp"
#include "sectors.hpp"
#include "threadpool.hpp"
#include "world.hpp"

// load compressor last
#include "compressor.hpp"
#include "generator/terragen.hpp"
#include "generator/objectq.hpp"
#include <algorithm>
#include <cstring>
#include <vector>

#include <library/timing/rolling_avg.hpp>
//#define TIMING

using namespace library;

namespace cppcraft
{
	unsigned int g_fres[Chunks::CHUNK_SIZE][Chunks::CHUNK_SIZE];
	unsigned int g_compres[Chunks::CHUNK_SIZE][Chunks::CHUNK_SIZE];

	std::deque<Sector*> Generator::queue;
	// multi-threaded shits, mutex for the finished queue
	// and the list of containers of finished jobs (gendata_t)
	static std::mutex mtx_genq;
	static std::vector<std::unique_ptr<terragen::gendata_t>> finished;

	void Generator::init()
	{
    logger << Log::INFO << "* Initializing terrain generator" << Log::ENDL;
		/// our esteemed generator ///
		terragen::Generator::init();
    terragen::ObjectQueue::init();
		// schedule everything to be generated
		for (int x = 0; x < sectors.getXZ(); x++)
		for (int z = 0; z < sectors.getXZ(); z++)
		{
			Generator::add(sectors(x, z));
		}
    // pre-sort generator queue
		std::sort(queue.begin(), queue.end(), GenerationOrder);
	}

	void Generator::run()
	{
#ifdef TIMING
    static library::RollingAvg timer;
    timer.begin();
#endif
		// sort by distance from center (radius)
    // the queue should on average be mostly sorted
		std::sort(queue.begin(), queue.end(), GenerationOrder);

		// queue from the top of the vector
		while (!queue.empty() && AsyncPool::available())
		{
			// because its a vector internally, we pop from the back
			Sector* sect = queue.front();
			queue.pop_front();
      // if the sector is no longer needed to be generated, go to next
      if (sect->generating() == false) continue;

			//printf("Generating sector (%d, %d)\n",
			//	sect->getX(), sect->getZ());

      delegate<void()> func =
      [wx = sect->getWX(), wz = sect->getWZ()]
      {
        // create immutable job data
  			auto gdata = std::make_unique<terragen::gendata_t> (wx, wz);
    		terragen::Generator::run(gdata.get());

    		// re-add the data back to the finished queue
    		mtx_genq.lock();
    		finished.push_back(std::move(gdata));
    		mtx_genq.unlock();
    	};
			// execute the generator job in background, delete job after its done
			AsyncPool::sched(std::move(func));
		}

    // retrieve from queue
    mtx_genq.lock();
    auto finvec = std::move(finished);
    mtx_genq.unlock();
    bool minimap_updated = false;

		// finished generator jobs
		for (auto& gdata : finvec)
		{
			const int x = gdata->wx - world.getWX();
			const int z = gdata->wz - world.getWZ();
			// check that the generated data is still
			// within our grid:
			if (x >= 0 && x < sectors.getXZ() &&
					z >= 0 && z < sectors.getXZ())
			{
				// resultant sector
				Sector& dest = sectors(x, z);
				//printf("Sector was generated: (%d, %d)\n", dest.getX(), dest.getZ());

				// swap out generated blocks
				dest.assignBlocks(gdata->unassignBlocks());
				// and, swap out the flatland data
				dest.flat().assign(gdata->flatl.unassign());
				// toggle sector generated flag, as well as removing generating flag
				dest.meshgen      = false; // make sure its added to meshgen
				dest.gen_flags    = Sector::GENERATED;
				dest.objects      = gdata->get_objects().size();
				dest.atmospherics = false;

				// add all the objects from this sector to object queue
				if (dest.objects > 0)
			     terragen::ObjectQueue::add(gdata->get_objects());
        else
        {
          // each time a sector gets generated, we can check if neighbors
          // are ready to be added to precompq
          sectors.onNxN(dest, 1, // 3x3
              [] (Sector& sect) -> bool {
                if (sect.isReadyForAtmos() && sect.isUpdatingMesh() == false)
                    sect.updateAllMeshes();
                return true;
              });
        }
				// add it to the minimap right now
        minimap.addSector(dest);
        minimap_updated = true;
			}
			else
			{
			#ifdef DEBUG
				printf("INVALID sector was generated: (%d, %d)\n", x, z);
			#endif
			}
      if (minimap_updated) minimap.setUpdated();
		}

    // allow more jobs in the async pool
    AsyncPool::release(finvec.size());

#ifdef TIMING
    timer.measure();
    printf("Generator took %f seconds (high: %f)\n",
          timer.getTime(), timer.highest());
#endif
	}

	void Generator::loadSector(Sector& sector, std::ifstream& file, unsigned int PL)
	{
		// start by setting sector as not having been generated yet
		//sector.generated = false;
		sector.atmospherics = false;

		// load sector, which can return a not-generated sector
		chunks.loadSector(sector, file, PL);

		// if the sector still not generated, we need to invoke the terrain generator
		if (sector.generated() == false)
		{
			/// invoke terrain generator here ///
		}
	}

	/**
	 * Generator will truncate sector(x, z) down to the nearest chunk,
	 * and attempt to load as many sectors from this chunk as possible in one go.
	 * Will first attempt to decompress a compressed column, then replace with modified sectors as they appear
	**/
	bool Generator::generate(Sector& sector)
	{
		if (sector.generated())
		{
			logger << Log::WARN << "Generator::generate(): sector content already generated" << Log::ENDL;
			return false;
		}

		// open this chunks .compressed file
		std::string sectorString = world.worldFolder() + "/" + chunks.getSectorString(sector);

		std::ifstream cf(sectorString + ".compressed", std::ios::in | std::ios::binary);
		bool cf_open = false;

		if (cf.good())
		{
			cf.seekg(sizeof(int));
			cf.read( (char*) g_compres, sizeof(g_compres) );
			cf_open = true;
		}

		// open this chunks (raw) .chunk file
		std::ifstream ff(sectorString + ".chunk", std::ios::in | std::ios::binary);
		bool ff_open = false;

		if (ff.good())
		{
			ff.seekg(sizeof(int));
			ff.read( (char*) g_fres, sizeof(g_fres) );
			ff_open = true;
		}

		// NOTE NOTE NOTE NOTE NOTE //
		//   NO EARLY EXITS HERE    //
		// NOTE NOTE NOTE NOTE NOTE //

		int dx = (sector.getX() + world.getWX()) & (Chunks::CHUNK_SIZE-1);
		int dz = (sector.getZ() + world.getWZ()) & (Chunks::CHUNK_SIZE-1);

		int x1 = sector.getX() - dx;
		int x2 = x1 + Chunks::CHUNK_SIZE;

		if (x1 < 0) x1 = 0;            // CLAMP AFTER x2 IS SET!!!
		if (x2 > sectors.getXZ()) x2 = sectors.getXZ();

		int z1 = sector.getZ() - dz;
		int z2 = z1 + Chunks::CHUNK_SIZE;

		if (z1 < 0) z1 = 0;            // CLAMP AFTER z2 IS SET!!!
		if (z2 > sectors.getXZ()) z2 = sectors.getXZ();

		for (int x = x1; x < x2; x++)
		{
			for (int z = z1; z < z2; z++)
			{
				// bottom / first sector in column
				Sector& sector = sectors(x, z);

				//-------------------------------------------------//
				// load only sectors that are flagged as 'unknown' //
				//-------------------------------------------------//

				if (sector.generated() == false)
				{
					// find sectors internal chunk position
					dx = (x + world.getWX()) & (Chunks::CHUNK_SIZE-1);
					dz = (z + world.getWZ()) & (Chunks::CHUNK_SIZE-1);

					// if compressed file is open, and there was an entry
					if (cf_open && (g_compres[dx][dz] != 0))
					{
						// read entire compressed column
						// compressed column also contains the flatland(x, z) for this area
						Compressor::load(cf, g_compres[dx][dz], x, z);

						// update minimap (eventually)
						minimap.sched(sector);
					}
					else
					{
						// reset flatlands
						//flatlands(x, z).reset();

						// clean out sector
						sector.clear();
					}

					if (ff_open)
					{
						// load single-file, if this sector has an entry
						if (g_fres[dz][dx])
						{
							// load sector using loadSectorEx method
							loadSector(sector, ff, g_fres[dz][dx]);
						}

					} // ff is open

				} // sector in unknown-state

			} // z
		} // x

		// time did not run out
		return false;

	} // generate()

}
