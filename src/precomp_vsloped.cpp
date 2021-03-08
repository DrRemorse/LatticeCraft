#include "precomp_thread_data.hpp"

#include "blockmodels.hpp"

namespace cppcraft
{
	/*
	int emitSloped(block_t id, int bx, int by, int bz, block_t model, block_t facing)
	{
		int vertices = 0;
		
		if (model != 0 && model != 1)
		{
			if (facing & 1)
			{
				vertices += blockmodels.slopedLeafs[model].copyTo(0, indic);
				applyFaceLighting_PZ(bx, by, bz+1);
				indic += 4;
			}
			if (facing & 2)
			{
				vertices += blockmodels.slopedLeafs[model].copyTo(1, indic);
				applyFaceLighting_NZ(bx, by, bz-1);
				indic += 4;
			}
		}
		
		vertices += blockmodels.slopedLeafs[model].copyTo(2, indic);
		applyFaceLighting_PY(bx, by+1, bz);
		indic += 4;
		// negative-Y
		if (facing & 8)
		{
			vertices += blockmodels.slopedLeafs[model].copyTo(3, indic);
			applyFaceLighting_NY(bx, by-1, bz);
			indic += 4;
		}
		
		if (model != 2 && model != 3)
		{
			if (facing & 16)
			{
				vertices += blockmodels.slopedLeafs[model].copyTo(4, indic);
				applyFaceLighting_PX(bx+1, by, bz);
				indic += 4;
			}
			if (facing & 32)
			{
				vertices += blockmodels.slopedLeafs[model].copyTo(5, indic);
				applyFaceLighting_NX(bx-1, by, bz);
				indic += 4;
			}
		}
		
		indic -= vertices; // go back to start
		
		short tid = Block::cubeFaceById(id, 0, 0);
		
		for (int i = 0; i < vertices; i++)
		{
			indic[i].w = tid;
			indic[i].biome = fbiome[0];
		}
		
		return vertices;
	}*/
}
