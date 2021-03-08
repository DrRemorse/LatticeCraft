#ifndef COLUMNS_HPP
#define COLUMNS_HPP

#include "renderconst.hpp"
#include "sectors.hpp"
#include "vertex_block.hpp"
#include "world.hpp"
#include <glm/vec3.hpp>
#include <array>
#include <string>

namespace cppcraft
{
	class Precomp;

	class Column {
	public:
		// position below Y-position when column first appearing in world
		static const float COLUMN_DEPRESSION;

		Column();

		// used by: Seamless
		void reset() {
      this->renderable = false;
    }
		// used by: Compiler pipeline
		void compile(int x, int y, int z, Precomp* pc);
    // column unique index
    int index() const noexcept { return m_idx; }

		// flags
		bool renderable = false; // is renderable
		bool hasdata = false;    // has uploaded gpu data
		bool aboveWater = true;  // is above the waterline (reflectable)

		unsigned int  vao; // vertex array object
		unsigned int  vbo; // vertex buffer
		unsigned int  ibo; // index buffer

		glm::vec3 pos; // rendering position

		//int indices    [RenderConst::MAX_UNIQUE_SHADERS];
		//int indexoffset[RenderConst::MAX_UNIQUE_SHADERS];

		uint32_t bufferoffset[RenderConst::MAX_UNIQUE_SHADERS];
		uint32_t vertices    [RenderConst::MAX_UNIQUE_SHADERS];

  private:
    int m_idx = 0;
    static int m_current_idx;
	};

	class Columns {
	public:
    // number of columns on Y-axis
    static const int HEIGHT = 1;
		void init();

		inline int getHeight() const
		{
			return HEIGHT;
		}

		// column index operator
		Column& operator() (int x, int z, int wdx, int wdz)
		{
			x = (x + wdx) % sectors.getXZ();
			z = (z + wdz) % sectors.getXZ();

			return columns.at(x * sectors.getXZ() + z);
		}

    size_t size() const noexcept {
      return columns.size();
    }

	private:
		std::vector<Column> columns;
  };
	extern Columns columns;

}

#endif
