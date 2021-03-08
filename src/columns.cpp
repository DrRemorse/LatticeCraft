#include "columns.hpp"

#include <library/log.hpp>
#include <library/opengl/opengl.hpp>
#include "camera.hpp"
#include "renderconst.hpp"
#include "precompiler.hpp"
#include "vertex_block.hpp"
#include <cstring>

using namespace library;

namespace cppcraft
{
	const float Column::COLUMN_DEPRESSION = 16.0f;
  int Column::m_current_idx = 0;

	// all the columns you'll ever need
	Columns columns;

	void Columns::init()
	{
		logger << Log::INFO << "* Initializing columns" << Log::ENDL;

		//////////////////////
		// allocate columns //
		//////////////////////
		const int num_columns = sectors.getXZ() * sectors.getXZ() * getHeight();
		this->columns.resize(num_columns);
	}

	Column::Column()
	{
		// initialize VAO to 0, signifying a column without valid GL resources
		this->vao = 0;
		// set initial flags
		this->renderable = false;
		this->hasdata = false;
    // generate new index for this column
    this->m_idx = m_current_idx++;
	}

	void Column::compile(int x, int y, int z, Precomp* pc)
	{
		///////////////////////////////////
		// generate resources for column //
		///////////////////////////////////

		(void) y;
		bool updateAttribs = false;

		if (this->vao == 0)
		{
			// vertex array object
			glGenVertexArrays(1, &this->vao);
			// vertex and index buffer object
			glGenBuffers(1, &this->vbo);
			updateAttribs = true;
		}

		for (int n = 0; n < RenderConst::MAX_UNIQUE_SHADERS; n++)
		{
			//this->indices[n]     = pc->indices[n];
			//this->indexoffset[n] = indices;
			this->vertices[n]     = pc->vertices[n];
			this->bufferoffset[n] = pc->bufferoffset[n];
		}

    // set each vertex to the columns unique ID
    for (auto& vtx : pc->datadump) {
      vtx.face = this->m_idx;
    }

		// bind vao
		glBindVertexArray(this->vao);

		// bind vbo and upload vertex data
		glBindBuffer(GL_ARRAY_BUFFER, this->vbo);
		//glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_STATIC_DRAW);
		glBufferData(GL_ARRAY_BUFFER,
                 pc->datadump.size() * sizeof(vertex_t), /* DONT COUNT TOTAL! */
                 pc->datadump.data(),
                 GL_STATIC_DRAW);

		// bind ibo and upload index data
		//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->ibo);
		//glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices * sizeof(indice_t), pc->indidump, GL_STATIC_DRAW);

		if (updateAttribs)
		{
		// attribute pointers
		glVertexAttribPointer(0, 4, GL_SHORT,		  GL_FALSE, sizeof(vertex_t), (GLvoid*) offsetof(vertex_t, x)); // vertex
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(1, 4, GL_BYTE,		  GL_TRUE,  sizeof(vertex_t), (GLvoid*) offsetof(vertex_t, nx)); // normal
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(2, 4, GL_SHORT,		  GL_FALSE, sizeof(vertex_t), (GLvoid*) offsetof(vertex_t, u)); // texture
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(3, 4, GL_UNSIGNED_BYTE, GL_TRUE,  sizeof(vertex_t), (GLvoid*) offsetof(vertex_t, color)); // biome color
		glEnableVertexAttribArray(3);
    glVertexAttribPointer(4, 4, GL_UNSIGNED_BYTE, GL_TRUE,  sizeof(vertex_t), (GLvoid*) offsetof(vertex_t, data1));
		glEnableVertexAttribArray(4);
    glVertexAttribPointer(5, 4, GL_UNSIGNED_BYTE, GL_TRUE,  sizeof(vertex_t), (GLvoid*) offsetof(vertex_t, data2));
		glEnableVertexAttribArray(5);
		}

#ifdef OPENGL_DO_CHECKS
		if (OpenGL::checkError())
		{
			logger << Log::ERR << "Column::upload(): OpenGL error. Line: " << __LINE__ << Log::ENDL;
			throw std::string("Column::upload(): OpenGL state error");
		}
#endif

		if (camera.getFrustum().column(x * BLOCKS_XZ + BLOCKS_XZ / 2,
									   z * BLOCKS_XZ + BLOCKS_XZ / 2,
									   0,  BLOCKS_Y,   BLOCKS_XZ / 2))
		{
			// update render list
			camera.needsupd = true;

			if (this->renderable == false)
			{
				// make sure it is added to renderq
				camera.recalc = true;
				this->pos.y = -COLUMN_DEPRESSION;
			}
		}

		// set as renderable,
		this->renderable = true;
		// the vbo has data stored in gpu
		this->hasdata = true;
	}

}
