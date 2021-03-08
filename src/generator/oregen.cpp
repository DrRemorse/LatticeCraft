#include "terragen.hpp"
#include "blocks.hpp"
#include "random.hpp"
#include <game.hpp>
#include <library/log.hpp>
#include <rapidjson/document.h>

using namespace library;

namespace terragen
{
  std::vector<OreInfo> OreGen::ores;

	void OreGen::init()
	{
    // load and apply the oregen JSON for each mod
    for (const auto& mod : cppcraft::game.mods())
    {
      for (const auto& mod_file : mod.json_files())
      {
        std::ifstream file(mod_file);
        const std::string str((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        rapidjson::Document doc;
        doc.Parse(str.c_str());
        CC_ASSERT(doc.IsObject(), "Oregen JSON must be valid");

        if (doc.HasMember("deposits"))
        {
          auto& data = doc["deposits"];
          for (auto itr = data.MemberBegin(); itr != data.MemberEnd(); ++itr)
          {
            CC_ASSERT(itr->value.IsObject(), "Ore deposit must be JSON object");
            const auto v = itr->value.GetObject();
            const int MAX_HEIGHT = v["max_height"].GetInt();
            const int CL_MIN = v["min_cluster"].GetInt();
            const int CL_MAX = v["max_cluster"].GetInt();
            const int COUNT  = v["count"].GetInt();
            // add to ore database
            OreGen::add({db::getb(itr->name.GetString()),
                        MAX_HEIGHT, CL_MIN, CL_MAX, COUNT});
          }
        }
      } // mod_file
    } // mod
    logger << "* OreGen loaded " << OreGen::size() << " ores" << Log::ENDL;
	}

  void OreGen::begin_deposit(gendata_t* gdata)
	{
    int counter = 0;
    for (size_t i = 0; i < ores.size(); i++)
    {
      const auto& ore = OreGen::get(i);

      for (int cl = 0; cl < ore.max_clusters; cl++)
      {
        int x = ihash(counter + 256, gdata->wx, gdata->wz) % BLOCKS_XZ;
        int z = ihash(counter - 256, gdata->wx, gdata->wz) % BLOCKS_XZ;

        const int ground = gdata->flatl(x, z).groundLevel;
        int y = ihash(counter, x, z) % std::min(ore.min_depth, ground);

    		OreGen::deposit(gdata, ore, x, y, z);
        counter++;
      }
    }
	}

	void OreGen::deposit(gendata_t* gdata, const OreInfo& ore, int x, int y, int z)
	{
		// find number of blocks to deposit
		const int count =
        ore.cluster_min + (ihash(x+40, y-10, z-30) % (ore.cluster_max - ore.cluster_min));

		for (int i = 0; i < count; i++)
		{
			Block& block = gdata->getb(x, y, z);
			if (block.getID() != STONE_BLOCK) return;

			block.setID(ore.block_id);

      // find next direction
			int dir = ihash(x-15, y-4, z+12) % 64;
			if (dir &  1) z++;
			if (dir &  2) z--;
			if (dir &  4) y++;
			if (dir &  8) y--;
			if (dir & 16) x++;
			if (dir & 32) x--;
			// prevent going outside bounds
			if (x < 0 || z < 0 || y < 1 || x >= BLOCKS_XZ || z >= BLOCKS_XZ) return;
		}
	}
}
