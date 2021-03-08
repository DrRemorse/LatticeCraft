#include "../precomp_thread_data.hpp"
#include "../blocks_bordered.hpp"
#include "../blockmodels.hpp"

namespace cppcraft
{
  static void
	emitCubeVertexPZ(PTD& ptd, const Block& block, int bx, int by, int bz)
	{
    const auto& blockdata = block.db();
		// copy cube-mesh object 0 (+z side)
		auto vtx = blockmodels.cubes[blockdata.model()].copyTo(0, ptd.current());

		if (ptd.shader == RenderConst::TX_REPEAT)
		{
			if (blockdata.repeat_y)
			{
				vtx[0].u = bx * ptd.REPEAT_FACTOR;       // 0, 0
				vtx[0].v = by * ptd.REPEAT_FACTOR;
				vtx[1].u = (1 + bx) * ptd.REPEAT_FACTOR; // 1, 0
				vtx[1].v = by * ptd.REPEAT_FACTOR;
				vtx[2].u = (1 + bx) * ptd.REPEAT_FACTOR; // 1, 1
				vtx[2].v = (1 + by) * ptd.REPEAT_FACTOR;
				vtx[3].u = bx * ptd.REPEAT_FACTOR;       // 0, 1
				vtx[3].v = (1 + by) * ptd.REPEAT_FACTOR;
			}
			else
			{
				vtx[0].u = bx * ptd.REPEAT_FACTOR;       // 0, 0
				vtx[0].v = 2 * ptd.REPEAT_FACTOR;
				vtx[1].u = (1 + bx) * ptd.REPEAT_FACTOR; // 1, 0
				vtx[1].v = 2 * ptd.REPEAT_FACTOR;
				vtx[2].u = (1 + bx) * ptd.REPEAT_FACTOR; // 1, 1
				vtx[2].v = 3 * ptd.REPEAT_FACTOR;
				vtx[3].u = bx * ptd.REPEAT_FACTOR;       // 0, 1
				vtx[3].v = 3 * ptd.REPEAT_FACTOR;
			}
		}
    vtx->w   = ptd.getConnectedTexture(block, bx, by, bz, 0);
    vtx[1].w = vtx->w;
    vtx[2].w = vtx->w;
    vtx[3].w = vtx->w;

		if (blockdata.isTerrainColored())
		{
			int index = blockdata.getColorIndex();
			vtx[0].color = ptd.getColor(bx  , bz+1, index); // 2
			vtx[1].color = ptd.getColor(bx+1, bz+1, index); // 3
			vtx[2].color = ptd.getColor(bx+1, bz+1, index); // 3
			vtx[3].color = ptd.getColor(bx  , bz+1, index); // 2
		}
		else if (blockdata.getColor != nullptr)
		{
			vtx[0].color = blockdata.getColor(block);
			vtx[3].color = vtx[2].color = vtx[1].color = vtx[0].color;
		}

    // calculate lighting
    ptd.faceLighting_PZ(block, vtx, bx, by, bz + 1);

	} // emitCubeVertexPZ()

	void emitCubeVertexNZ(PTD& ptd, const Block& block, int bx, int by, int bz)
	{
    const auto& blockdata = block.db();
		// copy cube-mesh object 1 (-z side)
		auto vtx = blockmodels.cubes[blockdata.model()].copyTo(1, ptd.current());

		if (ptd.shader == RenderConst::TX_REPEAT)
		{
			if (blockdata.repeat_y)
			{
				vtx[0].u = bx * ptd.REPEAT_FACTOR;       // 0, 0
				vtx[0].v = by * ptd.REPEAT_FACTOR;
				vtx[1].u = bx * ptd.REPEAT_FACTOR;       // 0, 1
				vtx[1].v = (1 + by) * ptd.REPEAT_FACTOR;
				vtx[2].u = (1 + bx) * ptd.REPEAT_FACTOR; // 1, 1
				vtx[2].v = (1 + by) * ptd.REPEAT_FACTOR;
				vtx[3].u = (1 + bx) * ptd.REPEAT_FACTOR; // 1, 0
				vtx[3].v = by * ptd.REPEAT_FACTOR;
			}
			else
			{
				vtx[0].u = bx * ptd.REPEAT_FACTOR;       // 0, 0
				vtx[0].v = 2 * ptd.REPEAT_FACTOR;
				vtx[1].u = bx * ptd.REPEAT_FACTOR;       // 0, 1
				vtx[1].v = 3 * ptd.REPEAT_FACTOR;
				vtx[2].u = (1 + bx) * ptd.REPEAT_FACTOR; // 1, 1
				vtx[2].v = 3 * ptd.REPEAT_FACTOR;
				vtx[3].u = (1 + bx) * ptd.REPEAT_FACTOR; // 1, 0
				vtx[3].v = 2 * ptd.REPEAT_FACTOR;
			}
		}
    vtx->w   = ptd.getConnectedTexture(block, bx, by, bz, 1);
    vtx[1].w = vtx->w;
    vtx[2].w = vtx->w;
    vtx[3].w = vtx->w;

		if (blockdata.isTerrainColored())
		{
			int index = blockdata.getColorIndex();
			vtx[0].color = ptd.getColor(bx  , bz, index); // 0
			vtx[1].color = ptd.getColor(bx  , bz, index); // 0
			vtx[2].color = ptd.getColor(bx+1, bz, index); // 1
			vtx[3].color = ptd.getColor(bx+1, bz, index); // 1
		}
    else if (blockdata.getColor != nullptr)
		{
			vtx[0].color = blockdata.getColor(block);
			vtx[3].color = vtx[2].color = vtx[1].color = vtx[0].color;
		}

    // calculate lighting
    ptd.faceLighting_NZ(block, vtx, bx, by, bz - 1);

	} // emitCubeVertexNZ()

	void emitCubeVertexPY(PTD& ptd, const Block& block, int bx, int by, int bz)
	{
		(void) by;
    const auto& blockdata = block.db();
		// 0.0, 0.0,  0.0, 1.0,  1.0, 1.0,  1.0, 0.0
		// copy cube-mesh object 2 (+y side)
		auto vtx = blockmodels.cubes[blockdata.model()].copyTo(2, ptd.current());

		if (ptd.shader == RenderConst::TX_REPEAT)
		{
			vtx[0].u = bx * ptd.REPEAT_FACTOR; // 0, 0
			vtx[0].v = bz * ptd.REPEAT_FACTOR;

			vtx[1].u = bx * ptd.REPEAT_FACTOR; // 0, 1
			vtx[1].v = (1 + bz) * ptd.REPEAT_FACTOR;

			vtx[2].u = (1 + bx) * ptd.REPEAT_FACTOR; // 1, 1
			vtx[2].v = (1 + bz) * ptd.REPEAT_FACTOR;

			vtx[3].u = (1 + bx) * ptd.REPEAT_FACTOR; // 1, 0
			vtx[3].v = bz * ptd.REPEAT_FACTOR;
		}
    vtx->w   = ptd.getConnectedTexture(block, bx, by, bz, 2);
    vtx[1].w = vtx[0].w;
    vtx[2].w = vtx[0].w;
    vtx[3].w = vtx[0].w;

		if (blockdata.isTerrainColored())
		{
			int index = blockdata.getColorIndex();
			vtx[0].color = ptd.getColor(bx  , bz  , index); // 0
			vtx[1].color = ptd.getColor(bx  , bz+1, index); // 2
			vtx[2].color = ptd.getColor(bx+1, bz+1, index); // 3
			vtx[3].color = ptd.getColor(bx+1, bz  , index); // 1
		}
    else if (blockdata.getColor != nullptr)
		{
			vtx[0].color = blockdata.getColor(block);
			vtx[3].color = vtx[2].color = vtx[1].color = vtx[0].color;
		}

    // calculate lighting
    ptd.faceLighting_PY(block, vtx, bx, by + 1, bz);

	} // emitBlockvertexPY()

	void emitCubeVertexNY(PTD& ptd, const Block& block, int bx, int by, int bz)
	{
		(void) by;
    const auto& blockdata = block.db();
		// copy cube-mesh object 3 (-y side)
		auto vtx = blockmodels.cubes[blockdata.model()].copyTo(3, ptd.current());

		if (ptd.shader == RenderConst::TX_REPEAT)
		{
			// {1,1,  0,1,  0,0,  1,0}
			vtx[0].u = bx * ptd.REPEAT_FACTOR; // 0, 0
			vtx[0].v = bz * ptd.REPEAT_FACTOR;

			vtx[1].u = (1 + bx) * ptd.REPEAT_FACTOR;       // 1, 0
			vtx[1].v = bz * ptd.REPEAT_FACTOR;

			vtx[2].u = (1 + bx) * ptd.REPEAT_FACTOR;       // 1, 1
			vtx[2].v = (1 + bz) * ptd.REPEAT_FACTOR;

			vtx[3].u = bx * ptd.REPEAT_FACTOR; // 0, 1
			vtx[3].v = (1 + bz) * ptd.REPEAT_FACTOR;
		}
    vtx->w   = ptd.getConnectedTexture(block, bx, by, bz, 3);
    vtx[1].w = vtx->w;
    vtx[2].w = vtx->w;
    vtx[3].w = vtx->w;

		if (blockdata.isTerrainColored())
		{
			int index = blockdata.getColorIndex();
			vtx[0].color = ptd.getColor(bx  , bz  , index); // 0
			vtx[1].color = ptd.getColor(bx+1, bz  , index); // 1
			vtx[2].color = ptd.getColor(bx+1, bz+1, index); // 3
			vtx[3].color = ptd.getColor(bx  , bz+1, index); // 2
		}
    else if (blockdata.getColor != nullptr)
		{
			vtx[0].color = blockdata.getColor(block);
			vtx[3].color = vtx[2].color = vtx[1].color = vtx[0].color;
		}

    // calculate lighting
    ptd.faceLighting_NY(block, vtx, bx, by - 1, bz);

	} // emitCubeVertexPY()

	void emitCubeVertexPX(PTD& ptd, const Block& block, int bx, int by, int bz)
	{
    const auto& blockdata = block.db();
		// copy cube-mesh object 4 (+x side)
		auto vtx = blockmodels.cubes[blockdata.model()].copyTo(4, ptd.current());

		if (ptd.shader == RenderConst::TX_REPEAT)
		{
			if (blockdata.repeat_y)
			{
				// V: 0.0, 0.0,  1.0, 0.0,  1.0, 1.0,  0.0, 1.0
				// T: 1.0, 0.0,  1.0, 1.0,  0.0, 1.0,  0.0, 0.0

				vtx[0].u = (bz - 1) * ptd.REPEAT_FACTOR; // 1, 0
				vtx[0].v = (by - 1) * ptd.REPEAT_FACTOR;
				vtx[1].u = (bz - 1) * ptd.REPEAT_FACTOR; // 1, 1
				vtx[1].v =  by      * ptd.REPEAT_FACTOR;
				vtx[2].u = (bz + 0) * ptd.REPEAT_FACTOR; // 0, 1
				vtx[2].v =  by      * ptd.REPEAT_FACTOR;
				vtx[3].u = (bz + 0) * ptd.REPEAT_FACTOR; // 0, 0
				vtx[3].v = (by - 1) * ptd.REPEAT_FACTOR;
			}
			else
			{
				vtx[0].u = (bz - 1) * ptd.REPEAT_FACTOR; // 1, 0
				vtx[0].v = 2  * ptd.REPEAT_FACTOR;
				vtx[1].u = (bz - 1) * ptd.REPEAT_FACTOR; // 1, 1
				vtx[1].v = 3  * ptd.REPEAT_FACTOR;
				vtx[2].u = bz * ptd.REPEAT_FACTOR;       // 0, 1
				vtx[2].v = 3  * ptd.REPEAT_FACTOR;
				vtx[3].u = bz * ptd.REPEAT_FACTOR;       // 0, 0
				vtx[3].v = 2  * ptd.REPEAT_FACTOR;
			}
		}
    vtx->w   = ptd.getConnectedTexture(block, bx, by, bz, 4);
    vtx[1].w = vtx->w;
    vtx[2].w = vtx->w;
    vtx[3].w = vtx->w;

		if (blockdata.isTerrainColored())
		{
			int index = blockdata.getColorIndex();
			vtx[0].color = ptd.getColor(bx+1, bz  , index); // 1
			vtx[1].color = ptd.getColor(bx+1, bz  , index); // 1
			vtx[2].color = ptd.getColor(bx+1, bz+1, index); // 3
			vtx[3].color = ptd.getColor(bx+1, bz+1, index); // 3
		}
    else if (blockdata.getColor != nullptr)
		{
			vtx[0].color = blockdata.getColor(block);
			vtx[3].color = vtx[2].color = vtx[1].color = vtx[0].color;
		}

    // calculate lighting
    ptd.faceLighting_PX(block, vtx, bx + 1, by, bz);

	} // exitBlockVertexPX()

	void emitCubeVertexNX(PTD& ptd, const Block& block, int bx, int by, int bz)
	{
    const auto& blockdata = block.db();
		// copy cube-mesh object 5 (+x side)
		auto vtx = blockmodels.cubes[blockdata.model()].copyTo(5, ptd.current());

		if (ptd.shader == RenderConst::TX_REPEAT)
		{
			if (blockdata.repeat_y)
			{
				vtx[0].u = bz * ptd.REPEAT_FACTOR;       // 0, 0
				vtx[0].v = by * ptd.REPEAT_FACTOR;
				vtx[1].u = (1 + bz) * ptd.REPEAT_FACTOR; // 1, 0
				vtx[1].v = by * ptd.REPEAT_FACTOR;
				vtx[2].u = (1 + bz) * ptd.REPEAT_FACTOR; // 1, 1
				vtx[2].v = (1 + by) * ptd.REPEAT_FACTOR;
				vtx[3].u = bz * ptd.REPEAT_FACTOR;       // 0, 1
				vtx[3].v = (1 + by) * ptd.REPEAT_FACTOR;
			}
			else
			{
				vtx[0].u = bz * ptd.REPEAT_FACTOR;       // 0, 0
				vtx[0].v = 2 * ptd.REPEAT_FACTOR;
				vtx[1].u = (1 + bz) * ptd.REPEAT_FACTOR; // 1, 0
				vtx[1].v = 2 * ptd.REPEAT_FACTOR;
				vtx[2].u = (1 + bz) * ptd.REPEAT_FACTOR; // 1, 1
				vtx[2].v = 3 * ptd.REPEAT_FACTOR;
				vtx[3].u = bz * ptd.REPEAT_FACTOR;       // 0, 1
				vtx[3].v = 3 * ptd.REPEAT_FACTOR;
			}
		}
    vtx->w   = ptd.getConnectedTexture(block, bx, by, bz, 5);
    vtx[1].w = vtx->w;
    vtx[2].w = vtx->w;
    vtx[3].w = vtx->w;

		if (blockdata.isTerrainColored())
		{
			int index = blockdata.getColorIndex();
			vtx[0].color = ptd.getColor(bx, bz  , index); // 0
			vtx[1].color = ptd.getColor(bx, bz+1, index); // 2
			vtx[2].color = ptd.getColor(bx, bz+1, index); // 2
			vtx[3].color = ptd.getColor(bx, bz  , index); // 0
		}
    else if (blockdata.getColor != nullptr)
		{
			vtx[0].color = blockdata.getColor(block);
			vtx[3].color = vtx[2].color = vtx[1].color = vtx[0].color;
		}

    // calculate lighting
    ptd.faceLighting_NX(block, vtx, bx - 1, by, bz);

	} // emitCubeVertexNX()

	void emitCube(PTD& ptd, int bx, int by, int bz, block_t facing)
	{
		///////////////////////////////////////////////
		////  loop through of all sides on a cube  ////
		////   ----   ----   ----   ----   ----    ////
		////  emit vertices & calculate lighting   ////
		///////////////////////////////////////////////
		const Block& block = ptd.sector->get(bx, by, bz);

		if (facing & 1) {  // +z
					// emit vertex PZ
					emitCubeVertexPZ(ptd, block, bx, by, bz);
    }
    if (facing & 2) {  // -z
					// emit vertex NZ
					emitCubeVertexNZ(ptd, block, bx, by, bz);
    }
		if (facing & 4) {  // +y
					// emit vertex PY
					emitCubeVertexPY(ptd, block, bx, by, bz);
    }
    if (facing & 8) {  // -y
					// emit vertex_NY
					emitCubeVertexNY(ptd, block, bx, by, bz);
    }
    if (facing & 16) {  // +x
					// emit vertex PX
					emitCubeVertexPX(ptd, block, bx, by, bz);
    }
    if (facing & 32) {  // -x
					// emit vertex NX
					emitCubeVertexNX(ptd, block, bx, by, bz);
    }

	} // emitCube()

}
