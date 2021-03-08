#pragma once

#include <cstdint>
#define OPENGL_DO_CHECKS

namespace cppcraft
{
	static const int BLOCKS_XZ  =   16;
  static const int CAVE_GRID2D =   4;
	static const int BLOCKS_Y   =  320;
	static const int WATERLEVEL =  128;
  static const float WATERLEVEL_FLT = WATERLEVEL / float(BLOCKS_Y-1);
  // we cannot build or generate blocks above this block height
  static const int TOP_BLOCK_Y = BLOCKS_Y-2;
  static const float TOP_BLOCK_FLT = TOP_BLOCK_Y / float(BLOCKS_Y);

	// scale up and down all terrains
	const float BIOME_SCALE = 0.0003f;
  const float UNDERGEN_SCALE = 0.001f;

	typedef uint32_t light_value_t;
}

extern void dump_trace();

#define LIKELY(x)    __builtin_expect(!!(x), 1)
#define UNLIKELY(x)  __builtin_expect(!!(x), 0)

#include <cassert>
#include <stdlib.h>
#include <cstdio>
#define CC_ASSERT(x, msg)            \
  if (UNLIKELY(!(x))) {              \
    fprintf(stderr, "%s() [%s:%d] %s\n", \
        __FUNCTION__, __FILE__, __LINE__, msg);\
    dump_trace(); abort(); }
