#include "../items/item.hpp"

#include "../game.hpp"
#include "../tiles.hpp"
#include "../voxelmodels.hpp"
#include <library/bitmap/colortools.hpp>
#include <rapidjson/document.h>
#include <fstream>
#undef NDEBUG
#include <cassert>

using namespace library;
using namespace db;

namespace terragen
{
	using cppcraft::tiledb;
  using cppcraft::VoxelModels;

  static void
  parse_item(const std::string name, const rapidjson::Value& v)
  {
    auto& item = ItemDB::get().create(name);
    // default name function
    assert(v.HasMember("name"));
    std::string m_name(v["name"].GetString());
    item.setName(ItemData::name_func_t::make_packed(
      [m_name] (const Item&) {
        return m_name;
      }));
    // tile
    assert(v.HasMember("tile"));
    {
      const char* tile = v["tile"].GetString();
      item.setTileID(tiledb.items(tile));
    }
    // voxelize
    if (v.HasMember("voxelize"))
    {
      item.setVoxelModel(
        VoxelModels::create(tiledb.items.diffuse(), item.getTileID())
      );
    }
    // rotation
    if (v.HasMember("hand_rotation"))
    {
      auto rot = v["hand_rotation"].GetArray();
      assert(rot.Size() == 3);
      item.setHandRotation({rot[0].GetFloat(), rot[1].GetFloat(), rot[2].GetFloat()});
    }
    // translation
    if (v.HasMember("hand_translation"))
    {
      auto pos = v["hand_translation"].GetArray();
      assert(pos.Size() == 3);
      item.setHandTranslation({pos[0].GetFloat(), pos[1].GetFloat(), pos[2].GetFloat()});
    }
    // activation
    if (v.HasMember("activation"))
    {
      auto act = v["activation"].GetArray();
      assert(act.Size() == 2);
      const std::string mode = act[0].GetString();
      if (mode == "build")
        item.setActivation(ItemData::ACT_BUILD, act[1].GetString());
      else {
        printf("Unknown activation mode: %s\n", mode.c_str());
      }
    }

  }

	void init_items()
	{
		auto& db = ItemDB::get();

    // ** the first item *MUST* be IT_NONE ** //
    ItemData& none = db.create("broken");
		assert(none.getID() == 0);
		none.setName([] (const Item&) { return "(none)"; });

    // load and apply the tiles JSON for each mod
    for (const auto& mod : cppcraft::game.mods())
    {
      for (const auto& mod_file : mod.json_files())
      {
        std::ifstream file(mod_file);
        const std::string str((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        rapidjson::Document doc;
        doc.Parse(str.c_str());

        CC_ASSERT(doc.IsObject(), "Items JSON must be valid");
        if (doc.HasMember("items"))
        {
          auto& obj = doc["items"];
          for (auto itr = obj.MemberBegin(); itr != obj.MemberEnd(); ++itr)
          {
            CC_ASSERT(itr->value.IsObject(), "Item must be JSON object");
            const std::string name = itr->name.GetString();
            const auto& v = itr->value.GetObject();

            try
            {
              parse_item(name, v);
            }
            catch (std::exception& e)
            {
              printf("Error parsing item %s: %s\n", name.c_str(), e.what());
              throw;
            }
          }
        }
      } // mod_file
    } // mod
    printf("* Loaded %zu items\n", db.size());
	}
}
