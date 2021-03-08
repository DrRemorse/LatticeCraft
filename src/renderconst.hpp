#ifndef RENDERCONST_HPP
#define RENDERCONST_HPP

#include "common.hpp"

namespace cppcraft
{
	class RenderConst
	{
	public:
		enum shaderline_t
		{
			// before "underwater" screenshot
			TX_REPEAT = 0,
			TX_SOLID,
			TX_TRANS_1SIDED,
			TX_TRANS_2SIDED,
			// after screenshot
			TX_WATER,
			TX_FLUIDS,
			TX_LAVA,

			MAX_UNIQUE_SHADERS
		};

		static const short VERTEX_SCALE = 32; // valid: 16, 32, 64
		static const short VERTEX_SHL   = 5;

		static const short WATER_VX_LEVEL = (WATERLEVEL - 1) * RenderConst::VERTEX_SCALE;

		static const int SKY_LEVEL = BLOCKS_Y - 48;
	};
}

#endif
