#ifndef SPIDERS_HPP
#define SPIDERS_HPP

#include "common.hpp"
#include "sectors.hpp"
#include <glm/vec3.hpp>

namespace cppcraft
{
	class Spiders {
	public:
		// various block getters
		static Block& getBlock(int x, int y, int z);
		static Block& getBlock(Sector&, int x, int y, int z);
		static Block& getBlock(float x, float y, float z, float size_xz);

		// converts a position (x, y, z) to an explicit in-system position
		// returns false if the position would become out of bounds (after conversion)
		static inline Sector* wrap(int& bx, int& by, int& bz) noexcept;
		// converts a position (s, x, y, z) to an explicit in-system position
		// returns false if the position would become out of bounds (after conversion)
		static inline Sector* wrap(Sector& s, int& bx, int& by, int& bz) noexcept;

		static Block testArea(float x, float y, float z);
		static Block testAreaEx(float x, float y, float z);

		// updating & modifying world
    // fast-path
    static bool  updateBlock(Sector&, int bx, int by, int bz, block_t bitfield);
		static bool  setBlock(Sector&, int bx, int by, int bz, const Block& block);
		static Block removeBlock(Sector&, int bx, int by, int bz);
    // slow-path
		static bool  updateBlock(int bx, int by, int bz, block_t bitfield);
		static bool  setBlock(int bx, int by, int bz, const Block& block);
		static Block removeBlock(int bx, int by, int bz);

		// world distance calculations
		static glm::vec3 distanceToWorldXZ(int wx, int wz);

		// updating neighbors
		static void updateSurroundings(Sector&, int bx, int by, int bz);

		// returns the light values at (x, y, z)
		static light_value_t getLightNow(float x, float y, float z);

    // stats
    static int64_t total_blocks_placed() noexcept;
	};
	extern Block air_block;


  inline Sector* Spiders::wrap(int& bx, int& by, int& bz) noexcept
  {
    const int fx = bx / Sector::BLOCKS_XZ;
		const int fz = bz / Sector::BLOCKS_XZ;

		if (UNLIKELY(fx < 0 || fx >= sectors.getXZ() ||
			           fz < 0 || fz >= sectors.getXZ() ||
			           by < 0 || by >= BLOCKS_Y))
				return nullptr;

		bx &= Sector::BLOCKS_XZ-1;
		bz &= Sector::BLOCKS_XZ-1;
		return &sectors(fx, fz);
	}

  inline Sector* Spiders::wrap(Sector& s, int& bx, int& by, int& bz) noexcept
	{
		const int fx = s.getX() + bx / Sector::BLOCKS_XZ;
		const int fz = s.getZ() + bz / Sector::BLOCKS_XZ;

    if (UNLIKELY(fx < 0 || fx >= sectors.getXZ() ||
			           fz < 0 || fz >= sectors.getXZ() ||
			           by < 0 || by >= BLOCKS_Y))
				return nullptr;

		bx &= Sector::BLOCKS_XZ-1;
		bz &= Sector::BLOCKS_XZ-1;
		return &sectors(fx, fz);
	}
}

#endif
