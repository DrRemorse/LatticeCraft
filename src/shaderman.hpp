#ifndef SHADERMAN_HPP
#define SHADERMAN_HPP

#include <library/opengl/shader.hpp>
#include "renderman.hpp"
#include <array>
#include <string>

namespace library
{
	class mat4;
	class WindowClass;
}

namespace cppcraft
{
	class Shaderman
	{
	public:
		void init(Renderer& renderer);

		typedef enum
		{
			ATMOSPHERE,
			CLOUDS,
			SUN, SUNPROJ,
			MOON,

			STD_BLOCKS,      // 1
			CULLED_BLOCKS,   // 2
			ALPHA_BLOCKS,    // 3
			BLOCKS_WATER,    // 4
			BLOCKS_RUNWATER, // 5
			BLOCKS_LAVA,     // 6
			BLOCKS_REFLECT,  // 7
			BLOCKS_DEPTH,    // 8

			FSTERRAINFOG,
			FSTERRAIN,

			OBJECT,
			VOXELOBJECT,
			ACTOR,
			PARTICLE,

			SELECTION,
			SELECTION_MINING,

			PLAYERHAND,
			PHAND_HELDITEM,
			VOXEL,
			PLAYERMODEL,

			GAUSS,
			BLUR,
			SUPERSAMPLING,
			LENSFLARE,
			POSTPROCESS,

			COMPASS,
			MINIMAP,
			GUI,
			GUI_COLOR,

			NUMBER_OF_SHADERS

		} shaderlist_t;

		auto& operator[] (shaderlist_t shd) {
      return shaders[shd];
    }

	private:
		std::array<library::Shader, NUMBER_OF_SHADERS> shaders;
	};
	extern Shaderman shaderman;
}

#endif
