#include "itemdata.hpp"

#include <cassert>
#ifdef CPPCRAFT_CLIENT
# include <tiles.hpp>
#endif

using namespace cppcraft;

namespace db
{
#ifdef CPPCRAFT_CLIENT
  uint32_t ItemData::getDiffuseTexture() const noexcept
  {
    return tiledb.items.diff_texture().getHandle();
  }
  uint32_t ItemData::getTonemapTexture() const noexcept
  {
    return tiledb.items.tone_texture().getHandle();
  }
#endif

}
