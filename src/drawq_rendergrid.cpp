#include "drawq.hpp"

#include <library/log.hpp>
#include <library/math/frustum.hpp>
#include <library/opengl/opengl.hpp>
#include "camera.hpp"
#include "columns.hpp"
#include "player.hpp"
#include "sectors.hpp"

using namespace library;

namespace cppcraft
{
	// forward
	void gridTesting(DrawQueue::rendergrid_t& rg, int x, int z, int axis);

	void DrawQueue::uniformGrid(rendergrid_t& rg, int x0, int x1, int z0, int z1, int quant)
	{
		const int  stepsize = rg.gridSize >> quant;
		const bool gridtest = (stepsize <= 2);

		const int bigstpx = rg.xstp * stepsize;
		const int bigstpz = rg.zstp * stepsize;

		// signed step
		const int sizex = bigstpx * 16;
		const int sizez = bigstpz * 16;
		// half size of this chunk
		// FIXME: "stepsize + 1" should be "stepsize," but that causes frustum test misses
		const float size = (stepsize+1) * 16;

		int countX = (x1 - x0) / bigstpx + 1;
		int countZ = (z1 - z0) / bigstpz + 1;

		if (rg.majority < 2)
		{
			for (int x = x0; countX-- != 0; x += bigstpx)
			{
				int x2 = x * Sector::BLOCKS_XZ + sizex;
				int cz = countZ;

				for (int z = z0; cz-- != 0; z += bigstpz)
				{
					if (rg.frustum->column(x2, z * Sector::BLOCKS_XZ + sizez, 0, Sector::BLOCKS_Y, size))
					{
						if (gridtest)
							gridTesting(rg, x, z, stepsize);
						else
							uniformGrid(rg, x, x + bigstpx - rg.xstp, z, z + bigstpz - rg.zstp, quant + 1);
					}
				}
			}
		}
		else
		{
			for (int z = z0; countZ-- != 0; z += bigstpz)
			{
				int z2 = z * Sector::BLOCKS_XZ + sizez;
				int cx = countX;

				for (int x = x0; cx-- != 0; x += bigstpx)
				{
					if (rg.frustum->column(x * Sector::BLOCKS_XZ + sizex, z2, 0, Sector::BLOCKS_Y, size))
					{
            if (gridtest)
							gridTesting(rg, x, z, stepsize);
						else
							uniformGrid(rg, x, x + bigstpx - rg.xstp, z, z + bigstpz - rg.zstp, quant + 1);
					}
				}
			}
		}

	}

	void DrawQueue::gridTesting(rendergrid_t& rg, int x, int z, int axis)
	{
		int x0 = x, x1;
		int z0 = z, z1;

		if (rg.xstp == 1)
		{
			if (x0 < VISIBILITY_BORDER) x0 = VISIBILITY_BORDER;

			x1 = x + (axis - 1);
			if (x1 >= sectors.getXZ() - VISIBILITY_BORDER)
				x1 = sectors.getXZ()-1 - VISIBILITY_BORDER;

			if (x0 >= sectors.getXZ() - VISIBILITY_BORDER || x0 > x1) return;
		}
		else
		{
			if (x0 >= sectors.getXZ() - VISIBILITY_BORDER)
				x0 = sectors.getXZ()-1 - VISIBILITY_BORDER;

			x1 = x - (axis - 1);
			if (x1 < VISIBILITY_BORDER) x1 = VISIBILITY_BORDER;

			if (x1 >= sectors.getXZ() - VISIBILITY_BORDER || x1 > x0) return;
		}

		if (rg.zstp == 1)
		{
			if (z0 < VISIBILITY_BORDER) z0 = VISIBILITY_BORDER;

			z1 = z + (axis - 1);
			if (z1 >= sectors.getXZ() - VISIBILITY_BORDER)
				z1 = sectors.getXZ()-1 - VISIBILITY_BORDER;

			if (z0 >= sectors.getXZ() - VISIBILITY_BORDER || z0 > z1) return;
		}
		else
		{
			if (z0 >= sectors.getXZ() - VISIBILITY_BORDER)
				z0 = sectors.getXZ()-1 - VISIBILITY_BORDER;

			z1 = z - (axis - 1);
			if (z1 < VISIBILITY_BORDER) z1 = VISIBILITY_BORDER;

			if (z1 >= sectors.getXZ() - VISIBILITY_BORDER || z1 > z0) return;
		}

		x = x0; z = z0;
		const float MAX_GRIDRAD =
        (camera.cameraViewSectors+1) * (camera.cameraViewSectors+1);

    // center measured in sectors
    const auto center_grid = glm::vec2(player.pos.x, player.pos.z) / float(BLOCKS_XZ);

		while (true)
		{
			auto& cv = columns(x, z, rg.wdx, rg.wdz);
			if (cv.renderable)
			{
        const float fx = (x + 0.5f) - center_grid.x;
    		const float fz = (z + 0.5f) - center_grid.y;
				if (fx*fx + fz*fz < MAX_GRIDRAD)
				{
					static const float gs_half = BLOCKS_XZ / 2;

					if (rg.frustum->column(
							(x * BLOCKS_XZ) + gs_half,
							(z * BLOCKS_XZ) + gs_half,
							0, BLOCKS_Y,  gs_half))
					{
						// this column is likely visible, and so we add all its meshes to queue
						cv.pos.x = x * BLOCKS_XZ;
						cv.pos.z = z * BLOCKS_XZ;

						for (size_t i = 0; i < lines.size(); i++)
						{
							if (cv.vertices[i] != 0)
							{
								// add to draw queue
                lines[i].push_back(&cv);
							}

						} // while i < shaders

					} // frustum test

				} // radial test
			}
			else if (cv.hasdata) // not renderable, but has VAO
			{
				// DONT DISABLE THIS, GPU WILL RUN OUT OF MEMORY IN A HEARTBEAT!!!!!!!!
				cv.hasdata = false;
				// free data from VAO/VBO (GPU is going to re-use this eventually)
				glBindVertexArray(cv.vao);
				glBindBuffer(GL_ARRAY_BUFFER, cv.vbo);
				glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_STATIC_DRAW);
			} // render test

			if (rg.majority < 2)
			{
				// Y -> Z -> X
				if (z == z1)
				{
					if (x == x1) return;

					x += rg.xstp;
					z = z0;
				}
				else z += rg.zstp;
			}
			else
			{
				// Y -> X -> Z
				if (x == x1)
				{
					if (z == z1) return;

					z += rg.zstp;
					x = x0;
				}
				else x += rg.xstp;
			} // majority

		} // do loop

	} // gridTesting()

}
