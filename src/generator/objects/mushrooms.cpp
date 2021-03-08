#include "mushrooms.hpp"

#include "../blocks.hpp"
#include "../object.hpp"
#include "../random.hpp"
#include <biomes.hpp>
#include <grid_walker.hpp>
#include "../../tiles.hpp"
#include "../../renderconst.hpp"
#include "helpers.hpp"
#include <library/timing/timer.hpp>
#include <library/bitmap/colortools.hpp>
#include <cmath>

using namespace cppcraft;
using namespace library;
//#define TIMING

namespace terragen
{
  using BlockData = db::BlockData;
  static const int TYPE_CORE     = 0;
  static const int TYPE_CORE_TOP = 16;
  static const int TYPE_TOP   = 32;
  static const int TYPE_EDGE  = 48;
  static const int TYPE_SPECK = 64;
  static const int TYPE_UNDER = 80;

  inline void shroom_set_type(Block& blk, int type) {
    blk.setExtra(blk.getExtra() | type);
  }

  void Mushroom::init()
  {
    auto& db = db::BlockDB::get();
    // the mushroom block
		{
			auto& blk = BlockData::createSolid();
			blk.getColor = [] (const Block& blk) {
        return Biomes::getSpecialColorRGBA(blk.getExtra() & 0xF);
      };
			blk.useMinimapFunction(
  			[] (const Block& blk, GridWalker&)
  			{
  				return Biomes::getSpecialColorRGBA(blk.getExtra() & 0xF);
  			});
			blk.getName = [] (const Block&) { return "Mushroom Block"; };
			blk.useTextureFunction(
			[] (const Block& blk, uint8_t face) -> short
			{
        int type = blk.getExtra() & 0xF0;
        // core
        if (type == TYPE_CORE) return tiledb.tiles("mushroom_core");
        // core_top
        if (type == TYPE_CORE_TOP) return tiledb.tiles("mushroom_coretop");
        // top
        if (type == TYPE_TOP) return tiledb.tiles("mushroom_top");
        // speck
        if (type == TYPE_SPECK) return tiledb.tiles("mushroom_speckle");
        // edge
        if (type == TYPE_EDGE) {
          if (face == 3) return tiledb.tiles("mushroom_topunder");
          return tiledb.tiles("mushroom_topedge");
        }
        // under
        if (type == TYPE_UNDER) return tiledb.tiles("mushroom_under");
        return 0;
			});
			blk.repeat_y = true;
			blk.shader   = RenderConst::TX_SOLID;
      blk.getSound = [] (const Block&) { return "cloth"; };
			db.assign("mushroom_block", blk);
		}

  }

  static void
  omushHood(int x, int y, int z, const int radius, block_t ID)
  {
  	float stretch_y = 0.75f;
  	float ofs_y     = (float)radius * 0.5f;
  	float hoodradix = (float)radius * 0.9f;

  	float rad, fdx, fdy, fdz;

  	// block with special from 0 to 15
    const Block copy(ID, 0, randf(x, y-11, z) * 16);
  	// chance for speckle dot material
  	const float speckle_chance = 0.05f;

  	for (int dx = -radius; dx <= radius; dx++)
  	{
  		for (int dz = -radius; dz <= radius; dz++)
  		{
        GridWalker walker(x+dx, 0, z+dz);

  			for (int dy = -4; dy <= radius; dy++)
  			{
  				fdx = (float)(dx * dx);
  				fdy = (float)dy + ofs_y; fdy *= fdy;
  				fdz = (float)(dz * dz);

  				rad = sqrtf( fdx + fdy + fdz );
  				if (rad <= radius)
  				{
            Block mat(copy);
  					if (rad >= radius * 0.95f)
  					{
  						if (randf(x+dx, y+dy, z+dz) < speckle_chance) {
                shroom_set_type(mat, TYPE_SPECK);
              } else {
                shroom_set_type(mat, TYPE_TOP);
              }
  					}
  					else
  					{
              shroom_set_type(mat, TYPE_UNDER);
  					}

  					if (rad > hoodradix)
  					{
              float bell = radius * (1.0f - stretch_y);
              //Spiders::setBlock(x+dx, y - bell + dy * stretch_y, z+dz, mat);
              walker.set_y(y - bell + dy * stretch_y).set(mat);
  					}

  				} // rad <= radius

  			} // dy
  		} // dz
  	} // dx

  }

  void Mushroom::huge_shroom(const SchedObject& obj)
  {
    const block_t SHROOM_BLOCK = db::getb("mushroom_block");
#ifdef TIMING
    Timer timer;
#endif

    const int height = obj.data;
  	const float lowrad = randf(obj.x, obj.z, obj.y) * 4 + 8;
  	float toprad = lowrad * 0.4f;
  	float currad = 0;

  	//! we want a good radius of 4 for a good platform to stand on
  	if (coretest(obj.x, obj.y, obj.z, 2, 3, height) == false) return;

  	const float jitter = 2.0f;
  	const float interpolback = 0.05f;
  	float jitter_x = (randf(obj.x+1, obj.y, obj.z-1) - 0.5f) * jitter;
  	float jitter_z = (randf(obj.x-1, obj.y, obj.z+1) - 0.5f) * jitter;

  	float dx = obj.x, dz = obj.z;

  	for (int dy = 0; dy <= height; dy++)
  	{
  		currad = 1.0f - (float)dy / (float)height;
  		currad *= currad;
  		currad = lowrad * currad + toprad * (1.0f - currad);

  		dx += jitter_x;  jitter_x *= 1.0f - interpolback;
  		dz += jitter_z;  jitter_z *= 1.0f - interpolback;

      Block mat_core(SHROOM_BLOCK, 0, TYPE_CORE);
  		if (dy == 0)
  			ocircleXZroots( (int)dx, obj.y + dy, (int)dz, currad, mat_core);
  		else {
        if (dy == height) shroom_set_type(mat_core, TYPE_CORE_TOP);
  			ocircleXZ( (int)dx, obj.y + dy, (int)dz, currad, mat_core);
      }
  	}

  	// create hood
  	const int RAD = (int)(currad * 8);
  	omushHood((int)dx, obj.y+height-1, (int)dz, RAD, SHROOM_BLOCK);

#ifdef TIMING
    printf("Huge mushroom took %f seconds\n", timer.getTime());
#endif
  }

  void Mushroom::wild_shroom(const SchedObject& obj)
  {
    const block_t SHROOM_BLOCK = db::getb("mushroom_block");
#ifdef TIMING
    Timer timer;
#endif

    const int height = obj.data;
  	const float lowrad = (float)height / 2.8f;
  	const float toprad = lowrad * 0.4f;
  	float currad = 0;

  	//! we want a good radius of 3 for a good platform to stand on
  	if (coretest(obj.x, obj.y, obj.z, 2, 3, height) == false) return;

  	const float jitter = 1.0f;
  	const float interpolback = 0.05f;
  	float jitter_x = randf(obj.x+3, obj.y+5, obj.z-4) * jitter - jitter * 0.5f;
  	float jitter_z = randf(obj.x-5, obj.y-3, obj.z+7) * jitter - jitter * 0.5f;

  	float dx = obj.x, dz = obj.z;
  	for (int dy = 0; dy <= height; dy++)
  	{
  		currad = powf((float)(height - dy) / (float)height, 2.5f);
  		currad = toprad * (1.0f - currad) + lowrad * currad;

  		dx += jitter_x;  jitter_x *= interpolback;
  		dz += jitter_z;  jitter_z *= interpolback;

  		// shroom core
  		if (dy == 0)
  		{
        Block mat_core(SHROOM_BLOCK, 0, TYPE_CORE);
  			ocircleXZroots( (int)dx, obj.y + dy, (int)dz, currad, mat_core);
  		}
  		else
  		{
        Block mat_core(SHROOM_BLOCK, 0, (dy != height) ? TYPE_CORE : TYPE_CORE_TOP);
  			ocircleXZ( (int)dx, obj.y + dy, (int)dz, currad, mat_core);
  		}
  	}

  	float rad_y = height * 0.4f;
  	int rad_xz = height;
  	float inner_rad, shiftup;
  	float dist;

  	const float speckle_chance = 0.05f;

  	// the block material used
  	const Block copy(SHROOM_BLOCK, 0, randf(obj.x, obj.y-11, obj.z) * 16);

  	const float shift_strength       = 3.0f;
  	const float shift_top_slope      = 0.5f;
  	const float shift_top_startslope = 0.0f;
  	const float shift_pos_slope      = 0.85f;
  	const float shift_pos_startslope = 0.0f;
  	const float shift_pos_cap        = rad_y * 0.5f;

		for (int hx = -rad_xz; hx <= rad_xz; hx++)
		{
			for (int hz = -rad_xz; hz <= rad_xz; hz++)
			{
				inner_rad = sqrtf( hx*hx + hz*hz );
				shiftup = (inner_rad - rad_y) * shift_strength;

				if (shiftup < shift_top_startslope)
				{
					shiftup = shift_top_startslope - powf(fabsf(shiftup - shift_top_startslope), shift_top_slope);
				}
				if (shiftup > shift_pos_startslope)
				{
					shiftup = shift_pos_startslope + powf(shiftup - shift_pos_startslope, shift_pos_slope);
				}
				if (shiftup > shift_pos_cap) shiftup = shift_pos_cap;

        // start walking at bottom of Y-axis
        GridWalker walker((int)dx + hx, obj.y + height - rad_xz, (int)dz + hz);

        for (int dy = -rad_xz; dy <= rad_xz; dy++)
      	{
  				int final_shiftup = (dy + shiftup) * (dy + shiftup);

  				dist = sqrtf( hx*hx*0.35 + hz*hz*0.35 + final_shiftup );

  				if (dist <= rad_y)
  				{
            Block mat(copy);
  					if (dy < -rad_y)
  					{
              shroom_set_type(mat, TYPE_EDGE);
  					}
  					else if (dist > rad_y - 2)
  					{
  						if (inner_rad < rad_y && dy < 0)
  						{	// undertop
                shroom_set_type(mat, TYPE_UNDER);
  						}
  						else if (randf(dx+hx, obj.y+dy, dz+hz) < speckle_chance)
  						{	// speckled top
                shroom_set_type(mat, TYPE_SPECK);
  						}
  						else
  						{	// top
                shroom_set_type(mat, TYPE_TOP);
  						}
  					}
  					else
  					{	// inside
              shroom_set_type(mat, TYPE_UNDER);
  					}
            // set final material
            walker.set(mat);
  				} // inside bell radius

          walker.move_y(1);
        } // dy
  		} // hz
  	} // hx
#ifdef TIMING
    //printf("Wild mushroom took %f seconds\n", timer.getTime());
#endif
  }

} // terragen
