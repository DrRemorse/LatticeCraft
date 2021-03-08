#pragma once
#include "block.hpp"

#include "common.hpp"
#include "sector.hpp"

namespace cppcraft
{
	struct bordered_sector_t
	{
		bordered_sector_t(Sector& sector);

		// Block & biome retrieval functions
		inline Block& get (int bx, int by, int bz)
		{
			return blks[(bx+1) * (BLOCKS_XZ+2) * BLOCKS_Y + (bz+1) * BLOCKS_Y + by];
		}
		inline auto& fget (int bx, int bz)
		{
			return flats[bx * (BLOCKS_XZ+1) + bz];
		}

		Block& operator() (int bx, int by, int bz)
		{
			return get(bx, by, bz);
		}
		auto& operator() (int bx, int bz)
		{
			return fget(bx, bz);
		}

    const int wx, wz;
  private:
    // all the blocks from source sector and neighbors
		alignas(32) std::array<Block, (BLOCKS_XZ+2) * (BLOCKS_XZ+2) * BLOCKS_Y> blks;

		// all the 2D data from source sector and neighbors
 	  alignas(32) std::array<Flatland::flatland_t, (BLOCKS_XZ+1) * (BLOCKS_XZ+1)> flats;
	};
}
