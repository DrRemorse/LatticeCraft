#pragma once
#include "spiders.hpp"

namespace cppcraft
{
  struct GridWalker
  {
    inline GridWalker(int x, int y, int z);
    inline GridWalker(Sector& sector, int x, int y, int z);
    inline GridWalker(const GridWalker&);
    inline GridWalker& operator= (const GridWalker&);

    // move safely with wrap-around
    inline GridWalker& move_x(int dx);
    inline GridWalker& move_y(int dy);
    inline GridWalker& move_z(int dz);
    inline GridWalker& move_xz(int dx, int dz);
    inline GridWalker& move(int dx, int dy, int dz);
    // set Y-value directly
    inline GridWalker& set_y(int y) { this->y = y; return *this; }

    inline Block& get() const;
    inline bool   set(const Block& blk) const;
    inline Block  remove() const;
    inline Block& peek_above() const;

    int getY() const noexcept { return y; }
    int getX() const noexcept { return sector->getX() * BLOCKS_XZ + x; }
    int getZ() const noexcept { return sector->getZ() * BLOCKS_XZ + z; }

    int terrain() const { return flat().terrain; }
    const Flatland::flatland_t& flat() const { return sector->flat()(x, z); }
    Flatland::flatland_t& flat() { return sector->flat()(x, z); }

    bool good() const noexcept { return sector != nullptr; }
    bool buildable() const noexcept { return good() && sector->generated(); }
    bool atmospherics() const noexcept { return good() && sector->atmospherics; }
  private:
    Sector* sector;
    int x, y, z;
  };
}

#ifdef CPPCRAFT_CLIENT
#include "grid_walker_client.hpp"
#else
#include "grid_walker_server.hpp"
#endif
