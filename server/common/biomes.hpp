#ifndef BIOME_HPP
#define BIOME_HPP

#include <cstdint>

namespace cppcraft
{
	class Biomes {
	public:
		typedef uint32_t color_t;

		// biome color ids
		enum biome_color_t
		{
			CL_STONE,
			CL_SOIL,
			CL_GRAVEL,
			CL_SAND,
			CL_GRASS,
			CL_TREES_A,
			CL_TREES_B,
			CL_WATER,

			CL_MAX
		};

		static int idToColorIndex(unsigned short id);

		// returns a 32-bit color from a fixed set of 16 colors
		static color_t getSpecialColorRGBA(int clid);
		static color_t getSpecialColorBGRA(int clid);
	};
}

#endif
