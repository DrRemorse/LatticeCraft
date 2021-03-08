#include "volumetrics.hpp"

#include "../terragen.hpp"
#include "../random.hpp"
#include "../../spiders.hpp"
#include <cstring>

using namespace cppcraft;

namespace terragen
{
  bool Volumetrics::discover(int x, int y, int z, int64_t id)
  {
  	const int DISCOVERIES = 8;
  	const int LEVELS = 4;

  	for (int i = 0; i < DISCOVERIES; i++)
  	{
  		int dx = x + randf(x + 14, y + i * 13, z + 13) * 16.0 - 8.0;
  		int dz = z + randf(x + 17, y - i *  7, z + 19) * 16.0 - 8.0;

  		for (int j = 1; j < LEVELS; j++)
  		{
  			const auto blk = Spiders::getBlock(dx, y - j, dz);
  			if (blk.getID() == id) return true;
  		}
  	}
  	return false;
  }

  int Volumetrics::depth(int x, int y, int z, int depth, const int SRC_ID)
  {
    int measurement = 0;
    while (depth-- > 0)
    {
    	const auto blk = Spiders::getBlock(x, y--, z);
    	if (blk.getID() != SRC_ID && blk.triviallyOverwriteable()) measurement++;
    }
    return measurement;
  }

  void Volumetrics::fillDown(int x, int y, int z, int mat, int depth, int travel)
  {
    if (x < 0 || z < 0 || x >= sectors.getXZ() * BLOCKS_XZ ||
        y < 0 || depth <= 0 || z >= sectors.getXZ() * BLOCKS_XZ) return;
  	travel--;

  	int dy;
  	for (dy = y; dy > 0; dy--)
  	{
  		auto& blk = Spiders::getBlock(x, dy, z);
  		if (blk.getID() == mat || !blk.triviallyOverwriteable())
  		{
  			if (dy != y && blk.getID() != mat && travel > 0)
  			{	// don't fill at (or over) own starting height and avoid own mat
  				fillDown(x+1, dy+1, z, mat, depth, travel);
  				fillDown(x-1, dy+1, z, mat, depth, travel);
  				fillDown(x, dy+1, z+1, mat, depth, travel);
  				fillDown(x, dy+1, z-1, mat, depth, travel);
  			}
  			return;
  		}
  		blk.setID(mat);
  	}
  	if (dy <= WATERLEVEL || travel == 0) return;
  	dy++; // go back up

  	// fill bottom, if possible (will not cause another fill immediately)
  	fillDown(x+1, dy, z, mat, depth, travel);
  	fillDown(x-1, dy, z, mat, depth, travel);
  	fillDown(x, dy, z+1, mat, depth, travel);
  	fillDown(x, dy, z-1, mat, depth, travel);
  }

  bool Volumetrics::flood(int x, int z, const int SIZE, depthmap_t& depth, pathmap_t& path)
  {
  	if (depth[x][z] == 0) // has no depth, simply return
  		return true;
  	// after checking for blockage, there's a possibility we are at any
  	// of the ends of the depth map, if we are, and its open, we failed to fill
  	if (x == 0 || x == SIZE-1 || z == 0 || z == SIZE-1) return false;

  	if (path[x][z]) // we've been here before, simply return
  		return true;

  	// has depth, so set path to true
  	path[x][z] = true;
  	// flood neighbors, immediately exit if any path failed
  	if (flood(x + 1, z, SIZE, depth, path) == false) return false;
  	if (flood(x - 1, z, SIZE, depth, path) == false) return false;
  	if (flood(x, z + 1, SIZE, depth, path) == false) return false;
  	if (flood(x, z - 1, SIZE, depth, path) == false) return false;
  	return true;
  }

  bool Volumetrics::fill(int x, int y, int z, int64_t id)
  {
  	// lookup table
    depthmap_t depth;

  	// calculate volume
  	for (int bx = 0; bx < VOLSIZE; bx++)
  	for (int bz = 0; bz < VOLSIZE; bz++)
  	{
  		depth[bx][bz] = Spiders::getBlock(x + bx - VOLRAD, y, z + bz - VOLRAD).isAir();
  	}

    pathmap_t paths{};
  	// try flood-filling at each position inside the rect
  	for (int bx = 1; bx < VOLSIZE-1; bx++)
  	for (int bz = 1; bz < VOLSIZE-1; bz++)
  	{
  		if (depth[bx][bz])
  		{
  			if (flood(bx, bz, VOLSIZE, depth, paths) == false)
  			{
  				// too many :(
  				return false;
  				// reset path map and try again
  				memset(&paths, 0, sizeof(paths));
  			}
  			else goto weGoodNow;
  		}
  	}
  	// all floods failed, exit
  	return false;
  	// flood-filling worked
weGoodNow:
  	// if we get here, we have a 2D volume (with possible depth exits)
  	// fill the path we found
  	for (int bx = 1; bx < VOLSIZE-1; bx++)
  	for (int bz = 1; bz < VOLSIZE-1; bz++)
  	{
  		if (paths[bx][bz])
  			fillDown(x + bx - VOLRAD, y, z + bz - VOLRAD, id, VOLDEPTH, VOLTRAVEL);
  	}

  	return true;
  }

  void Volumetrics::job_fill(const SchedObject& obj)
  {
    Volumetrics::fill(obj.x, obj.y, obj.z, obj.data);
  }

} // terragen
