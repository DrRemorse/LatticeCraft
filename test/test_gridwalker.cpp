#include <grid_walker.hpp>

#include <catch.hpp>
using namespace cppcraft;

Sectors sectors(32);

TEST_CASE("Grid walker sanity checks")
{
  GridWalker walker1(64, 64, 64);
  REQUIRE(walker1.good());

  GridWalker walker2(walker1);
  REQUIRE(walker2.good());
}

TEST_CASE("Grid walker Y-axis")
{
  GridWalker walker1(64, 64, 64);
  REQUIRE(walker1.good());

  for (int y = 0; y < 64; y++) {
    walker1.move_y(-1);
    REQUIRE(walker1.good());
  }
}
