#include "precomp_thread.hpp"

#include <library/log.hpp>
#include "precompiler.hpp"
#include "renderconst.hpp"
#include "sectors.hpp"
#include "tiles.hpp"
#include <cstring>

using namespace library;

namespace cppcraft
{
	const int PTD::REPEAT_FACTOR = RenderConst::VERTEX_SCALE / TileDB::TILES_PER_BIG_TILE;

	void PrecompThread::precompile(Precomp& pc)
	{
		// set sector from precomp
		ptd.sector = &pc.sector;

		// iterate up to skylevel for each (x, z)
		for (int bx = 0;  bx < BLOCKS_XZ; bx++)
		for (int bz = 0;  bz < BLOCKS_XZ; bz++)
		for (int by = 0;  by < pc.sector(bx, bz).skyLevel; by++)
		{
			// get pointer to current block
			Block& block = pc.sector(bx, by, bz);

			// ignore AIR
			if (LIKELY(block.getID() != _AIR))
			{
				// process one block id, and potentially add it to mesh
				// the generated mesh is added to a shaderline determined by its block id
				ptd.process_block(block, bx, by, bz);
			}
		}

		// count the number of vertices generated
		size_t total = 0;
		for (const auto& vec : ptd.vertices) {
      total += vec.size();
		}

		if (total == 0)
		{
			printf("[!] No vertices (total) for (%d, %d)\n",
				      pc.sector.wx, pc.sector.wz);
			// no vertices, we can exit early
			return;
		}

		// reserve exact number of vertices
		pc.datadump.reserve(total);

		// prepare for next stage
		size_t cnt = 0;
		for (int i = 0; i < RenderConst::MAX_UNIQUE_SHADERS; i++)
		{
      if (!ptd.vertices[i].empty())
      {
        pc.datadump.insert(pc.datadump.end(),
                           ptd.vertices[i].begin(), ptd.vertices[i].end());
      }
      pc.vertices[i] = ptd.vertices[i].size();
      pc.bufferoffset[i] = cnt;
      cnt += ptd.vertices[i].size();
		}

    assert(pc.datadump.size() == total);
	}

}
