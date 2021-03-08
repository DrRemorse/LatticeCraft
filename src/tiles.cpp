#include "tiles.hpp"

#include <library/log.hpp>
#include <library/opengl/opengl.hpp>
#include <library/bitmap/colortools.hpp>
#include "game.hpp"
#include "gameconf.hpp"
#include <common.hpp>
#include <rapidjson/document.h>

using namespace library;

namespace cppcraft
{
	TileDB tiledb;

  static void
  parse_tile_database(tile_database& db, rapidjson::Value& obj)
  {
    CC_ASSERT(obj.HasMember("source"), "Tiles JSON object must have 'source'");
    const std::string source_file = obj["source"].GetString();

    CC_ASSERT(obj.HasMember("data"), "Tiles JSON object must have 'data'");
    auto& data = obj["data"];
    for (auto itr = data.MemberBegin(); itr != data.MemberEnd(); ++itr)
    {
      CC_ASSERT(itr->value.IsArray(), "Tile must be array of two or more coordinates");
      const auto v = itr->value.GetArray();
      const int DIFF_X = v[0].GetInt();
      const int DIFF_Y = v[1].GetInt();
      int TONE_X = 0;
      int TONE_Y = 0;
      if (v.Size() >= 4) {
        TONE_X = v[2].GetInt();
        TONE_Y = v[3].GetInt();
      }
      // add tile
      db.add_tile(itr->name.GetString(),
          source_file, DIFF_X, DIFF_Y, TONE_X, TONE_Y);
    }
  }

	void TileDB::init()
	{
		logger << Log::INFO << "* Initializing tiles" << Log::ENDL;

		// normal tiles
		tiles = tile_database(config.get("tiles.size", 32), true);
    // big tiles (4x)
    bigtiles = tile_database(tiles.tilesize() * TILES_PER_BIG_TILE, true);
    // item tiles
    items = tile_database(config.get("items.size", 32), false);
    // particle tiles
    particles = tile_database(config.get("partic.size", 32), false);

		// players
		this->skinSize = config.get("players.size", 32);

    // load and apply the tiles JSON for each mod
    for (const auto& mod : game.mods())
    {
      for (const auto& mod_file : mod.json_files())
      {
        std::ifstream file(mod_file);
        const std::string str((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        rapidjson::Document doc;
        doc.Parse(str.c_str());

        CC_ASSERT(doc.IsObject(), "Tiles JSON must be valid");
        if (doc.HasMember("block_tiles"))
        {
          auto& obj = doc["block_tiles"];
          parse_tile_database(tiledb.tiles, obj);
        }
        if (doc.HasMember("big_tiles"))
        {
          auto& obj = doc["big_tiles"];
          parse_tile_database(tiledb.bigtiles, obj);
        }
        if (doc.HasMember("item_tiles"))
        {
          auto& obj = doc["item_tiles"];
          parse_tile_database(tiledb.items, obj);
        }
        if (doc.HasMember("particle_tiles"))
        {
          auto& obj = doc["particle_tiles"];
          parse_tile_database(tiledb.particles, obj);
        }
      } // mod file
    } // mods
    printf("* Loaded %zu big tiles\n", tiledb.bigtiles.size());
    printf("* Loaded %zu tiles\n", tiledb.tiles.size());
    printf("* Loaded %zu item tiles\n", tiledb.items.size());
    printf("* Loaded %zu particle tiles\n", tiledb.particles.size());

    // free some memory
    this->unload_temp_store();
	}

  tile_database::tile_database(int tilesize, bool tone)
      : m_tile_size(tilesize), m_tone_enabled(tone)
  {
    m_diffuse.convert_to_tilesheet(tilesize, RGBA8(255, 0, 255, 255));
    if (m_tone_enabled) {
      m_tonemap.convert_to_tilesheet(tilesize, 0);
    }
  }

  void tile_database::assign(std::string name, const short TILE_ID)
  {
    CC_ASSERT(TILE_ID >= 0 && TILE_ID < m_diffuse.getTilesX(),
              "Tile ID must be within bounds of tilesheet");
    namedb.emplace(std::piecewise_construct,
                   std::forward_as_tuple(std::move(name)),
                   std::forward_as_tuple(TILE_ID));
  }

  void tile_database::add_tile(const std::string& name,
                const std::string& source_file,
                const int DIFF_X, const int DIFF_Y,
                const int TONE_X, const int TONE_Y)
  {
    const Bitmap& source_img = tiledb.get_bitmap(source_file);
    // generate tile ID
    const int TILE_ID = m_diffuse.getTilesX();
    // add to tile array
    m_diffuse.add_tile(source_img, DIFF_X, DIFF_Y);
    // if tonemap is enabled, use it always
    if (m_tone_enabled) {
      if (TONE_X == 0 && TONE_Y == 0)
        m_tonemap.add_tile(m_tonemap, 0, 0);
      else
        m_tonemap.add_tile(source_img, TONE_X, TONE_Y);
    }

    //printf("Tile %s has ID %d\n", name.c_str(), TILE_ID);
    this->assign(name, TILE_ID);
  }

  void tile_database::create_textures()
  {
    /// diffuse tileset ///
		m_diff_texture = Texture(GL_TEXTURE_2D_ARRAY);
		m_diff_texture.create(diffuse(), true, GL_REPEAT, GL_NEAREST, GL_LINEAR_MIPMAP_LINEAR);
		m_diff_texture.setAnisotropy(gameconf.anisotropy);
    glGenerateMipmap(GL_TEXTURE_2D_ARRAY);
		if (OpenGL::checkError()) throw std::runtime_error("Tile database: diffuse error");
    /// (optional) color dye tileset ///
    if (this->m_tone_enabled)
    {
      m_tone_texture = Texture(GL_TEXTURE_2D_ARRAY);
	    m_tone_texture.create(tonemap(), true, GL_REPEAT, GL_NEAREST, GL_LINEAR_MIPMAP_LINEAR);
      m_tone_texture.setAnisotropy(gameconf.anisotropy);
      glGenerateMipmap(GL_TEXTURE_2D_ARRAY);
      if (OpenGL::checkError()) throw std::runtime_error("Tile database: tonemap error");
    }
  }

  const Bitmap& TileDB::get_bitmap(const std::string& fname)
  {
    // create temp store on demand
    if (temp == nullptr) temp = std::make_unique<temp_store> ();
    // find bitmap if its there already
    auto it = temp->find(fname);
    if (it != temp->end()) {
      return it->second;
    }
    // if not, load it
    auto insit =
    temp->emplace(std::piecewise_construct,
                  std::forward_as_tuple(fname),
                  std::forward_as_tuple(fname, Bitmap::PNG));
    return insit.first->second; // iterator
  }

  void TileDB::unload_temp_store()
  {
    temp = nullptr;
  }
}
