#include "postproc.hpp"

#include "terragen.hpp"
#include "blocks.hpp"
#include "terrain/terrains.hpp"
#include "oregen.hpp"
#include <library/noise/voronoi.hpp>
#include <glm/gtc/noise.hpp>

using namespace cppcraft;
using namespace db;

namespace terragen
{
	void PostProcess::init()
	{
		// add some åres
		OreGen::init();
	}

	void PostProcess::run(gendata_t* gdata)
	{
		/// go go go gø go go go ///
		for (int x = 0; x < BLOCKS_XZ; x++)
		for (int z = 0; z < BLOCKS_XZ; z++)
		{
      auto& flat = gdata->flatl(x, z);

      // process overworld using terrain postprocessing function
			const auto& terr = terrains[flat.terrain];
			int y = flat.skyLevel;
      if (LIKELY(terr.on_process)) {
        y = terr.on_process(gdata, x, z, flat.skyLevel-1, 0);
      }

      // 1. search for next underworld
      // 2. post-process from Y to nextY **inside** next underworld
      auto& cave = gdata->flatl.cave(x, z);
      int uy = y / 4;
      while (uy > 0)
      {
        const auto uid = cave.underworld[uy];
        // find next terrain ID
        int nextY = uy-1;
        for (; nextY > 0; nextY--)
        {
          const auto next = cave.underworld[nextY];
          if (next != uid) break;
        }
        // process underworld using cave postprocessing function
        //printf("Processing %d to %d for ID %d\n", uy * 4, nextY * 4, uid);
        if (LIKELY(cave_terrains[uid].on_process)) {
          cave_terrains[uid].on_process(gdata, x, z, uy * 4, nextY * 4);
        }
        uy = nextY;
      }

      // do the manual post-processing last
      // - set skylevel and groundlevel
      //printf("Post-processing for terrain %d\n", tid);
  		int skyLevel = 0;
      int gndLevel = 0;

      const int MAX_Y = flat.skyLevel;
  		for (int y = MAX_Y-1; y > 0; y--)
  		{
  			Block& block = gdata->getb(x, y, z);

  			// count air
  			if (!block.isAir())
        {
  				if (skyLevel == 0) skyLevel = y+1;
          if (gndLevel == 0) {
            if (block.isTransparent() == false) gndLevel = y+1;
          }
          // mark this Y-value as having a light source
          if (block.isLight()) gdata->setLight(y);
  			}
  			// use skylevel to determine when we are below sky
  			block.setLight((skyLevel == 0) ? 15 : 0, 0);
  		} // y
      gndLevel = std::max(1, gndLevel);

  		// set initial groundlevel, skylevel
  		flat.groundLevel = gndLevel;
  		flat.skyLevel = skyLevel;
			// guarantee that the bottom block is hard as adminium
			gdata->getb(x, 0, z) = Block(BEDROCK);
		} // next x, z

	} // PostProcess::run()

  /// calculate zone ID for (x, z)
  int PostProcess::get_zone(gendata_t* gdata, int x, int z) noexcept
  {
		const glm::vec2 p = gdata->getBaseCoords2D(x, z) * 0.01f;
		return library::Voronoi::getid(p.x, p.y,
				   library::Voronoi::vor_chebyshev); // distance function
  }

} // terragen
