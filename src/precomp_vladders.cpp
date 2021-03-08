#include "precomp_thread_data.hpp"

#include "blockmodels.hpp"

namespace cppcraft
{
	/*
	int emitLadder(block_t id, int bx, int by, int bz, block_t block_facing)
	{
		blockmodels.ladders.copyTo(block_facing, indic);
		
		switch (block_facing)
		{
		case 0:
			applyFaceLighting_PZ(bx, by, bz);
			indic[0].biome = fbiome[0]; // (0, 0)
			indic[1].biome = fbiome[1]; // (1, 0)
			indic[2].biome = fbiome[1]; // (1, 0)
			indic[3].biome = fbiome[0]; // (0, 0)
			for (int i = 0; i < 4; i++)
			{
				indic[i].u += bx * RenderConst::VERTEX_SCALE;
				indic[i].v += by * RenderConst::VERTEX_SCALE;
			}
			break;
		case 1:
			applyFaceLighting_NZ(bx, by, bz);
			indic[0].biome = fbiome[2]; // (0, 1)
			indic[1].biome = fbiome[2]; //
			indic[2].biome = fbiome[3]; // (1, 1)
			indic[3].biome = fbiome[3]; //
			for (int i = 0; i < 4; i++)
			{
				indic[i].u += bx * RenderConst::VERTEX_SCALE;
				indic[i].v += by * RenderConst::VERTEX_SCALE;
			}
			break;
		case 2:
			applyFaceLighting_PX(bx, by, bz);
			indic[0].biome = fbiome[0]; // (0, 0)
			indic[1].biome = fbiome[0]; //
			indic[2].biome = fbiome[2]; // (0, 1)
			indic[3].biome = fbiome[2]; //
			for (int i = 0; i < 4; i++)
			{
				indic[i].u += bz * RenderConst::VERTEX_SCALE;
				indic[i].v += by * RenderConst::VERTEX_SCALE;
			}
			break;
		case 3:
			applyFaceLighting_NX(bx, by, bz);
			indic[0].biome = fbiome[1]; // (1, 0)
			indic[1].biome = fbiome[3]; // (1, 1)
			indic[2].biome = fbiome[3]; // (1, 1)
			indic[3].biome = fbiome[1]; // (1, 0)
			for (int i = 0; i < 4; i++)
			{
				indic[i].u += bz * RenderConst::VERTEX_SCALE;
				indic[i].v += by * RenderConst::VERTEX_SCALE;
			}
			break;
		}
		
		indic[0].w = Block::cubeFaceById(id, 0, 0);
		indic[1].w = indic[0].w;
		indic[2].w = indic[0].w;
		indic[3].w = indic[0].w;
		
		return 4; // 4 vertices
	}*/
}
