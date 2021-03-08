#include "minimap.hpp"

#include <library/log.hpp>
#include <library/bitmap/bitmap.hpp>
#include <library/bitmap/colortools.hpp>
#include <library/opengl/vao.hpp>
#include <library/opengl/texture.hpp>
#include <grid_walker.hpp>
#include <biomes.hpp>
#include "player.hpp"
#include "sectors.hpp"
#include "seamless.hpp"
#include "shaderman.hpp"
#include "spiders.hpp"
#include <cstring>
#include <mutex>

using namespace library;

namespace cppcraft
{
  static const int PIX_PER_SECT = 2;
	// the one and only Minimap(TM)
	Minimap minimap;
	VAO minimapVAO;
	std::mutex minimapMutex;

	Minimap::Minimap()
	{
		this->bitmap  = nullptr;
		this->texture = nullptr;
	}

	void Minimap::init()
	{
		logger << Log::INFO << "* Initializing minimap" << Log::ENDL;

		// 32-bits, one pixel per sector on (X, Z) axes
		bitmap = new Bitmap(sectors.getXZ() * PIX_PER_SECT,
                        sectors.getXZ() * PIX_PER_SECT);
		memset(bitmap->data(), 0, bitmap->getWidth() * bitmap->getHeight() * sizeof(Bitmap::rgba8_t));
		// create texture
		texture = new Texture(GL_TEXTURE_2D);
		texture->create(*bitmap, true, GL_REPEAT, GL_LINEAR, GL_LINEAR);

		struct minimap_vertex_t
		{
			GLfloat x, y, z;
		};

		// vertices
		const minimap_vertex_t vertices[4] =
		{
			{ -0.5, -0.5, 0.0 },
			{  0.5, -0.5, 0.0 },
			{  0.5,  0.5, 0.0 },
			{ -0.5,  0.5, 0.0 }
		};

		// vertex array object
		minimapVAO.begin(sizeof(minimap_vertex_t), 4, vertices);
		minimapVAO.attrib(0, 3, GL_FLOAT, GL_FALSE, 0);
		minimapVAO.end();
	}

	void Minimap::update(double px, double pz)
	{
		// minimap subpixel offset
    glm::vec2 sub_offset(
      (px - (sectors.getXZ() * Sector::BLOCKS_XZ / PIX_PER_SECT)) / Seamless::OFFSET * PIX_PER_SECT,
      (pz - (sectors.getXZ() * Sector::BLOCKS_XZ / PIX_PER_SECT)) / Seamless::OFFSET * PIX_PER_SECT
    );

		minimapMutex.lock();
    this->render_offset = glm::vec2(this->offset.x, this->offset.y) + sub_offset;

		// update synchronization
		if (this->needs_update)
		{
			this->needs_update = false;
			// bind minimap texture
			texture->bind(0);
			// re-upload pixel data (and auto-generate mipmaps)
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, texture->getWidth(), texture->getHeight(), 0, GL_RGBA, GL_UNSIGNED_BYTE, bitmap->data());
			// done
		}
		minimapMutex.unlock();
	}

	void Minimap::render(glm::mat4& mvp)
	{
		Shader& shd = shaderman[Shaderman::MINIMAP];

		shd.bind();
		shd.sendMatrix("matprojview", mvp);

		// position intra-block offset
    glm::vec2 shd_offset = this->render_offset;
		shd_offset = glm::vec2(0.5f + shd_offset.x / bitmap->getWidth(),
                           0.5f + shd_offset.y / bitmap->getHeight());
		shd.sendVec2("offset", shd_offset);

		// bind minimap texture
		texture->bind(0);
		// render minimap
		minimapVAO.render(GL_QUADS);
	}

  static Bitmap::rgba8_t mixColor(Bitmap::rgba8_t a, Bitmap::rgba8_t b, float mixlevel)
	{
		if (a == b) return a;

		unsigned char* p = (unsigned char*)&a;
		unsigned char* q = (unsigned char*)&b;

		for (unsigned int i = 0; i < sizeof(a); i++)
		{
			p[i] = uint8_t(p[i] * (1.0 - mixlevel) + q[i] * mixlevel);
		}
		return a;
	}

  static const float ELEVATION_MEAN   = WATERLEVEL + 16;
  static const float ELEVATION_FACTOR = 0.7f;

	static Bitmap::rgba8_t elevationColor(Bitmap::rgba8_t c, float dy)
	{
		unsigned char* p = (unsigned char*)&c;
		// result = source + dy * luminance
		p[0] = glm::clamp(int(p[0] + dy), 0, 255);
		p[1] = glm::clamp(int(p[1] + dy), 0, 255);
		p[2] = glm::clamp(int(p[2] + dy), 0, 255);
		return c;
	}

	static Bitmap::rgba8_t getBlockColor(Sector& sector, int x, int z)
	{
		// get the topmost block at location
		const int y = sector.flat()(x, z).skyLevel-1;
    const auto& blk = sector(x, y, z);
		const auto& db  = blk.db();
		// determine the minimap color
    uint32_t c;
    if (db.isMinimapIndexColored()) {
      c = sector.flat()(x, z).fcolor[db.getMinimapColor()];
    } else {
      GridWalker walker(sector, x, y, z);
		  c = db.getMinimapColor(blk, walker);
    }

		// basic elevation coloring
		return elevationColor(c, (y - ELEVATION_MEAN) * ELEVATION_FACTOR);
	}

	// addSector: called from Generator::generate()
	// each time block data at skylevel is updated, this function COULD be called
	// one solution is to wait for 8 block changes, disregard skylevel, and update
	// the algorithm would be sectorblock::version % 8
	void Minimap::addSector(Sector& sector)
	{
		// read certain blocks from sector, and determine pixel value
		// set pixel value in the correct 2x2 position on pixel table
		Bitmap::rgba8_t colors[4];
    /*
		colors[0] = mixColor( //  sector ( x,  z )
					getBlockColor(sector,  3,  3),
					getBlockColor(sector,  4,  4), 0.5);

		colors[1] = mixColor(
					getBlockColor(sector,  3, 12),
					getBlockColor(sector,  4, 11), 0.5);

		colors[2] = mixColor(
					getBlockColor(sector, 12,  3),
					getBlockColor(sector, 11,  4), 0.5);

		colors[3] = mixColor(
					getBlockColor(sector, 12, 12),
					getBlockColor(sector, 11, 11), 0.5);
    */
    colors[0] = getBlockColor(sector, 3, 3);
    colors[1] = getBlockColor(sector, 3, 12);
    colors[2] = getBlockColor(sector, 12, 3);
		colors[3] = getBlockColor(sector, 12, 12);

		// set final color @ pixel (px, pz)
    const int px = (sector.getX() * PIX_PER_SECT + offset.x) % bitmap->getWidth();
    const int py = (sector.getZ() * PIX_PER_SECT + offset.y) % bitmap->getHeight();

		Bitmap::rgba8_t* pixels = bitmap->data();
		const int scan = bitmap->getWidth();

		pixels[ py      * scan + px] = colors[0];
		pixels[(py + 1) * scan + px] = colors[1];
		pixels[ py      * scan + px + 1] = colors[2];
		pixels[(py + 1) * scan + px + 1] = colors[3];
	}
  void Minimap::setUpdated() noexcept
  {
    std::lock_guard<std::mutex> lock(minimapMutex);
    // will see it, eventually
		this->needs_update = true;
  }

	void Minimap::roll(int x, int z) noexcept
	{
    std::lock_guard<std::mutex> lock(minimapMutex);
    this->offset += glm::ivec2(x, z) * PIX_PER_SECT;
    if (offset.x < 0) offset.x += bitmap->getWidth();
    if (offset.y < 0) offset.y += bitmap->getHeight();
	}
}
