#include "../precomp_thread_data.hpp"
#include "../blocks_bordered.hpp"
#include "../blockmodels.hpp"

namespace cppcraft
{
	inline short randomHeight(int bx, int bz)
	{
		return (((bx + bz * 3) & 7) - 2) * 5;
	}

	void emitCross(PTD& ptd, int bx, int by, int bz, block_t)
	{
    const Block& block = ptd.sector->get(bx, by, bz);
    // copy cross-mesh object 0 (cross)
		auto vtx = blockmodels.crosses.copyTo(0, ptd.current());

		// huge boring list of cross-lighting
		vtx[0].light = ptd.smoothLight(block, bx, by, bz,  bx-1, by, bz,  bx, by, bz-1,  bx-1, by, bz-1);
		vtx[1].light = ptd.smoothLight(block, bx+1, by, bz+1,  bx, by, bz+1,  bx+1, by, bz,  bx, by, bz);
		vtx[2].light = vtx[1].light;
		vtx[3].light = vtx[0].light;
		vtx[4].light = ptd.smoothLight(block, bx+1, by, bz,  bx, by, bz,  bx+1, by, bz-1,  bx, by, bz-1);
		vtx[5].light = ptd.smoothLight(block, bx, by, bz+1,  bx-1, by, bz+1,  bx, by, bz,  bx-1, by, bz);
		vtx[6].light = vtx[5].light;
		vtx[7].light = vtx[4].light;

		// terrain color
		Block& blk = ptd.sector->get(bx, by, bz);
		auto clid = blk.db().getColorIndex();

		vtx[0].color = ptd.getColor(bx  , bz  , clid); // (0, 0)
		vtx[3].color = ptd.getColor(bx  , bz  , clid);

		vtx[4].color = ptd.getColor(bx+1, bz  , clid); // (1, 0)
		vtx[5].color = ptd.getColor(bx+1, bz  , clid);

		vtx[6].color = ptd.getColor(bx  , bz+1, clid); // (0, 1)
		vtx[7].color = ptd.getColor(bx  , bz+1, clid);

		vtx[2].color = ptd.getColor(bx+1, bz+1, clid); // (1, 1)
		vtx[1].color = ptd.getColor(bx+1, bz+1, clid);

		// set first vertex tile id
		vtx->w = blk.getTexture(0);
		vtx->ao = 127;
		// set reset
		for (int i = 1; i < blockmodels.crosses.size(0); i++)
		{
			vtx[i].w  = vtx->w;
			vtx[i].ao = vtx->ao;
		}

		// random height for non-special crosses
		short height = randomHeight(bx, bz);
		vtx[2].y += height;
		vtx[3].y += height;
		vtx[6].y += height;
		vtx[7].y += height;
	}
}
