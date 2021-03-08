#pragma once

#include "../blocks.hpp"
namespace terragen
{
	using cppcraft::block_t;
	using cppcraft::Block;

	extern int ofacing(int dx, int dz);
	extern int ofacingNeg(int dx, int dz);

	extern bool coretest(int x, int y, int z, int ground_rad, int air_rad, int height);
	extern void downSpider(int x, int y, int z, block_t id, int tries);

	extern void ocircleXZ(int x, int y, int z, int radius, Block);
	extern void ocircleXZstencil(int x, int y, int z, int rad, Block, float chance);
	extern void ocircleXZroots(int x, int y, int z, int radius, Block);
	extern void oellipsoidXZ(int x, int y, int z, int radius, float radx, float radz, Block);
	extern void oellipsoidXY(int x, int y, int z, int radius, float radx, float rady, float stencil, Block);

	extern void obell(int x, int y, int z, Block, int lower, int height, int radius, int inner_rad, int midlevel, float midstrength, float understrength, float stencilchance);
}
