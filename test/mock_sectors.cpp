#include "sectors.hpp"

namespace cppcraft
{
  Sectors sectors(32);
}

#include <catch.hpp>
using namespace cppcraft;

TEST_CASE("Sectors class is sane")
{
  REQUIRE(sectors.getXZ() == 32);

}
