#ifndef DRAWQ_HPP
#define DRAWQ_HPP

#include "renderconst.hpp"
#include <array>
#include <vector>

namespace library {
	class Frustum;
}

namespace cppcraft
{
	class Column;

	class DrawQueue
	{
	private:
    typedef std::vector<Column*> drawq_shaderline_t;
		std::array<drawq_shaderline_t, RenderConst::MAX_UNIQUE_SHADERS> lines;
		bool above;

	public:
		DrawQueue(bool aboveWaterOnly) : above(aboveWaterOnly) {}

		// initialize drawing queue
		void init();
		// reset drawing queue (all shaderlines)
		void reset()
    {
      for (auto& vec : lines) vec.clear();
    }

		auto& operator[] (int shader) {
			return lines.at(shader);
		}

		int size() const
		{
			size_t cnt = 0;
			for (auto& vec : lines) cnt += vec.size();
			return cnt;
		}

		/// octtree-like frustum culling ///
		// building a new draw queue, but ignoring columns that are too far out
		// the draw queue deals only with columns
		static const int VISIBILITY_BORDER = 4;

		struct rendergrid_t
		{
			int xstp, ystp, zstp;
			int majority;
			int gridSize;
			float playerY;
      int wdx, wdz;
			const library::Frustum* frustum;
		};

		void uniformGrid(rendergrid_t& rg, int x0, int x1, int z0, int z1, int quant);
		void gridTesting(rendergrid_t& rg, int x, int z, int axis);
	};
	extern DrawQueue drawq;
	extern DrawQueue reflectionq;
}

#endif
