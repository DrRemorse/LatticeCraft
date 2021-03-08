#pragma once
#include "sectorblock.hpp"
#include "common.hpp"

namespace cppcraft
{
  struct readonly_wrapper_t
  {
    readonly_wrapper_t(sectorblock_t& sb, int X, int Z)
        : blocks(sb), x(X), z(Z) {}

    inline const Block& operator() (int x, int y, int z) const {
      return blocks(x, y, z);
    }

    const sectorblock_t blocks;
    const int x;
    const int z;
  };
	struct ReadonlyBlocks
	{
		ReadonlyBlocks(int wx, int wz, int radius);

		auto& operator() (int x, int z)
		{
      if (x < -radius || x > radius) throw std::out_of_range("Not within radius");
      if (z < -radius || z > radius) throw std::out_of_range("Not within radius");
      return storage[ (x + radius) * (radius*2+1) + (z + radius) ];
		}
    auto size() const noexcept {
      return storage.size();
    }

    const int wx, wz;
    const int radius;
  private:
    std::vector<readonly_wrapper_t> storage;
	};
}
