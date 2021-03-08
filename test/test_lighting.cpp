#include "lighting.hpp"
#include "sectors.hpp"
#include "spiders.hpp"

#include <catch.hpp>
using namespace cppcraft;

static inline void printChannel(int x, int y, int z, int ch)
{
  printf("block(%d, %d, %d) == %d\n", x, y, z, Spiders::getBlock(x, y, z).getChannel(ch));
}

static void close_sector(Sector& sector)
{
  for (int x = 0; x < Sector::BLOCKS_XZ; x++)
  for (int z = 0; z < Sector::BLOCKS_XZ; z++)
  {
    sector(x, 0, z) = Block(1);
    sector(x, Sector::BLOCKS_Y-1, z) = Block(1);
  }
}

TEST_CASE("Various light filling tests")
{
  auto& s = sectors(1, 1);
  s.flat().assign_new();
  s.atmospherics = true;
  // for this test we also have to fake generating all sectors
  sectors(0, 0).add_genflag(Sector::GENERATED);
  sectors(1, 0).add_genflag(Sector::GENERATED);
  sectors(0, 1).add_genflag(Sector::GENERATED);
  sectors(1, 1).add_genflag(Sector::GENERATED);
  sectors(2, 1).add_genflag(Sector::GENERATED);
  sectors(1, 2).add_genflag(Sector::GENERATED);
  sectors(2, 2).add_genflag(Sector::GENERATED);

  static const int START_Y = 4;
  s.flat()(0, 0).skyLevel = START_Y;
  REQUIRE(s(0, START_Y, 0).getID() == _AIR);
  REQUIRE(s(1, START_Y, 0).getID() == _AIR);

  auto& db = db::BlockDB::get();
  auto& solid = db.create("solid");
  const block_t SOLID = solid.getID();
  auto& solid_block = s(1, START_Y+1, 0);
  solid_block.setID(SOLID);
  // make sure the solid is solid
  REQUIRE(!solid_block.isTransparent());

  // create sun source
  s(0, START_Y, 0).setSkyLight(15);
  Lighting::propagateChannel(&s, 0, START_Y, 0, {0, 0, 15});
  Lighting::propagateChannel(&s, 0, START_Y, 0, {0, 1, 15});
  //Lighting::propagateChannel(&s, 0, START_Y, 0, {0, 2, 15});
  Lighting::propagateChannel(&s, 0, START_Y, 0, {0, 3, 15});
  Lighting::propagateChannel(&s, 0, START_Y, 0, {0, 4, 15});
  Lighting::propagateChannel(&s, 0, START_Y, 0, {0, 5, 15});

  // main column should be fully bright
  for (int y = START_Y; y >= 1; y--) {
    REQUIRE(s(0, y, 0).getSkyLight() == Block::SKYLIGHT_MAX - (START_Y - y));
  }
  // first neighbors should be MAX-1
  for (int y = START_Y; y >= 1; y--) {
    REQUIRE(Spiders::getBlock(16-1, y, 16).getSkyLight() == Block::SKYLIGHT_MAX-1 - (START_Y - y));
    REQUIRE(Spiders::getBlock(16+1, y, 16).getSkyLight() == Block::SKYLIGHT_MAX-1 - (START_Y - y));
    REQUIRE(Spiders::getBlock(16, y, 16-1).getSkyLight() == Block::SKYLIGHT_MAX-1 - (START_Y - y));
    REQUIRE(Spiders::getBlock(16, y, 16+1).getSkyLight() == Block::SKYLIGHT_MAX-1 - (START_Y - y));
  }
  // second dist should be MAX-2
  for (int y = START_Y; y >= 1; y--) {
    REQUIRE(Spiders::getBlock(16-2, y, 16).getSkyLight() == Block::SKYLIGHT_MAX-2 - (START_Y - y));
    REQUIRE(Spiders::getBlock(16+2, y, 16).getSkyLight() == Block::SKYLIGHT_MAX-2 - (START_Y - y));

    REQUIRE(Spiders::getBlock(16, y, 16-2).getSkyLight() == Block::SKYLIGHT_MAX-2 - (START_Y - y));
    REQUIRE(Spiders::getBlock(16, y, 16+2).getSkyLight() == Block::SKYLIGHT_MAX-2 - (START_Y - y));

    REQUIRE(Spiders::getBlock(16-1, y, 16-1).getSkyLight() == Block::SKYLIGHT_MAX-2 - (START_Y - y));
    REQUIRE(Spiders::getBlock(16-1, y, 16+1).getSkyLight() == Block::SKYLIGHT_MAX-2 - (START_Y - y));
    REQUIRE(Spiders::getBlock(16+1, y, 16-1).getSkyLight() == Block::SKYLIGHT_MAX-2 - (START_Y - y));
    REQUIRE(Spiders::getBlock(16+1, y, 16+1).getSkyLight() == Block::SKYLIGHT_MAX-2 - (START_Y - y));
  }
  // third should be MAX-3
  for (int y = START_Y; y >= 1; y--) {
    REQUIRE(Spiders::getBlock(16-3, y, 16).getSkyLight() == Block::SKYLIGHT_MAX-3 - (START_Y - y));
    REQUIRE(Spiders::getBlock(16+3, y, 16).getSkyLight() == Block::SKYLIGHT_MAX-3 - (START_Y - y));
  }

  // the solid block should be blocking sunlight
  // side of the block:
  REQUIRE(Spiders::getBlock(16+2, START_Y+1, 16).getSkyLight() == 12);
  // between top and side
  REQUIRE(Spiders::getBlock(16+2, START_Y+2, 16).getSkyLight() == 11);
  // top side of the block
  REQUIRE(Spiders::getBlock(16+1, START_Y+2, 16).getSkyLight() == 10);

  // place block right on sunray
  Spiders::setBlock(16, START_Y, 16, Block(SOLID));
  for (int x = 0; x < 32; x++)
  for (int z = 0; z < 32; z++)
  for (int y = START_Y; y >= 1; y--) {
    REQUIRE(Spiders::getBlock(x, y, z).getSkyLight() == 0);
  }

}
