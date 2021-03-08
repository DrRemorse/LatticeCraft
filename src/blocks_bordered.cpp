#include "blocks_bordered.hpp"

#include "sectors.hpp"
#include <cstring>

namespace cppcraft
{
	bordered_sector_t::bordered_sector_t(Sector& sector)
    : wx(sector.getWX()), wz(sector.getWZ())
	{
		// copy entire row from sector into sectorblock
		for (int x = 0; x < BLOCKS_XZ; x++)
		{
			Block* src = &sector(x, 0, 0);
			Block* dst = &get(x, 0, 0);
			memcpy(dst, src, BLOCKS_XZ * BLOCKS_Y * sizeof(Block));
		}

		// copy borders from neighbors
		// (-X)
		if (sector.getX() > 0)
		{
			Sector& nbor = sectors(sector.getX()-1, sector.getZ());
			for (int z = 0; z < BLOCKS_XZ; z++)
			{
				Block* src = &nbor(BLOCKS_XZ-1, 0, z);
				Block* dst = &get(-1, 0, z);
				memcpy(dst, src, BLOCKS_Y * sizeof(Block));
			}
		}
		else
		{
			// when out of range, we need to set it to _AIR to prevent strange things
			for (int z = 0; z < BLOCKS_XZ; z++)
			for (int y = 0; y < BLOCKS_Y; y++)
				get(-1, y, z) = Block(_AIR);
		}
		// (+X)
		if (sector.getX()+1 < sectors.getXZ())
		{
			Sector& nbor = sectors(sector.getX()+1, sector.getZ());
			for (int z = 0; z < BLOCKS_XZ; z++)
			{
				Block* src = &nbor(0, 0, z);
				Block* dst = &get(BLOCKS_XZ, 0, z);
				memcpy(dst, src, BLOCKS_Y * sizeof(Block));
			}
		}
		else
		{
			// when out of range, we need to set it to _AIR to prevent strange things
			for (int z = 0; z < BLOCKS_XZ; z++)
			for (int y = 0; y < BLOCKS_Y; y++)
				get(BLOCKS_XZ, y, z) = Block(_AIR);
		}
		// (-Z)
		if (sector.getZ() > 0)
		{
			Sector& nbor = sectors(sector.getX(), sector.getZ()-1);
			for (int x = 0; x < BLOCKS_XZ; x++)
			{
				Block* src = &nbor(x, 0, BLOCKS_XZ-1);
				Block* dst = &get(x, 0, -1);
				memcpy(dst, src, BLOCKS_Y * sizeof(Block));
			}
		}
		else
		{
			// when out of range, we need to set it to _AIR to prevent strange things
			for (int x = 0; x < BLOCKS_XZ; x++)
			for (int y = 0; y < BLOCKS_Y; y++)
				get(x, y, -1) = Block(_AIR);
		}
		// (+Z)
		if (sector.getZ()+1 < sectors.getXZ())
		{
			Sector& nbor = sectors(sector.getX(), sector.getZ()+1);
			for (int x = 0; x < BLOCKS_XZ; x++)
			{
				Block* src = &nbor(x, 0, 0);
				Block* dst = &get(x, 0, BLOCKS_XZ);
				memcpy(dst, src, BLOCKS_Y * sizeof(Block));
			}
		}
		else
		{
			// when out of range, we need to set it to _AIR to prevent strange things
			for (int x = 0; x < BLOCKS_XZ; x++)
			for (int y = 0; y < BLOCKS_Y; y++)
				get(x, y, BLOCKS_XZ) = Block(_AIR);
		}
		// (-XZ)
		if (sector.getX() > 0 && sector.getZ() > 0)
		{
			Sector& nbor = sectors(sector.getX()-1, sector.getZ()-1);

			Block* src = &nbor(BLOCKS_XZ-1, 0, BLOCKS_XZ-1);
			Block* dst = &get(-1, 0, -1);
			memcpy(dst, src, BLOCKS_Y * sizeof(Block));
		}
		else
		{
			// when out of range, we need to set it to _AIR to prevent strange things
			for (int y = 0; y < BLOCKS_Y; y++)
				get(-1, y, -1) = Block(_AIR);
		}
		// (+XZ)
		if (sector.getX() < sectors.getXZ()-1
		 && sector.getZ() < sectors.getXZ()-1)
		{
			Sector& nbor = sectors(sector.getX()+1, sector.getZ()+1);

			Block* src = &nbor(0, 0, 0);
			Block* dst = &get(BLOCKS_XZ, 0, BLOCKS_XZ);
			memcpy(dst, src, BLOCKS_Y * sizeof(Block));
		}
		else
		{
			// when out of range, we need to set it to _AIR to prevent strange things
			for (int y = 0; y < BLOCKS_Y; y++)
				get(BLOCKS_XZ, y, BLOCKS_XZ) = Block(_AIR);
		}
		// (+X-Z)
		if (sector.getX() < sectors.getXZ()-1 && sector.getZ() > 0)
		{
			Sector& nbor = sectors(sector.getX()+1, sector.getZ()-1);

			Block* src = &nbor(0, 0, BLOCKS_XZ-1);
			Block* dst = &get(BLOCKS_XZ, 0, -1);
			memcpy(dst, src, BLOCKS_Y * sizeof(Block));
		}
		else
		{
			// when out of range, we need to set it to _AIR to prevent strange things
			for (int y = 0; y < BLOCKS_Y; y++)
				get(BLOCKS_XZ, y, -1) = Block(_AIR);
		}
		// (-X+Z)
		if (sector.getX() > 0 && sector.getZ() < sectors.getXZ()-1)
		{
			Sector& nbor = sectors(sector.getX()-1, sector.getZ()+1);

			Block* src = &nbor(BLOCKS_XZ-1, 0, 0);
			Block* dst = &get(-1, 0, BLOCKS_XZ);
			memcpy(dst, src, BLOCKS_Y * sizeof(Block));
		}
		else
		{
			// when out of range, we need to set it to _AIR to prevent strange things
			for (int y = 0; y < BLOCKS_Y; y++)
				get(-1, y, BLOCKS_XZ) = Block(_AIR);
		}

		/// flatland data ///

		// copy entire row from flatland
		for (int x = 0; x < BLOCKS_XZ; x++)
		{
			auto* src = &sector.flat()(x, 0);
			auto* dst = &this->fget(x, 0);
			memcpy(dst, src, BLOCKS_XZ * sizeof(Flatland::flatland_t));
		}

		// +x
		if (sector.getX() < sectors.getXZ()-1)
		{
			Sector& nbor = sectors(sector.getX()+1, sector.getZ());
      CC_ASSERT(nbor.generated(), "Neighboring sector wasn't generated");

			for (int z = 0; z < BLOCKS_XZ; z++)
			{
				this->fget(BLOCKS_XZ, z) = nbor.flat()(0, z);
			}
		}
		// +z
		if (sector.getZ() < sectors.getXZ()-1)
		{
			Sector& nbor = sectors(sector.getX(), sector.getZ()+1);
      CC_ASSERT(nbor.generated(), "Neighboring sector wasn't generated");

			for (int x = 0; x < BLOCKS_XZ; x++)
			{
				this->fget(x, BLOCKS_XZ) = nbor.flat()(x, 0);
			}

		}
		// +xz
		if (sector.getX() < sectors.getXZ()-1 &&
			sector.getZ() < sectors.getXZ()-1)
		{
			Sector& nbor = sectors(sector.getX()+1, sector.getZ()+1);
      CC_ASSERT(nbor.generated(), "Neighboring sector wasn't generated");

			this->fget(BLOCKS_XZ, BLOCKS_XZ) = nbor.flat()(0, 0);
		}
		else
		{
			// we always want valid values!
			this->fget(BLOCKS_XZ, BLOCKS_XZ) =
				this->fget(BLOCKS_XZ-1, BLOCKS_XZ-1);
		}
	} // bordered_sectorblock_t()

}
