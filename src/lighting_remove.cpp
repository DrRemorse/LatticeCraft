#include "lighting.hpp"

#include "sectors.hpp"
#include "spiders.hpp"
#include <queue>

namespace cppcraft
{
	using emitter_t = Lighting::emitter_t;

  void removeSkylight(int x, int y, int z, char dir, char lvl, std::queue<emitter_t>& q)
  {
  	while (true)
  	{
  		// move in ray direction
  		switch (dir) {
  		case 0: x++; break;
  		case 1: x--; break;
  		case 2: y++; break;
  		case 3: y--; break;
  		case 4: z++; break;
  		case 5: z--; break;
  		}

  		// validate new position
  		if (x < 0 || y < 1 || z < 0 ||
  			x >= sectors.getXZ() * BLOCKS_XZ ||
  			z >= sectors.getXZ() * BLOCKS_XZ ||
  			y >= BLOCKS_Y) break;

  		Sector& sector = sectors(x / BLOCKS_XZ, z / BLOCKS_XZ);
  		assert(sector.generated());
  		Block& blk2 = sector(x & (BLOCKS_XZ-1), y, z & (BLOCKS_XZ-1));

  		auto block_lvl = blk2.getSkyLight();
  		// exit when we encounter a node with zero or maximum light
  		if (block_lvl == 0)
  		{
  			break;
  		}
  		else if (lvl < block_lvl)
  		{
        assert (block_lvl > 1);
  			// when the level is same or above, we will use this to refill the removed volume
        q.emplace(x, y, z, 0, dir, block_lvl);
  			break;
  		}
  		// our light was stronger, continue to remove

  		// set it to zero
      //printf("removeSkylight(%d, %d, %d): %d >= %d\n", x, y, z, lvl, block_lvl);
  		blk2.setSkyLight(0);

  		// simulate decrease of light level
  		lvl -= Lighting::lightPenetrate(blk2);
  		if (lvl <= 0) break;

  		// make sure the sectors mesh is updated, since something was changed
  		if (!sector.isUpdatingMesh())
  			   sector.updateAllMeshes();

  		switch (dir) {
  		case 0: // +x
  		case 1: // -x
  			removeSkylight(x, y, z, 2, lvl, q);
  			removeSkylight(x, y, z, 3, lvl, q);
  			removeSkylight(x, y, z, 4, lvl, q);
  			removeSkylight(x, y, z, 5, lvl, q);
  			break;
  		case 2: // +y
  		case 3: // -y
  			removeSkylight(x, y, z, 0, lvl, q);
  			removeSkylight(x, y, z, 1, lvl, q);
  			removeSkylight(x, y, z, 4, lvl, q);
  			removeSkylight(x, y, z, 5, lvl, q);
  			break;
  		case 4: // +z
  		case 5: // -z
  			removeSkylight(x, y, z, 0, lvl, q);
  			removeSkylight(x, y, z, 1, lvl, q);
  			removeSkylight(x, y, z, 2, lvl, q);
  			removeSkylight(x, y, z, 3, lvl, q);
  			break;
  		}
  	} // ray
  }

  // for each (x, y, z) that has a non-zero level lower than removed.lvl,
  // set them to zero. if the node is also a light source, queue it up,
  // so we can re-propagate its light after removing the @removed lightsource
  void removeChannel(int x, int y, int z, char dir, emitter_t& removed, std::queue<emitter_t>& q)
  {
  	// move in ray direction
  	switch (dir) {
  	case 0: x++; break;
  	case 1: x--; break;
  	case 2: y++; break;
  	case 3: y--; break;
  	case 4: z++; break;
  	case 5: z--; break;
  	}

  	// validate new position
  	if (x < 0 || y < 1 || z < 0 ||
  		x >= sectors.getXZ() * BLOCKS_XZ ||
  		z >= sectors.getXZ() * BLOCKS_XZ ||
  		y >= BLOCKS_Y) return;

  	Sector& sector = sectors(x / BLOCKS_XZ, z / BLOCKS_XZ);
  	//assert(sector.generated());
  	Block& blk2 = sector(x & (BLOCKS_XZ-1), y, z & (BLOCKS_XZ-1));

  	// exit when we encounter a node with non-zero light
  	// that is less than the level we are removing
  	char level = blk2.getChannel(removed.ch);
  	if (level != 0 && level < removed.lvl)
  	{
  		q.emplace(x, y, z, removed.ch, dir, level);
  		return;
  	}
  	else if (level >= removed.lvl)
  	{
  		q.emplace(x, y, z, removed.ch, dir, level);
  	}
  	else // level == 0
  	{
  		return;
  	}

  	// set it to zero
  	blk2.setChannel(removed.ch, 0);
  	// make sure the sectors mesh is updated, since something was changed
  	if (!sector.isUpdatingMesh()) sector.updateAllMeshes();

  	switch (dir)
  	{
  	case 0: // +x
  	case 1: // -x
  		removeChannel(x, y, z, 2, removed, q);
  		removeChannel(x, y, z, 3, removed, q);
  		removeChannel(x, y, z, 4, removed, q);
  		removeChannel(x, y, z, 5, removed, q);
  		break;
  	case 2: // +y
  	case 3: // -y
  		removeChannel(x, y, z, 0, removed, q);
  		removeChannel(x, y, z, 1, removed, q);
  		removeChannel(x, y, z, 4, removed, q);
  		removeChannel(x, y, z, 5, removed, q);
  		break;
  	case 4: // +z
  	case 5: // -z
  		removeChannel(x, y, z, 0, removed, q);
  		removeChannel(x, y, z, 1, removed, q);
  		removeChannel(x, y, z, 2, removed, q);
  		removeChannel(x, y, z, 3, removed, q);
  		break;
  	}
  }

  void Lighting::removeSkyLight(int x, int y1, int y2, int z, char lvl)
	{
		std::queue<emitter_t> q;
    // remove all light from whole column
    for (int y = y1; y <= y2; y++) {
      // start rays in all 6 directions
		  for (int dir = 0; dir < 6; dir++)
			  removeSkylight(x, y, z, dir, lvl, q);
    }
		// re-flood edge values that were encountered
		while (!q.empty())
		{
			const emitter_t& e = q.front();
      // if the light level remained the same: reflood
      auto lvl = Spiders::getBlock(e.x, e.y, e.z).getSkyLight();
      if (lvl == e.lvl) {
        floodOutof(e.x, e.y, e.z, 0, lvl);
      }
			q.pop();
		}

	} // removeSkyLight()

}
