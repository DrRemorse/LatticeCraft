#include "blockdata.hpp"
#include <cassert>
#include "../biomes.hpp"
#include "../block.hpp"

#ifdef CPPCRAFT_CLIENT
#include <renderconst.hpp>
#include <tiles.hpp>

namespace cppcraft
{
	// most common mesh is the solid cube
	extern void emitCube(cppcraft::PTD& ptd, int bx, int by, int bz, block_t);
	extern void emitCross(PTD& ptd, int bx, int by, int bz, block_t);
}
#endif
using namespace cppcraft;

namespace db
{
#ifdef CPPCRAFT_CLIENT
  uint32_t BlockData::getDiffuseTexture() const noexcept
  {
    switch (this->shader) {
    case RenderConst::TX_REPEAT:
        return tiledb.bigtiles.diff_texture().getHandle();
    default:
        return tiledb.tiles.diff_texture().getHandle();
    }
  }
  uint32_t BlockData::getTonemapTexture() const noexcept
  {
    switch (this->shader) {
    case RenderConst::TX_REPEAT:
        return tiledb.bigtiles.tone_texture().getHandle();
    default:
        return tiledb.tiles.tone_texture().getHandle();
    }
  }
#endif

  // create a most default solid registry block, then return it
	BlockData& BlockData::createSolid(const std::string name)
	{
		BlockData& solid = BlockDB::get().create(name);
		solid.blocksMovement = [] (const Block&) { return true; };
		solid.forwardMovement = [] (const Block&) { return false; };
		solid.hasActivation = [] (const Block&) { return false; };
		solid.physicalHitbox3D = [] (const Block&, float, float, float) { return true; };
#ifdef CPPCRAFT_CLIENT
		solid.selectionHitbox3D = [] (const Block&, float, float, float) { return true; };
    solid.shader = RenderConst::TX_SOLID;
		solid.visibilityComp = nullptr;
		solid.emit = cppcraft::emitCube;
#endif
		return solid;
	}
	BlockData& BlockData::createFluid(const std::string name)
	{
    BlockData& fluid = BlockDB::get().create(name);
		fluid.blocksMovement = [] (const Block&) { return false; };
		fluid.forwardMovement = [] (const Block&) { return true; };
		fluid.hasActivation = [] (const Block&) { return false; };
		fluid.liquid      = true;
		fluid.transparent = true;
    fluid.setBlock(false);
		fluid.physicalHitbox3D = [] (const Block&, float, float, float) { return true; };
		fluid.selectionHitbox3D = [] (const Block&, float, float, float) { return false; };
#ifdef CPPCRAFT_CLIENT
		fluid.transparentSides = BlockData::SIDE_ALL; // none of them solid
		fluid.visibilityComp =
		[] (const Block& src, const Block& dst, uint16_t mask)
		{
			// if they are both the same ID, we will not add this face
			if (src.getID() == dst.getID())
				return 0;
			// otherwise, business as usual, only add towards transparent sides
			return mask & dst.getTransparentSides();
		};
		fluid.emit = cppcraft::emitCube;
#endif
		return fluid;
	}
	BlockData& BlockData::createLeaf(const std::string name)
	{
    BlockData& leaf = BlockDB::get().create(name);
		leaf.blocksMovement = [] (const Block&) { return true; };
		leaf.forwardMovement = [] (const Block&) { return false; };
		leaf.hasActivation = [] (const Block&) { return false; };
		leaf.transparent = true;
		leaf.physicalHitbox3D = [] (const Block&, float, float, float) { return true; };

#ifdef CPPCRAFT_CLIENT
		leaf.shader = RenderConst::TX_TRANS_1SIDED;
		leaf.selectionHitbox3D = [] (const Block&, float, float, float) { return true; };
		leaf.transparentSides = BlockData::SIDE_ALL; // none of them solid
		leaf.visibilityComp =
		[] (const Block& src, const Block& dst, uint16_t mask)
		{
			// if they are both the same ID, we only add every 2nd face
			if (src.getID() == dst.getID()) {
          if (src.getSkyLight() < 10) return 0;
				  return mask & (1 + 4 + 32);
      }
			// otherwise, business as usual, only add towards transparent sides
			return mask & dst.getTransparentSides();
		};
		leaf.emit = cppcraft::emitCube;
#endif
		return leaf;
	}
	BlockData& BlockData::createCross(const std::string name)
	{
    BlockData& blk = BlockDB::get().create(name);
		blk.cross       = true;  // is indeed a cross
		blk.transparent = true;  // transparent as fuck
    blk.setBlock(false); // dont generate AO
		blk.blocksMovement = [] (const Block&) { return false; };
		blk.forwardMovement = [] (const Block&) { return true; };
		blk.hasActivation = [] (const Block&) { return false; };
    blk.setMinimapColor(Biomes::CL_GRASS);
		blk.physicalHitbox3D = [] (const Block&, float, float, float) { return true; };
#ifdef CPPCRAFT_CLIENT
		blk.repeat_y = false;
		blk.shader = RenderConst::TX_TRANS_2SIDED;
		blk.selectionHitbox3D = [] (const Block&, float, float, float) { return true; };
		blk.transparentSides = BlockData::SIDE_ALL; // none of them solid
		blk.visibilityComp =
		[] (const Block&, const Block&, uint16_t mask)
		{
			return mask; // always draw crosses
		};
		blk.emit = cppcraft::emitCross;
#endif
		return blk;
	}
}
