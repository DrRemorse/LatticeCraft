#pragma once
#include <block.hpp>
#include <db/blockdata.hpp>

namespace terragen
{
	using cppcraft::block_t;
	using cppcraft::Block;

	extern void init_blocks();

  // the default blocks
  extern block_t BEDROCK;
  extern block_t STONE_BLOCK;
  extern block_t SOIL_BLOCK;
  extern block_t BEACH_BLOCK;
  extern block_t WATER_BLOCK;
  extern block_t MOLTEN_BLOCK;
  extern block_t LAVA_BLOCK;
}
