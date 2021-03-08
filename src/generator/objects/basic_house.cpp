#include "../object.hpp"
#include "../blocks.hpp"
#include "../random.hpp"
#include "../../sectors.hpp"
#include "../../spiders.hpp"
#include "../../world.hpp"
#include <library/noise/voronoi.hpp>
#include <glm/vec2.hpp>
#include <stdio.h>
#include <cassert>

namespace terragen
{
	using cppcraft::Spiders;
	using cppcraft::sectors;
	using cppcraft::Sector;
	using cppcraft::world;

	bool isHouse(int cellX, int cellZ)
	{
		// random is based on world position
		int random = rand2d(cellX * BLOCKS_XZ, cellZ * BLOCKS_XZ);
		if ((random & 3) == 0) return false;

		// p is based on position relative to world center
		glm::vec2 p((cellX - cppcraft::World::WORLD_CENTER) * BLOCKS_XZ,
					(cellZ - cppcraft::World::WORLD_CENTER) * BLOCKS_XZ);

		int v = library::Voronoi::getid(p.x * 0.01, p.y * 0.01,
				library::Voronoi::vor_chebyshev); // distance function
		return (v & 15) == 0;
	}

	void basic_house(const SchedObject& obj)
	{
		return;
		int random = rand2d(obj.x, obj.z);

		if (obj.y <= 64) return; // immediately avoid water
		if ((random & 3) == 0) return; // discard half the houses

		Block base(db::getb("patterned_stone"));
		Block brwood(db::getb("brown_wood"));
		Block plank(db::getb("plank"));
		Block dirtsoil(db::getb("grass_block"));
		Block gravel2(db::getb("gravel"));

		// local coordinates (centered on sector)
		int x = obj.x + BLOCKS_XZ / 2;
		int y = obj.y;
		int z = obj.z + BLOCKS_XZ / 2;

		// validate field
		Sector& sector = sectors(x / BLOCKS_XZ, z / BLOCKS_XZ);
		int h1 = sector.flat()(0, 0).groundLevel;
		int h2 = sector.flat()(BLOCKS_XZ-1, 0).groundLevel;
		int h3 = sector.flat()(0, BLOCKS_XZ-1).groundLevel;
		int h4 = sector.flat()(BLOCKS_XZ-1, BLOCKS_XZ-1).groundLevel;
		// if the level between the corners is too much, we will exit
		if (std::abs(h1-h2) > 2) return;
		if (std::abs(h2-h3) > 2) return;
		if (std::abs(h3-h4) > 2) return;
		if (std::abs(h4-h1) > 2) return;

		const int height = 6;
		int radX = 3 + (random >> 2 & 3);
		int radZ = 4 + (random >> 4 & 1);

		int houseType = (random >> 8) & 3;
		if (houseType < 2)
		{
			for (int dx = -radX; dx <= radX; dx++)
			for (int dz = -radZ; dz <= radZ; dz++)
			for (int dy = 0; dy <= height; dy++)
			{
				int fx = x + dx; int fy = y + dy; int fz = z + dz;

				if (dx == -radX || dx == radX || dz == -radZ || dz == radZ || dy == 0 || dy == height)
				{
					if (dy == 0)
					{
						Spiders::setBlock(fx, fy, fz, base);
					}
					else if ((dx == -radX || dx == radX) && (dz == -radZ || dz == radZ))
						Spiders::setBlock(fx, fy, fz, brwood);
					else
					{
						Spiders::setBlock(fx, fy, fz, plank);
					}
				}
			}
		}
		else if (houseType == 2)
		{
			// farm
			for (int dx = -radX; dx <= radX; dx++)
			for (int dz = -radZ; dz <= radZ; dz++)
			{
				if (dx == -radX || dx == radX || dz == -radZ || dz == radZ)
					Spiders::setBlock(x+dx, y, z+dz, brwood);
				else
					Spiders::setBlock(x+dx, y, z+dz, dirtsoil);
			}
		}
		else
		{
			// blank space
			for (int dx = -radX; dx <= radX; dx++)
			for (int dz = -radZ; dz <= radZ; dz++)
			{
				Spiders::setBlock(x+dx, y-1, z+dz, gravel2);
				Spiders::removeBlock(x+dx, y, z+dz);
			}
		}

		int x0 = 7 - radX;
		int x1 = 7 + radX + 1;
		int z0 = 7 - radZ;
		int z1 = 7 + radZ + 1;

		// create roads
		x &= ~(BLOCKS_XZ-1);
		z &= ~(BLOCKS_XZ-1);
		y -= 1;
		// remember that (wx, wz) is an offset from center of world
		int cellX = obj.x / BLOCKS_XZ;
		int cellZ = obj.z / BLOCKS_XZ;

		bool roadPX = isHouse(cellX+1, cellZ);
		bool roadNX = isHouse(cellX-1, cellZ);
		bool roadPZ = isHouse(cellX, cellZ+1);
		bool roadNZ = isHouse(cellX, cellZ-1);

		// if both sides of an axis have roads, then
		// ALL sides have roads, except when 3 sides are on
		roadPX = roadPX || (roadPZ && roadNZ && !roadNX);
		roadNX = roadNX || (roadPZ && roadNZ && !roadPX);
		roadPZ = roadPZ || (roadPX && roadNX && !roadNZ);
		roadNZ = roadNZ || (roadPX && roadNX && !roadPZ);

		if (roadPX)
		{
			// create road towards +x
			for (int dz = 0; dz < BLOCKS_XZ; dz++)
			for (int dx = x1; dx < BLOCKS_XZ; dx++)
			{
				Spiders::setBlock   (x + dx, y  , z + dz, gravel2);
				Spiders::removeBlock(x + dx, y+1, z + dz);
			}
		}
		if (roadNX)
		{
			// create road towards -x
			for (int dz = 0; dz < BLOCKS_XZ; dz++)
			for (int dx = 0; dx < x0; dx++)
			{
				Spiders::setBlock   (x + dx, y  , z + dz, gravel2);
				Spiders::removeBlock(x + dx, y+1, z + dz);
			}
		}
		if (roadPZ)
		{
			// create road towards +z
			for (int dx = 0; dx < BLOCKS_XZ; dx++)
			for (int dz = z1; dz < BLOCKS_XZ; dz++)
			{
				Spiders::setBlock   (x + dx, y  , z + dz, gravel2);
				Spiders::removeBlock(x + dx, y+1, z + dz);
			}
		}
		if (roadNZ)
		{
			// create road towards -z
			for (int dx = 0; dx < BLOCKS_XZ; dx++)
			for (int dz = 0; dz < z0; dz++)
			{
				Spiders::setBlock   (x + dx, y  , z + dz, gravel2);
				Spiders::removeBlock(x + dx, y+1, z + dz);
			}
		}
	}
}
