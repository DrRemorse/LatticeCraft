#include "lighting.hpp"

#include "sectors.hpp"
#include "spiders.hpp"
#include <queue>


namespace cppcraft
{
  static_assert(sizeof(Lighting::propagate_t) <= 8, "Needs to fit in a register");
  using emitter_t = Lighting::emitter_t;

  void Lighting::propagateChannel(
      Sector* sector, int bx, int by, int bz, propagate_t p)
  {
    while (p.level > 0)
    {
  		// move in ray direction
  		switch (p.dir) {
  		case 0:
          if (UNLIKELY(++bx >= BLOCKS_XZ)) {
            if (sector->getX()+1 >= sectors.getXZ()) return;
            sector = &sectors(sector->getX()+1, sector->getZ());
            bx = 0;
          }
          break;
  		case 1:
          if (UNLIKELY(--bx < 0)) {
            if (sector->getX()-1 < 0) return;
            sector = &sectors(sector->getX()-1, sector->getZ());
            bx = BLOCKS_XZ-1;
          }
          break;
  		case 2:
          if (UNLIKELY(++by >= BLOCKS_Y)) return;
          break;
  		case 3:
          if (UNLIKELY(--by < 0)) return;
          break;
  		case 4:
          if (UNLIKELY(++bz >= BLOCKS_XZ)) {
            if (sector->getZ()+1 >= sectors.getXZ()) return;
            sector = &sectors(sector->getX(), sector->getZ()+1);
            bz = 0;
          }
          break;
  		case 5:
          if (UNLIKELY(--bz < 0)) {
            if (sector->getZ()-1 < 0) return;
            sector = &sectors(sector->getX(), sector->getZ()-1);
            bz = BLOCKS_XZ-1;
          }
          break;
  		}

  		Block& blk2 = (*sector)(bx, by, bz);
  		// decrease light level based on what we hit
  		p.level -= lightPenetrate(blk2);

  		// once level reaches zero we are done, so early exit
  		if (p.level <= 0) break; // impossible to have a value less than < 0
  		// avoid lowering light values for air
  		if (blk2.getChannel(p.ch) >= p.level) break;

  		// set new light level
      //printf("propagateChannel(%d, %d, %d): ch %d to %d (from %d) X=%d Z=%d bx=%d bz=%d\n",
      //        x, y, z, ch, level, blk2.getChannel(ch), sector.getX(), sector.getZ(), bx, bz);
  		blk2.setChannel(p.ch, p.level);

  		// make sure the sectors mesh is updated, since something was changed
  		if (!sector->isUpdatingMesh()) sector->updateAllMeshes();

      // update all neighboring sectors :(
      if (UNLIKELY(bx == 0 || bx == BLOCKS_XZ-1 || bz == 0 || bz == BLOCKS_XZ-1))
      {
        if (bx == 0)
        {
          if (sector->getX() > 0)
              sectors(sector->getX()-1, sector->getZ()).updateAllMeshes();
        }
        else if (bx == BLOCKS_XZ-1)
        {
          if (sector->getX()+1 < sectors.getXZ())
              sectors(sector->getX()+1, sector->getZ()).updateAllMeshes();
        }
        if (bz == 0)
        {
          if (sector->getZ() > 0)
              sectors(sector->getX(), sector->getZ()-1).updateAllMeshes();
        }
        else if (bz == BLOCKS_XZ-1)
        {
          if (sector->getZ()+1 < sectors.getXZ())
              sectors(sector->getX(), sector->getZ()+1).updateAllMeshes();
        }
      }

  		switch (p.dir) {
  		case 0: // +x
  		case 1: // -x
  			propagateChannel(sector, bx, by, bz, {p.ch, 2, p.level});
  			propagateChannel(sector, bx, by, bz, {p.ch, 3, p.level});
  			propagateChannel(sector, bx, by, bz, {p.ch, 4, p.level});
  			propagateChannel(sector, bx, by, bz, {p.ch, 5, p.level});
  			break;
  		case 2: // +y
  		case 3: // -y
  			propagateChannel(sector, bx, by, bz, {p.ch, 0, p.level});
  			propagateChannel(sector, bx, by, bz, {p.ch, 1, p.level});
  			propagateChannel(sector, bx, by, bz, {p.ch, 4, p.level});
  			propagateChannel(sector, bx, by, bz, {p.ch, 5, p.level});
  			break;
  		case 4: // +z
  		case 5: // -z
  			propagateChannel(sector, bx, by, bz, {p.ch, 0, p.level});
  			propagateChannel(sector, bx, by, bz, {p.ch, 1, p.level});
  			propagateChannel(sector, bx, by, bz, {p.ch, 2, p.level});
  			propagateChannel(sector, bx, by, bz, {p.ch, 3, p.level});
  			break;
  		}
    } // for (level)
  }

}
