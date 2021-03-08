#include "readonly_blocks.hpp"

#ifdef CPPCRAFT_CLIENT
#include <sectors.hpp>
#include <world.hpp>
namespace cppcraft
{
  ReadonlyBlocks::ReadonlyBlocks(int WX, int WZ, int Radius)
    : wx(WX), wz(WZ), radius(Radius)
  {
    const int sx = wx - world.getWX();
    const int sz = wz - world.getWZ();

    for (int x = -radius; x <= radius; x++)
    for (int z = -radius; z <= radius; z++)
    {
      storage.emplace_back(
            sectors(sx+x, sz+z).getBlocks(), x, z);
    }
  }

}
#endif
