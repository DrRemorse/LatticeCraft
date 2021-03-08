#pragma once
#include <common.hpp>

namespace terragen
{
  struct Simulator
  {
    static const int TERRA_PICKS = 8;
    static const int BLOCK_ROUNDS = 1;

    static const int SIM_RADIUS = 12;

    static void run(double time);
  };
}
