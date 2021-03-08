#include "lighting.hpp"

#include <library/math/toolbox.hpp>
#include "sectors.hpp"
#include "spiders.hpp"
#include <cmath>
#include <queue>

using namespace library;

namespace cppcraft
{
  using emitter_t = Lighting::emitter_t;
	extern void removeChannel(int x, int y, int z, char dir, emitter_t& removed, std::queue<emitter_t>& q);

	void Lighting::init()
	{
		extern Block air_block;
		air_block.setLight(15, 0);
	}

	light_value_t Lighting::lightValue(const Block& block)
	{
		light_value_t result = 0;

		for (int ch = 0; ch < Block::CHANNELS; ch++)
		{
			int V = block.getChannel(ch);
			// apply compound decay
			V = 255.0f * powf(1.0f - 0.12f, 15 - V);
			// shift into correct channel
			result |= V << (ch * 8);
		}
		return result;
	}

  inline void beginPropagateSkylight(Sector* s, int bx, int by, int bz, char mask)
  {
  	if (mask & 1) Lighting::propagateChannel(s, bx, by, bz, {0, 0, 15}); // +x
  	if (mask & 2) Lighting::propagateChannel(s, bx, by, bz, {0, 1, 15}); // -x
  	if (mask & 4) Lighting::propagateChannel(s, bx, by, bz, {0, 4, 15}); // +z
  	if (mask & 8) Lighting::propagateChannel(s, bx, by, bz, {0, 5, 15}); // -z
  }

  void Lighting::atmosphericFlood(Sector& sector)
  {
    for (int x = 0; x < BLOCKS_XZ; x++)
    for (int z = 0; z < BLOCKS_XZ; z++)
    {
      // get skylevel .. again
      const int sky = sector.flat()(x, z).skyLevel;

      for (int y = BLOCKS_Y-1; y >= sky; y--)
      {
        // propagate skylight outwards, starting with light level 15 (max)
    		char mask = 15;
    		if (x < BLOCKS_XZ-1)
    			if (sector.flat()(x+1, z).skyLevel <= y) mask &= ~1;
    		if (x > 0)
    			if (sector.flat()(x-1, z).skyLevel <= y) mask &= ~2;
    		if (z < BLOCKS_XZ-1)
    			if (sector.flat()(x, z+1).skyLevel <= y) mask &= ~4;
    		if (z > 0)
    			if (sector.flat()(x, z-1).skyLevel <= y) mask &= ~8;

		    beginPropagateSkylight(&sector, x, y, z, mask);
      } // y

	    // try to enter water and other transparent blocks at the skylevel
	    if (sector(x, sky, z).isTransparent())
			   propagateChannel(&sector, x, sky, z, {0, 3, 14});

    } // x, z

	  torchlight(sector);

	  // avoid doing it again
	  sector.atmospherics = true;
  } // atmospheric flood

  void Lighting::torchlight(Sector& sector)
  {
    int light_count = 0;
    for (int y = 1; y <= sector.getHighestLightPoint(); y++)
    if (sector.hasLight(y))
    {
      for (int x = 0; x < BLOCKS_XZ; x++)
      for (int z = 0; z < BLOCKS_XZ; z++)
      {
  		  Block& block = sector(x, y, z);

        if (block.isLight())
  		  {
          light_count++;
    			const int ch = 1;

    			// set to max blocklight value
          uint8_t opacity = block.getOpacity(0);
          block.setChannel(ch, opacity);

          // mask out impossible paths
          int mask = 63;
          if (x < BLOCKS_XZ-1)
            if (!sector(x+1, y, z).isTransparent()) mask &= ~1;
          if (x > 0)
            if (!sector(x-1, y, z).isTransparent()) mask &= ~2;

          if (!sector(x, y+1, z).isTransparent()) mask &= ~4;
          if (!sector(x, y-1, z).isTransparent()) mask &= ~8;

          if (z < BLOCKS_XZ-1)
            if (!sector(x, y, z+1).isTransparent()) mask &= ~16;
          if (z > 0)
            if (!sector(x, y, z-1).isTransparent()) mask &= ~32;

          // propagate block light in all directions
          if (mask & 1)
            propagateChannel(&sector, x, y, z, {ch, 0, opacity}); // +x
          if (mask & 2)
            propagateChannel(&sector, x, y, z, {ch, 1, opacity}); // -x
          if (mask & 4)
            propagateChannel(&sector, x, y, z, {ch, 2, opacity}); // +y
          if (mask & 8)
            propagateChannel(&sector, x, y, z, {ch, 3, opacity}); // -y
          if (mask & 16)
            propagateChannel(&sector, x, y, z, {ch, 4, opacity}); // +z
          if (mask & 32)
            propagateChannel(&sector, x, y, z, {ch, 5, opacity}); // -z

    		} // isLight()
      } // x, z
    } // is light source(y)
    // update sectors light count (mostly for debugging)
    sector.getBlocks().light_count = light_count;
  } // atmospheric flood

  #define DO_FLOOD_INTO(dir) {                 \
      s2 = Spiders::wrap(*s, bx, by, bz);      \
      if (s2 != nullptr) {                     \
        auto lvl = (*s2)(bx, by, bz).getChannel(ch); \
        if (lvl > 1) propagateChannel(s2, bx, by, bz, {ch, dir, lvl}); \
      }}

  void Lighting::floodInto(Sector* s, int x, int y, int z, short ch)
  {
    Sector* s2 = nullptr;
    int bx, by, bz;

	  // for each neighbor to this block, flood in from
    bx = x-1; by = y; bz = z;
    DO_FLOOD_INTO(0); // +x

    bx = x+1; by = y; bz = z;
    DO_FLOOD_INTO(0); // -x

    bx = x; by = y-1; bz = z;
    DO_FLOOD_INTO(2); // +y

    bx = x; by = y+1; bz = z;
    DO_FLOOD_INTO(3); // -y

    bx = x; by = y; bz = z-1;
    DO_FLOOD_INTO(4); // +z

    bx = x; by = y; bz = z+1;
    DO_FLOOD_INTO(5); // -z
  }

	void Lighting::floodOutof(int x, int y, int z, short ch, short lvl)
	{
    Sector* s = Spiders::wrap(x, y, z);
    assert(s != nullptr);
		// for each neighbor to this block, try to
		// propagate light from ch outwards
		propagateChannel(s, x, y, z, {ch, 0, lvl}); // +x
		propagateChannel(s, x, y, z, {ch, 1, lvl}); // -x
		propagateChannel(s, x, y, z, {ch, 2, lvl}); // +y
		propagateChannel(s, x, y, z, {ch, 3, lvl}); // -y
		propagateChannel(s, x, y, z, {ch, 4, lvl}); // +z
		propagateChannel(s, x, y, z, {ch, 5, lvl}); // -z
	}

	void Lighting::skyrayDownwards(Sector& sector, int bx, int by, int bz)
	{
		for (int y = by; y >= 0; y--)
		{
			if (sector(bx, y, bz).isAir())
			{
        // promote to queen
        sector(bx, y, bz).setSkyLight(15);
      }
      else
      {
        // set new skylevel to this y-value
        const int new_skylevel = y + 1;
				sector.flat()(bx, bz).skyLevel = new_skylevel;
        // send out rays on all sides along column
        for (y = by; y >= new_skylevel; y--) {
  				propagateChannel(&sector, bx, y, bz, {0, 0, 15}); // +x
  				propagateChannel(&sector, bx, y, bz, {0, 1, 15}); // -x
  				propagateChannel(&sector, bx, y, bz, {0, 4, 15}); // +z
  				propagateChannel(&sector, bx, y, bz, {0, 5, 15}); // -z
        }
        y = new_skylevel - 1;
				// try to enter below, if its transparent
				if (sector(bx, y, bz).isTransparent())
				{
					propagateChannel(&sector, bx, y, bz, {0, 3, 14});
				}
				break;
			}
		}
	}

	void Lighting::removeLight(const Block& blk, int srcX, int srcY, int srcZ)
	{
    // queue of lights we passed
		std::queue<emitter_t> q;

		assert(blk.isLight());
		// radius of light
		uint8_t rad = blk.getOpacity(0);

		// clear out ALL torchlight in this radius,
		// and remember all the lights we pass
		for (int x = srcX - rad; x <= srcX + rad; x++)
		for (int z = srcZ - rad; z <= srcZ + rad; z++)
		for (int y = srcY - rad; y <= srcY + rad; y++)
		{
			// validate new position
			if (x < 0 || y < 1 || z < 0 ||
				  x >= sectors.getXZ() * BLOCKS_XZ ||
				  z >= sectors.getXZ() * BLOCKS_XZ ||
				  y >= BLOCKS_Y)  continue;

			Sector& sector = sectors(x / BLOCKS_XZ, z / BLOCKS_XZ);
			//assert(sector.generated());
			int bx = x & (BLOCKS_XZ-1);
			int bz = z & (BLOCKS_XZ-1);
			Block& blk2 = sector(bx, y, bz);

			if (blk2.isLight())
			{
				q.emplace(x, y, z, 1, 0, blk2.getOpacity(0));
			}
			else
			{
				blk2.setTorchLight(0);
				if (!sector.isUpdatingMesh())
					sector.updateAllMeshes();
			}
		}

		// re-flood lights that we crossed by
		while (!q.empty())
		{
			const emitter_t& e = q.front();
      auto lvl = Spiders::getBlock(e.x, e.y, e.z).getSkyLight();
      if (lvl > 1) floodOutof(e.x, e.y, e.z, e.ch, lvl);
      q.pop();
		}

		// go through all the edges and send rays back inwards (?)
		//....

	} // removeLight()

} // namespace
