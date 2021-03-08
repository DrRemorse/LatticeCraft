#include "auto_growth.hpp"

#include <grid_walker.hpp>

using namespace db;

namespace terragen
{
  void auto_growth(block_t source_id, const block_t DEST_ID, bool daylight)
  {
    auto& source = db::BlockDB::get()[source_id];
    source.on_tick =
    [DEST_ID, daylight] (cppcraft::GridWalker& walker)
    {
      // validate atmospherics
      if (walker.atmospherics() == false) return;
      // if enough light, ...
      auto lva = walker.peek_above().getSkyLight();
      bool light_test = (daylight) ? (lva > 8) : lva < 8;
      if (light_test)
      {
        // look for rad=1 nearby grass with air above
        for (int x = 0; x < 3; x++)
        for (int z = 0; z < 3; z++)
        {
          if (x == 1 && z == 1) continue;
          GridWalker copy(walker);
          copy.move(x-1, -1, z-1);
          // validate location
          if (copy.buildable())
          {
            for (int y = 0; y < 3; y++)
            {
              // promote if grass
              if (copy.get().getID() == DEST_ID)
              {
                //printf("Promoted block\n");
                walker.set(Block(DEST_ID));
                return;
              }
              copy.move_y(1);
            }
          }
        }
      }
    };
  } // auto_growth()


} // terragen
