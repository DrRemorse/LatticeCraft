#include "spiders.hpp"

#include <library/log.hpp>
#include <common.hpp>
#include "chunks.hpp"
#include "minimap.hpp"
#include "lighting.hpp"
#include "sectors.hpp"
using namespace library;

static int64_t total_blocks_placed = 0;

namespace cppcraft
{
	bool Spiders::updateBlock(int bx, int by, int bz, block_t bits)
	{
		Sector* s = Spiders::wrap(bx, by, bz);
		if (s == nullptr) return false;
    return updateBlock(*s, bx, by, bz, bits);
  }
  bool Spiders::updateBlock(Sector& sector, int bx, int by, int bz, block_t bits)
  {
    if (UNLIKELY(sector.generated() == false))
		{
			printf("Could not setblock on %d, %d: not generated\n",
					   sector.getX(), sector.getZ());
			return false;
		}
		Block& block = sector(bx, by, bz);
		// set bitfield directly
		block.setBits(bits);
		// make sure the mesh is updated
		sector.updateAllMeshes();
		// write updated sector to disk
		//chunks.addSector(*s);
		return true;
	}

	bool Spiders::setBlock(int bx, int by, int bz, const Block& newblock)
	{
		Sector* s = Spiders::wrap(bx, by, bz);
		if (UNLIKELY(s == nullptr) || by > TOP_BLOCK_Y)
		{
			printf("Could not setblock(%d, %d, %d): out of bounds\t",
					   bx, by, bz);
			return false;
		}
    return setBlock(*s, bx, by, bz, newblock);
  }
  bool Spiders::setBlock(Sector& sector, int bx, int by, int bz, const Block& newblock)
  {
    if (UNLIKELY(sector.generated() == false))
		{
			printf("Could not setblock on %d, %d: not generated\n",
					   sector.getX(), sector.getZ());
			return false;
		}
    ::total_blocks_placed++;
		// set new block
		Block& blk = sector(bx, by, bz);
		blk = newblock;
		// if setting this block changes the skylevel, propagate zero-light down
		int skylevel = sector.flat()(bx, bz).skyLevel;
		if (by >= skylevel)
		{
			// from hero to zero
			for (int y = skylevel; y <= by; y++) {
				sector(bx, y, bz).setSkyLight(0);
      }

			// re-flood skylight down to old skylevel
			if (sector.atmospherics) {
        // only if atmospherics is already finished for this sector
        Lighting::deferredRemove(sector, bx, skylevel, by, bz, 15-1);
      }
      // set new skylevel?
      sector.flat()(bx, bz).skyLevel = by+1;
      // update minimap since top-level changed
      minimap.sched(sector);
		}
		else
		{
      auto level = blk.getSkyLight();
      blk.setSkyLight(0);
			// for all 6 sides of the block we added, theres a possibility that we blocked off light
			// re-flood light on all sides
			if (sector.atmospherics && level > 1) {
        // only if atmospherics is already finished for this sector
        Lighting::deferredRemove(sector, bx, by, by, bz, level-1);
      }
		}

		// for lights, we will flood lighting outwards
		if (UNLIKELY(blk.isLight()))
		{
      // mark light existing at by
      sector.getBlocks().setLight(by);
      // set light source level for block
			blk.setTorchLight(blk.getOpacity(0));
      // start flooding
			Lighting::floodOutof(sector.getX()*BLOCKS_XZ + bx,
                           by,
                           sector.getZ()*BLOCKS_XZ + bz,
                           1, blk.getTorchLight());
		}

		// write updated sector to disk
		//chunks.addSector(sector);
    // update mesh
		sector.updateAllMeshes();
		// update nearby sectors only if we are at certain edges
		updateSurroundings(sector, bx, by, bz);
		return true;
	}

	Block Spiders::removeBlock(int bx, int by, int bz)
	{
		Sector* s = Spiders::wrap(bx, by, bz);
		// if the given position is outside the local area, null will be returned
		if (s == nullptr) return air_block;
    if (UNLIKELY(s->generated() == false))
		{
			printf("Could not removeBlock(%d, %d, %d): not generated\t",
					   bx, by, bz);
			return false;
		}
    return removeBlock(*s, bx, by, bz);
  }

  Block Spiders::removeBlock(Sector& sector, int bx, int by, int bz)
  {
		// make a copy of the block, so we can return it
		Block block = sector(bx, by, bz);
		assert(block.getID() != _AIR);

		// set the block to _AIR
		sector(bx, by, bz).setID(_AIR);

		// when the skylevel is the current height, we know that the it must be propagated down
		int skylevel = sector.flat()(bx, bz).skyLevel;
		if (by >= skylevel-1)
		{
			Lighting::skyrayDownwards(sector, bx, by, bz);
      // update minimap since top-level changed
      minimap.sched(sector);
		}
    else if (block.isTransparent() == false) {
      // try to flood this new fancy empty space with light
      Lighting::floodInto(&sector, bx, by, bz, 0);
    }

    // to remove lights we will have to do a more.. thorough job
		if (block.isLight()) {
			Lighting::removeLight(block, sector.getX()*BLOCKS_XZ + bx,
                                   by,
                                   sector.getZ()*BLOCKS_XZ + bz);
    }
    else {
      Lighting::floodInto(&sector, bx, by, bz, 1);
    }

		// write updated sector to disk
		//chunks.addSector(*s);
    // update the mesh, so we can see the change!
		sector.updateAllMeshes();
		// update neighboring sectors (depending on edges)
		updateSurroundings(sector, bx, by, bz);
		// return COPY of block
		return block;
	}

	inline void updateNeighboringSector(Sector& sector, int)
	{
		// if the sector in question has blocks already,
		if (sector.generated())
			// just regenerate his mesh
			sector.updateAllMeshes();
	}

	void Spiders::updateSurroundings(Sector& sector, int bx, int by, int bz)
	{
		if (bx == 0)
		{
			if (sector.getX())
			{
				Sector& testsector = sectors(sector.getX()-1, sector.getZ());
				updateNeighboringSector(testsector, by);
			}
		}
		else if (bx == Sector::BLOCKS_XZ-1)
		{
			if (sector.getX()+1 != sectors.getXZ())
			{
				Sector& testsector = sectors(sector.getX()+1, sector.getZ());
				updateNeighboringSector(testsector, by);
			}
		}
		if (bz == 0)
		{
			if (sector.getZ())
			{
				Sector& testsector = sectors(sector.getX(), sector.getZ()-1);
				updateNeighboringSector(testsector, by);
			}
		}
		else if (bz == Sector::BLOCKS_XZ-1)
		{
			if (sector.getZ()+1 != sectors.getXZ())
			{
				Sector& testsector = sectors(sector.getX(), sector.getZ()+1);
				updateNeighboringSector(testsector, by);
			}
		}
	}

  int64_t Spiders::total_blocks_placed() noexcept {
    return ::total_blocks_placed;
  }
}
