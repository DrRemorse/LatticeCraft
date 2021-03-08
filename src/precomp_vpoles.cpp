#include "precomp_thread_data.hpp"

#include "blockmodels.hpp"

namespace cppcraft
{
	/*
	int PrecompThreadData::emitPole(block_t id, int bx, int by, int bz, block_t model)
	{
		int vertices = blockmodels.poles.copyTo(model, indic);
		
		applyFaceLightingAll(bx, by, bz);
		
		short tid = Block::cubeFaceById(id, 0, 0);
		
		for (int i = 0; i < vertices; i++)
		{
			indic[i].w = tid;
		}
		
		return vertices;
	}*/
}
