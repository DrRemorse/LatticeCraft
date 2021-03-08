#include "spiders.hpp"

#include "player.hpp"
#include "sector.hpp"
#include "world.hpp"
#include <glm/vec3.hpp>
#include <cmath>

namespace cppcraft
{
	glm::vec3 Spiders::distanceToWorldXZ(int wx, int wz)
	{
		// integral player position
		int plx = player.pos.x;
		int plz = player.pos.z;
		// fractional values (internal to a block position)
		double pfracX = player.pos.x - plx;
		double pfracZ = player.pos.z - plz;

		// current world position (in absolute sectors)
		int currentWX = world.getWX() + plx / BLOCKS_XZ;
		int currentWZ = world.getWZ() + plz / BLOCKS_XZ;

		// current delta-world position (in absolute blocks)
		double dwx = (wx - currentWX) * BLOCKS_XZ;
		double dwz = (wz - currentWZ) * BLOCKS_XZ;

		// add deltas from center of a sector to player
		dwx -= BLOCKS_XZ * 0.5 - 0.5 + (plx & (Sector::BLOCKS_XZ-1)) + pfracX;
		dwz -= BLOCKS_XZ * 0.5 - 0.5 + (plz & (Sector::BLOCKS_XZ-1)) + pfracZ;

		// normalize
		double L = sqrt( dwx*dwx + dwz*dwz );
		if (L == 0.0) return glm::vec3( 0.0, 0.0, -1.0 );

		return glm::vec3( dwx / L, 0.0, dwz / L );
	}

}
