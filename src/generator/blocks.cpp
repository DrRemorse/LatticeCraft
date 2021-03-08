#include "blocks.hpp"

#include "biomegen/biome.hpp"
#include "simulation/auto_growth.hpp"
#include "../game.hpp"
#include "../renderconst.hpp"
#include "../tiles.hpp"
#include <grid_walker.hpp>
#include <library/bitmap/colortools.hpp>
#include <rapidjson/document.h>
#include <fstream>
#include <cassert>

using namespace library;
using namespace db;

namespace terragen
{
  // the essential blocks
  block_t STONE_BLOCK, SOIL_BLOCK, BEACH_BLOCK, WATER_BLOCK;
  block_t BEDROCK, MOLTEN_BLOCK, LAVA_BLOCK;

	using cppcraft::Sector;
	using cppcraft::tiledb;
	using cppcraft::RenderConst;

	static inline int getDepth(GridWalker& walker, const block_t ID)
	{
    const int MAX_Y = walker.getY();
		for (int y = MAX_Y;y > 0; y--) {
      if (walker.get().getID() != ID) return MAX_Y - y;
      walker.move_y(-1);
    }
		return 1;
	}

  static db::BlockData&
  create_from_type(const std::string& name, const std::string& type)
  {
    if (type == "solid") return BlockData::createSolid(name);
    if (type == "fluid") return BlockData::createFluid(name);
    if (type == "leaf") return BlockData::createLeaf(name);
    if (type == "cross") return BlockData::createCross(name);
    throw std::out_of_range("Not acceptable JSON block type " + type + " for block " + name);
  }
  static uint32_t color_index_from(const std::string& type)
  {
    if (type == "stone")  return Biomes::CL_STONE;
    if (type == "soil")   return Biomes::CL_SOIL;
    if (type == "gravel") return Biomes::CL_GRAVEL;
    if (type == "sand")   return Biomes::CL_SAND;
    if (type == "grass")  return Biomes::CL_GRASS;
    if (type == "trees_a") return Biomes::CL_TREES_A;
    if (type == "trees_b") return Biomes::CL_TREES_B;
    if (type == "water")  return Biomes::CL_WATER;
    throw std::out_of_range("Not acceptable color index type: " + type);
  }
  static uint32_t color_index_from(const rapidjson::Value& obj)
  {
    if (obj.IsArray()) {
      return RGBA8(obj[0].GetInt(), obj[1].GetInt(), obj[2].GetInt(), obj[3].GetInt());
    }
    return color_index_from(obj.GetString());
  }
  static void set_tiling_function(db::BlockData& block,
                                  const cppcraft::tile_database& tiles,
                                  const rapidjson::Value& v)
  {
    // a single string means a single tile for all sides
    if (v.IsString())
    {
      const std::string id = v.GetString();
      block.useTileID(tiles(id));
    }
    // array can mean (top, side, bottom) or (top, side, face, bottom)
    else if (v.IsArray())
    {
      auto array = v.GetArray();
      if (array.Size() == 3)
      {
        const block_t top = tiles(array[0].GetString());
        const block_t side = tiles(array[1].GetString());
        const block_t bottom = tiles(array[2].GetString());
        block.useTextureFunction(
        [top, side, bottom] (const Block&, uint8_t face)
        {
          if (face == 2) return top;
          if (face != 3) return side;
          return bottom;
        });
      }
      else if (array.Size() == 4)
      {
        const block_t front = tiles(array[0].GetString());
        const block_t top   = tiles(array[1].GetString());
        const block_t side  = tiles(array[2].GetString());
        const block_t bottom = tiles(array[3].GetString());
        block.useTextureFunction(
        [front, top, side, bottom] (const Block& blk, uint8_t face)
        {
          if (blk.getBits() == face) return front;
          if (face == 2) return top;
          if (face != 3) return side;
          return bottom;
        });
      }
      else {
        throw std::out_of_range("Tile array must have 3 or 4 elements.");
      }
    }
    else {
      throw std::out_of_range("Tile identity had incorrect type. Must be string or array.");
    }
  }

  static void
  parse_block(const std::string name, const rapidjson::Value& v)
  {
    // type must be specified
    CC_ASSERT(v.HasMember("type"), "Blocks must have the 'type' field");
    auto& block = create_from_type(name, v["type"].GetString());
    // name
    std::string m_name(v["name"].GetString());
    block.getName =
    BlockData::name_func_t::make_packed(
      [m_name] (const Block&) {
        return m_name;
      });
    // model
    if (v.HasMember("model")) {
      block.setModel(v["model"].GetInt());
    }
    // shader
    if (v.HasMember("shader")) {
      block.shader = v["shader"].GetInt();
    }
    // color
    if (v.HasMember("color")) {
      uint32_t color = color_index_from(v["color"]);
      if (color < 256) block.setColorIndex(color);
      else block.getColor = [color] (const Block&) { return color; };
    }
    // minimap
    if (v.HasMember("minimap")) {
      uint32_t color = color_index_from(v["minimap"]);
      block.setMinimapColor(color);
    }
    // tile
    if (v.HasMember("tile"))
    {
      auto& tile = v["tile"];
      CC_ASSERT(tile.IsArray(), "JSON tile field can only contain array");
      // tile type and ID
      const std::string type = tile[0].GetString();
      //
      if (type == "tiles")
      {
        bool is_connected = false;
        // subtype check
        if (tile.Size() == 3)
        {
          const std::string id = tile[1].GetString();
          const std::string attr = tile[2].GetString();
          if (attr == "connected") {
            // gather tiles
            int tile_ids[10];
            tile_ids[0] = tiledb.tiles(id);
            for (int i = 1; i < 10; i++) {
              tile_ids[i] = tiledb.tiles(id + "_" + std::to_string(i));
            }
            // create basic CT function
            block.useConnectedTexture(
              BlockData::conntex_func_t::make_packed(
              [tile_ids] (const connected_textures_t& ct, uint8_t face) -> short
              {
                // TODO: write me
                return 0;
              }));
            is_connected = true;
          } else {
            throw std::out_of_range("Not acceptable tiling attribute: " + attr);
          }
        }
        // regular tiles
        if (is_connected == false)
        {
          set_tiling_function(block, tiledb.tiles, tile[1]);
        }
      } // tiles
      else if (type == "big" || type == "bigtiles")
      {
        block.repeat_y = false;
        if (tile.Size() == 3)
        {
          const std::string attr = tile[2].GetString();
          // repeating tiles
          if (attr == "repeat") {
            block.repeat_y = true;
          } else {
            throw std::out_of_range("Not acceptable tiling attribute: " + attr);
          }
        } // attr

        set_tiling_function(block, tiledb.bigtiles, tile[1]);
        block.shader = cppcraft::RenderConst::TX_REPEAT;
      }
    }
    // sound
    if (v.HasMember("sound")) {
      const std::string sound = v["sound"].GetString();
      block.getSound =
      BlockData::sound_func_t::make_packed(
        [sound] (const Block&) { return sound;
      });
    }
    // light emission value
    if (v.HasMember("light")) {
      block.setLightColor(v["light"].GetInt(), 0, 0);
    }
  }

	void init_blocks()
	{
		auto& db = BlockDB::get();

    BlockData& air = db.create("air");
    // ** the first Block *MUST* be _AIR ** //
		assert(air.getID() == 0);
		air.transparent = true;
    air.setBlock(false);
		air.blocksMovement = [] (const Block&) { return false; };
		air.forwardMovement = [] (const Block&) { return true; };
		air.getColor = [] (const Block&) { return 255; };
		air.getName  = [] (const Block&) { return "Air"; };
		// you can never hit or select _AIR
		air.physicalHitbox3D = [] (const Block&, float, float, float) { return false; };
		air.selectionHitbox3D = [] (const Block&, float, float, float) { return false; };
		air.transparentSides = BlockData::SIDE_ALL;

    // load and apply the tiles JSON for each mod
    for (const auto& mod : cppcraft::game.mods())
    {
      for (const auto& mod_file : mod.json_files())
      {
        std::ifstream file(mod_file);
        const std::string str((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        rapidjson::Document doc;
        doc.Parse(str.c_str());

        CC_ASSERT(doc.IsObject(), "Blocks JSON must be valid");
        if (doc.HasMember("blocks"))
        {
          auto& obj = doc["blocks"];
          for (auto itr = obj.MemberBegin(); itr != obj.MemberEnd(); ++itr)
          {
            CC_ASSERT(itr->value.IsObject(), "Block must be JSON object");
            const std::string name = itr->name.GetString();
            const auto& v = itr->value.GetObject();

            try
            {
              parse_block(name, v);
            }
            catch (std::exception& e)
            {
              printf("Error parsing block %s: %s\n", name.c_str(), e.what());
              throw;
            }
          }
        }
      } // mod_file
    } // mod

		// create _WATER
		{
			auto& fluid = BlockData::createFluid();
      fluid.setColorIndex(Biomes::CL_WATER);
			fluid.useMinimapFunction(
  			[] (const Block&, GridWalker& walker)
  			{
          // measure ocean depth
  				const float depth = 1.0 - getDepth(walker, WATER_BLOCK) / 64.0;
  				// create gradiented ocean blue
  				return RGBA8(depth * depth * 62, depth*depth * 140, depth * 128, 255);
  			});
			fluid.getName = [] (const Block&) { return "Water"; };
			fluid.shader = RenderConst::TX_WATER;
      fluid.useTileID(tiledb.tiles("water"));
			db.assign("water", fluid);
		}
		// create _LAVA
		{
			auto& fluid = BlockData::createFluid();
			fluid.getColor = [] (const Block&) { return BGRA8(0, 0, 0, 0); };
			fluid.setMinimapColor(RGBA8(240, 140, 64, 255));
			fluid.getName = [] (const Block&) { return "Lava"; };
			fluid.shader = RenderConst::TX_LAVA;
      fluid.useTileID(tiledb.tiles("lava"));
			fluid.setBlock(false);
			fluid.transparent = false;
			fluid.setLightColor(10, 7, 3);
			db.assign("lava", fluid);
		}
    printf("* Loaded %zu blocks\n", db.size());

    // for now ...
    auto_growth(db::getb("soil"), db::getb("grass_block"));

    // initialize essential blocks
    BEDROCK      = db::getb("bedrock");
    STONE_BLOCK  = db::getb("stone");
    SOIL_BLOCK   = db::getb("soil");
    BEACH_BLOCK  = db::getb("beach");
    WATER_BLOCK  = db::getb("water");
    MOLTEN_BLOCK = db::getb("molten");
    LAVA_BLOCK   = db::getb("lava");
	}
}
