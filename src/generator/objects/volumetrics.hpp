#ifndef VOLUMETRICS_HPP
#define VOLUMETRICS_HPP

#include <cstdint>
#include <array>
#include "../object.hpp"

namespace terragen
{
  struct Volumetrics
  {
    static const int VOLSIZE = 32;
    typedef std::array<std::array<bool, VOLSIZE>, VOLSIZE> pathmap_t;
    typedef std::array<std::array<bool, VOLSIZE>, VOLSIZE> depthmap_t;

    static bool discover(int x, int y, int z, int64_t id);
    static bool fill(int x, int y, int z, int64_t id);
    static void job_fill(const SchedObject&);

    static bool flood(int x, int z, const int SIZE, depthmap_t&, pathmap_t&);

  private:
    static const int VOLRAD = VOLSIZE / 2;
    static const int VOLDEPTH = 64;
    static const int VOLTRAVEL = 16;
    static void fillDown(int x, int y, int z, int mat, int depth, int travel);
    static int  depth(int x, int y, int z, int depth, const int SRC_ID);
  };
}

#endif
