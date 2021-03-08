#include "sectors.hpp"

#include "generator.hpp"
#include <cassert>
#include <stdint.h>

namespace cppcraft
{
  Sectors::Sectors(const int xz) {
    this->rebuild(xz);
  }

	Sector* Sectors::sectorAt(float x, float z)
	{
		if (x >= 0 && x < BLOCKS_XZ * getXZ() &&
			  z >= 0 && z < BLOCKS_XZ * getXZ())
		{
			return getSector(x / BLOCKS_XZ, z / BLOCKS_XZ);
		}
		return nullptr;
	}

	// allocates and initializes all Sectors
	void Sectors::rebuild(int sectors_xz)
	{
		// set number of sectors on X/Z axes
		this->sectors_XZ = sectors_xz;
    // rebuild vector
    size_t sectsz = sizeof(Sector) + sizeof(sectorblock_t);
    size_t total = this->sectors_XZ * this->sectors_XZ;
    size_t bytes = sectsz * total;
    printf("Allocating %zu * %zu sectors = %zu bytes (%zu MiB)\n",
            sectsz, total, bytes, bytes / (1024 * 1024));
    sectors.clear();
		sectors.reserve(total);
		// iterate and construct sectors
		for (int x = 0; x < sectors_XZ; x++)
		for (int z = 0; z < sectors_XZ; z++)
		{
      sectors.emplace_back(new Sector(x, z));
		} // y, z, x
    assert(sectors.size() == total);
	}

	void Sectors::updateAll()
	{
    for (auto& sector : sectors)
       sector->updateAllMeshes();
	}
	void Sectors::regenerateAll()
	{
    for (auto& sector : sectors)
		{
			// clear generated flag and add to generator queue
			sector->gen_flags &= ~1;
			Generator::add(*sector);
		}
	}

	Flatland::flatland_t* Sectors::flatland_at(int x, int z)
	{
		// find flatland sector
		int fx = x / Sector::BLOCKS_XZ;
		fx %= getXZ();
		int fz = z / Sector::BLOCKS_XZ;
		fz %= getXZ();
		// avoid returning garbage when its not loaded yet
		if (getSector(fx, fz)->generated() == false)
			return nullptr;

		// find internal position
		int bx = x & (Sector::BLOCKS_XZ-1);
		int bz = z & (Sector::BLOCKS_XZ-1);
		// return data structure
		return &flatland(fx, fz)(bx, bz);
	}

	bool Sectors::onNxN(const Sector& sect, int size, delegate<bool(Sector&)> func)
	{
		int x0 = sect.getX()-size; x0 = (x0 >= 0) ? x0 : 0;
		int x1 = sect.getX()+size; x1 = (x1 < getXZ()) ? x1 : getXZ()-1;
		int z0 = sect.getZ()-size; z0 = (z0 >= 0) ? z0 : 0;
		int z1 = sect.getZ()+size; z1 = (z1 < getXZ()) ? z1 : getXZ()-1;

		for (int x = x0; x <= x1; x++)
		for (int z = z0; z <= z1; z++)
		{
			if ( func(*getSector(x, z)) == false )
				return false;
		}
		return true;
	}

}
