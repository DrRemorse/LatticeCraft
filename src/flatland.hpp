#ifndef FLATLAND_HPP
#define FLATLAND_HPP

#include "common.hpp"
#include <cassert>
#include <cstdint>
#include <vector>

namespace cppcraft
{
	class Flatland {
	public:
		static const int FLATCOLORS = 8;
		typedef uint32_t color_t;

		struct flatland_t
		{
			std::array<color_t, FLATCOLORS> fcolor;
			int16_t terrain;
			int16_t skyLevel;
			int16_t groundLevel;
		};
    typedef std::vector<flatland_t> data_array_t;

    struct caveland_t
    {
      std::array<uint8_t, BLOCKS_Y / 4 + 1> underworld;
    };
    typedef std::vector<caveland_t> cave_array_t;

		// returns a reference to flatland_t for the 2D location (x, z)
		const flatland_t& operator() (int x, int z) const {
			return m_data.at(x * BLOCKS_XZ + z);
		}
		flatland_t& operator() (int x, int z) {
			return m_data.at(x * BLOCKS_XZ + z);
		}

    // a reduced-size array containing 4x4 less data
    const caveland_t& cave(int x, int z) const {
			return m_cave.at(x / CAVE_GRID2D * CAVE_GRID2D + z / CAVE_GRID2D);
		}
    caveland_t& cave(int x, int z) {
			return m_cave.at(x / CAVE_GRID2D * CAVE_GRID2D + z / CAVE_GRID2D);
		}

    void assign_new()
    {
      m_data = data_array_t(BLOCKS_XZ * BLOCKS_XZ);
      m_cave = cave_array_t(CAVE_GRID2D * CAVE_GRID2D);
    }
		// assigns new data from some source, eg. a terrain generator
    typedef std::pair<data_array_t, cave_array_t> assign_pair;
		void assign(assign_pair new_data)
		{
			m_data = std::move(new_data.first);
      m_cave = std::move(new_data.second);
		}
		// unassigns the current data, and returns it
		assign_pair unassign() {
			return { std::move(m_data), std::move(m_cave) };
		}

	private:
		data_array_t m_data;
    cave_array_t m_cave;

	public:
		// the (decompressed) file record size of a flatland-sector
		static const int FLATLAND_SIZE =
        BLOCKS_XZ * BLOCKS_XZ * sizeof(flatland_t)
        + 4 * 4 * sizeof(cave_array_t);
	};
}

#endif
