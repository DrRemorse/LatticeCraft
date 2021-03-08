#include "precomp_thread_data.hpp"

#include "blockmodels.hpp"

namespace cppcraft
{
	/*
	int PrecompThreadData::emitDoor(const Block& currentBlock, int bx, int by, int bz, block_t sides)
	{
		// find direction door is facing
		int model = currentBlock.getFacing();
		// if the door is open, select next set of doors
		if (currentBlock.getExtra() & 2) model += 4;
		
		int vertices = blockmodels.doors.copyTo(model, indic);
		
		applyFaceLightingAll(bx, by, bz);
		
		short tid = Block::cubeFaceById(currentBlock.getID(), 0, currentBlock.getExtra() & 1);
		
		for (int i = 0; i < vertices; i++)
		{
			indic[i].w = tid;
		}
		
		return vertices;
	}*/
}
