#include "sector.hpp"

#include <library/log.hpp>
#include "precompq.hpp"
#include "sectors.hpp"
#include "world.hpp"
#include <cassert>
#include <cstring>
#include <cmath>

using namespace library;

namespace cppcraft
{
	// returns the world absolute coordinates for this sector X and Z
	int Sector::getWX() const
	{
		return world.getWX() + this->x;
	}
	int Sector::getWZ() const
	{
		return world.getWZ() + this->z;
	}

	float Sector::distanceTo(const Sector& sector, int bx, int bz) const
	{
		// centroidal
		int dx = ((getX() - sector.getX()) * BLOCKS_XZ) + (BLOCKS_XZ / 2 - bx);
		int dz = ((getZ() - sector.getZ()) * BLOCKS_XZ) + (BLOCKS_XZ / 2 - bz);

		return sqrtf(dx*dx + dz*dz) - (BLOCKS_XZ / 2) * sqrtf(3.0);
	}

	bool Sector::isReadyForAtmos() const
	{
		return sectors.onNxN(*this, 1, // 3x3
		[] (Sector& sect)
		{
			return sect.generated() && sect.objects == 0;
		});
	}
	void Sector::updateAllMeshes()
	{
		precompq.add(*this);
	}

  void Sector::clear()
  {
    for (auto& bl : m_blocks->b)
        bl = Block(_AIR, 0, 0, 15);
    this->gen_flags = GENERATED;
    this->objects   = 0;
    this->atmospherics = false;
  }

	void Sector::regenerate()
	{
		gen_flags    = 0;
		objects      = 0;
		atmospherics = false;
	}
}
