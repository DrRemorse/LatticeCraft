#ifndef PRECOMPILER_HPP
#define PRECOMPILER_HPP

#define USE_BIOMEDATA

#include "blocks_bordered.hpp"
#include "renderconst.hpp"
#include "vertex_block.hpp"
#include <deque>
#include <vector>

namespace cppcraft
{
	class Sector;

	class alignas(32) Precomp {
	public:
		/// this constructor MUST be called from main world thread
		explicit Precomp(Sector& sect)
  		: sector(sect) {}

		// our source sector (with additional data)
		bordered_sector_t sector;
    // absolute world position
    int getWX() const noexcept { return sector.wx; };
    int getWZ() const noexcept { return sector.wz; };
		// resulting mesh data
		std::vector<vertex_t> datadump;
    // total amount of vertices for each shader line
    // DOES NOT EQUAL the size of the vertex datadump
    // due to terrain optimization stages
    uint32_t vertices    [RenderConst::MAX_UNIQUE_SHADERS];
    uint32_t bufferoffset[RenderConst::MAX_UNIQUE_SHADERS];
		// resulting index data
		//indice_t* indidump;
		//indice_t indices           [RenderConst::MAX_UNIQUE_SHADERS];
	};

}

#endif
