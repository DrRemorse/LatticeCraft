#include "lighting.hpp"

#include "sectors.hpp"
#include "spiders.hpp"
#include "world.hpp"
#include <library/timing/timer.hpp>
#include <deque>
#include <queue>
using namespace library;
//#define TIMING
static const int MAX_REMOVALS = 800;

namespace cppcraft
{
  extern void removeSkylight(int x, int y, int z, char dir, char lvl, std::queue<Lighting::emitter_t>&);
  struct DeferredRemovedLight {
    int  x;
    int  y1, y2;
    int  z;
    short lvl;
  };
  static std::deque<DeferredRemovedLight> lreque;

  void Lighting::deferredRemove(
        Sector& sector, int bx, int y1, int y2, int bz, short lvl)
  {
    const int x = bx + sector.getWX() * BLOCKS_XZ;
    const int z = bz + sector.getWZ() * BLOCKS_XZ;
    lreque.push_back({x, y1, y2, z, lvl});
  }

  void Lighting::handleDeferred()
  {
    if (lreque.empty()) return;
    Timer timer;
    std::queue<Lighting::emitter_t>  lrefill;

    // remove all light all scheduled locations
    int removals = 0;
    while (lreque.empty() == false)
    {
      auto& loc = lreque.front();
      // calculate local coordinates, and validate
      int x = loc.x - world.getWX() * BLOCKS_XZ;
      int z = loc.z - world.getWZ() * BLOCKS_XZ;
      if (x >= 0 && z >= 0 && x < sectors.getXZ() * BLOCKS_XZ
                           && z < sectors.getXZ() * BLOCKS_XZ)
      {
        for (int y = loc.y1; y <= loc.y2; y++) {
          // start rays in all 6 directions
    		  for (char dir = 0; dir < 6; dir++)
    			   removeSkylight(x, y, z, dir, loc.lvl, lrefill);
        }
      }
      lreque.pop_front();
      removals++;
      if (removals >= MAX_REMOVALS) break;
    }
		// re-flood edge values that were encountered
#ifdef TIMING
    int sources = 0;
#endif
		while (!lrefill.empty())
		{
			const emitter_t& e = lrefill.front();
      int bx = e.x;
      int by = e.y;
      int bz = e.z;
      Sector* sector = Spiders::wrap(bx, by, bz);
      if (sector != nullptr)
      {
        short lvl = (*sector)(bx, by, bz).getSkyLight();
        if (lvl == e.lvl) {
          char dir = e.dir;
          switch (e.dir) {
          case 0: dir = 1; break;
          case 1: dir = 0; break;
          case 2: dir = 3; break;
          case 3: dir = 2; break;
          case 4: dir = 5; break;
          case 5: dir = 4; break;
          default: assert(0 && "Invalid direction in skylight emitter");
          }
          propagateChannel(sector, bx, by, bz, {0, dir, lvl});
        }
      }
			lrefill.pop();
#ifdef TIMING
      sources++;
#endif
		}

#ifdef TIMING
    printf("Light correction took %f secs, %d removals %d sources\n",
            timer.getTime(), removals, sources);
#endif
  }

}
