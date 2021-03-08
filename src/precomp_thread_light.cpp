#include "precomp_thread_data.hpp"

#include "blocks_bordered.hpp"
#include <cmath>

namespace cppcraft
{
	light_value_t PTD::getLight(int x, int y, int z)
	{
		Block& block = sector->get(x, y, z);

		light_value_t RGBA = 0;
		for (int ch = 0; ch < Block::CHANNELS; ch++)
		{
			int V = block.getChannel(ch);
			RGBA |= (V * 17) << (ch * 8);
		}
		return RGBA;
	}

	light_value_t PTD::smoothLight(const Block& source,
                                 int x1, int y1, int z1,
                                 int x2, int y2, int z2,
                                 int x3, int y3, int z3,
                                 int x4, int y4, int z4)
	{
		// TODO: calculate the actual light values...
		Block* bl[4];
		bl[0] = &sector->get(x1, y1, z1);
		bl[1] = &sector->get(x2, y2, z2);
		bl[2] = &sector->get(x3, y3, z3);
		bl[3] = &sector->get(x4, y4, z4);

    light_value_t final_light = 0;
    int ramp = 0;
    for (int ch = 0; ch < Block::CHANNELS; ch++)
    {
  		uint8_t V = 0;
      int total = 0;
  		for (auto& blk : bl)
  		{
        if (ch == 0) {
          if (blk->isTransparent()) {
  			     V += blk->getChannel(ch);
             total++;
          }
          // ambient occlusion gradients
          if (source.isTransparent() && blk->isTransparent() && !blk->isAir())
            ramp++;
          else if (blk->isBlock()) ramp++;
        }
        else if (blk->isTransparent() || blk->isLight()) {
            V += blk->getChannel(ch);
            total++;
        }
  		}
      if (total != 0)
      {
        if (ch == 0)
        {
          const float lv = (float) V / total;
          float factor = 1.0f - std::min(y1 , WATERLEVEL) / (float) BLOCKS_Y;
          float light = powf(0.88f, (15.0f - lv) * factor * factor);
          final_light |= int(25 + 230.0f * light);
          // ambient occlusion gradients
          static unsigned char shadowRamp[] =
              { 127, 127 - 28, 127 - 40, 127 - 56, 127 - 64 };
          final_light |= shadowRamp[ramp] << 16;
        }
        else
        {
          const float lv = (float) V / total;
          final_light |= int(lv * 17) << (ch * 8);
        }
      }
    }
    return final_light;
	}

  inline void set_light(vertex_t& vtx, uint32_t light)
  {
    vtx.ao = light >> 16;
    vtx.light = light & 0xFFFF;
  }

	void PTD::faceLighting_PZ(const Block& blk, vtx_iterator vtx, int bx, int by, int bz)
	{
		set_light(vtx[0], smoothLight(blk, bx  , by,   bz,  bx-1,by,bz,   bx-1,by-1,bz,  bx,by-1,bz));
		set_light(vtx[1], smoothLight(blk, bx+1, by,   bz,  bx, by, bz,   bx,by-1,bz,  bx+1,by-1,bz));
		set_light(vtx[2], smoothLight(blk, bx+1, by+1, bz,  bx,by+1,bz,   bx,by,bz,    bx+1,by,bz));
		set_light(vtx[3], smoothLight(blk, bx  , by+1, bz,  bx-1,by+1,bz, bx-1,by,bz,    bx,by,bz));
	}

	void PTD::faceLighting_NZ(const Block& blk, vtx_iterator vtx, int bx, int by, int bz)
	{
		set_light(vtx[0], smoothLight(blk, bx  , by  , bz,   bx-1, by  ,bz,  bx, by-1, bz,  bx-1, by-1, bz));
		set_light(vtx[1], smoothLight(blk, bx  , by+1, bz,   bx-1, by+1,bz,  bx, by  , bz,  bx-1, by  , bz));
		set_light(vtx[2], smoothLight(blk, bx+1, by+1, bz,   bx,  by+1, bz,  bx+1, by, bz,  bx, by  ,   bz));
		set_light(vtx[3], smoothLight(blk, bx+1, by  , bz,   bx,  by,   bz,  bx+1,by-1,bz,  bx, by-1,   bz));
	}

	void PTD::faceLighting_PY(const Block& blk, vtx_iterator vtx, int bx, int by, int bz)
	{
		set_light(vtx[0], smoothLight(blk, bx, by, bz,    bx-1, by, bz,  bx, by, bz-1,  bx-1, by, bz-1));
		set_light(vtx[1], smoothLight(blk, bx, by, bz+1,  bx-1, by, bz+1,  bx, by, bz,  bx-1, by, bz));
		set_light(vtx[2], smoothLight(blk, bx+1, by, bz+1,  bx, by, bz+1,  bx+1, by, bz,  bx, by, bz));
		set_light(vtx[3], smoothLight(blk, bx+1, by, bz,  bx, by, bz,  bx+1, by, bz-1,  bx, by, bz-1));
	}

	void PTD::faceLighting_NY(const Block& blk, vtx_iterator vtx, int bx, int by, int bz)
	{
		set_light(vtx[0], smoothLight(blk, bx  , by, bz  , bx-1, by, bz,  bx, by, bz-1,  bx-1, by, bz-1));
		set_light(vtx[1], smoothLight(blk, bx+1, by, bz  , bx, by, bz,  bx+1, by, bz-1,  bx, by, bz-1));
		set_light(vtx[2], smoothLight(blk, bx+1, by, bz+1, bx, by, bz+1,  bx+1, by, bz,  bx, by, bz));
		set_light(vtx[3], smoothLight(blk, bx  , by, bz+1, bx-1, by, bz+1,  bx, by, bz,  bx-1, by, bz));
	}

	void PTD::faceLighting_PX(const Block& blk, vtx_iterator vtx, int bx, int by, int bz)
	{
		set_light(vtx[0], smoothLight(blk, bx, by,   bz  ,   bx,by,bz-1,   bx,by-1,bz-1,   bx,by-1,bz));
		set_light(vtx[1], smoothLight(blk, bx, by+1, bz  ,   bx,by+1,bz-1, bx,by,bz-1,     bx,by,bz));
		set_light(vtx[2], smoothLight(blk, bx, by+1, bz+1,   bx,by+1,bz,   bx,by,bz,       bx,by,bz+1));
		set_light(vtx[3], smoothLight(blk, bx, by,   bz+1,   bx,by,bz,     bx,by-1,bz,     bx,by-1,bz+1));
	}

	void PTD::faceLighting_NX(const Block& blk, vtx_iterator vtx, int bx, int by, int bz)
	{
		set_light(vtx[0], smoothLight(blk, bx, by,   bz  , bx,by,bz-1,   bx,by-1,bz-1,   bx,by-1,bz));
		set_light(vtx[1], smoothLight(blk, bx, by,   bz+1, bx,by,bz,     bx,by-1,bz,     bx,by-1,bz+1));
		set_light(vtx[2], smoothLight(blk, bx, by+1, bz+1, bx,by+1,bz,   bx,by,bz,       bx,by,bz+1));
		set_light(vtx[3], smoothLight(blk, bx, by+1, bz  , bx,by+1,bz-1, bx,by,bz-1,     bx,by,bz));
	}

}
