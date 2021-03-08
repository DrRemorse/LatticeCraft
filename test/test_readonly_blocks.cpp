#include <readonly_blocks.hpp>
#include <sectors.hpp>

#include <catch.hpp>
using namespace cppcraft;

extern Sectors sectors;

TEST_CASE("Read-only blocks sanity check")
{
  ReadonlyBlocks rob(8, 8, 1);
  REQUIRE(rob.size() == 9);

}

TEST_CASE("Read-only blocks test each sector")
{
  ReadonlyBlocks rob(8, 8, 2);

  for (int x = -2; x <= 2; x++)
  for (int z = -2; z <= 2; z++)
  {
    REQUIRE(rob(x, z).x == x);
    REQUIRE(rob(x, z).z == z);
  }

}

TEST_CASE("Read-only blocks test out-of-bounds")
{
  ReadonlyBlocks rob(8, 8, 2);

  REQUIRE_THROWS(rob(-3, -3));
  REQUIRE_THROWS(rob(-3,  3));
  for (int x = -2; x <= 2; x++)
  {
    REQUIRE_THROWS(rob(x, -3));
    REQUIRE_THROWS(rob(x,  3));

    REQUIRE_THROWS(rob(-3, x));
    REQUIRE_THROWS(rob( 3, x));
  }
  REQUIRE_THROWS(rob( 3, -3));
  REQUIRE_THROWS(rob( 3,  3));
}
