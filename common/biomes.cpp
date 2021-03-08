#include "biomes.hpp"

#include <library/bitmap/colortools.hpp>

using namespace library;

namespace cppcraft
{
	static uint8_t colors16[16][3] =
	{
		{ 20,  12,   28 },
		{ 68,  36,   52 },
		{ 48,  52,  109 },
		{ 78,  74,   78 },
		{ 133, 76,   48 },
		{ 52,  101,  36 },
		{ 208, 70,   72 },
		{ 117, 113,  97 },
		{ 89,  125, 206 },
		{ 210, 125,  44 },
		{ 133, 149, 161 },
		{ 109, 170,  44 },
		{ 210, 170, 153 },
		{ 109, 194, 202 },
		{ 218, 212,  94 },
		{ 222, 238, 214 }
	};

	Biomes::color_t Biomes::getSpecialColorBGRA(int clid)
	{
		return BGRA8(colors16[clid][0], colors16[clid][1], colors16[clid][2], 255);
	}

	Biomes::color_t Biomes::getSpecialColorRGBA(int clid)
	{
		return RGBA8(colors16[clid][0], colors16[clid][1], colors16[clid][2], 255);
	}

}
