#pragma once
#include "block.hpp"

#include "common.hpp"
#include <array>
#include <cstdint>

namespace cppcraft
{
  struct sectorblock_t
  {
    Block& operator() (int x, int y, int z) {
      return b[x * BLOCKS_XZ * BLOCKS_Y + z * BLOCKS_Y + y];
    }
    const Block& operator() (int x, int y, int z) const {
      return b[x * BLOCKS_XZ * BLOCKS_Y + z * BLOCKS_Y + y];
    }
    std::array<Block, BLOCKS_XZ * BLOCKS_XZ * BLOCKS_Y> b;

    void setLight(short y) noexcept {
      m_lights.at(y / 64) |= 1 << (y % 64);
      if (y > highest_light_y) highest_light_y = y;
    }
    bool getLight(int y) const noexcept {
      return m_lights.at(y / 64) & (1 << (y % 64));
    }
    void clearLights() {
      for (auto& val : m_lights) val = 0;
    }

    uint16_t version() const noexcept { return m_version; }
    void next_version() { m_version++; }

    uint16_t light_count = 0;
    int16_t  highest_light_y = 0;
  private:
    std::array<uint64_t, BLOCKS_Y / 64> m_lights;
    uint16_t m_version = 0;
  };
  static_assert(sizeof(sectorblock_t::b) == BLOCKS_XZ*BLOCKS_XZ*BLOCKS_Y* sizeof(Block),
                "The sectorblock array must be the size of an entire sector");
}
