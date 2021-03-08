#ifndef MUSHROOMS_HPP
#define MUSHROOMS_HPP

#include <cstdint>
#include <array>
#include "../object.hpp"

namespace terragen
{
  struct Mushroom {
    static void init();
    static void wild_shroom(const SchedObject&);
    static void huge_shroom(const SchedObject&);
  };
}

#endif
