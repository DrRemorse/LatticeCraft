#include "../terragen.hpp"
#include "../blocks.hpp"
#include "../random.hpp"
#include "../postproc.hpp"
#include "helpers.hpp"
#include "terrains.hpp"
#include <library/bitmap/colortools.hpp>
#include <library/noise/cosnoise.hpp>
#include <glm/gtc/noise.hpp>
#include <Simplex.h>

using namespace glm;
using namespace cppcraft;
using namespace library;

namespace terragen
{
  static const float DESERT_HEIGHT3D = 1.0f;

  static float pillarize(vec3 p, float noise)
  {
    const vec2 p2(p.x, p.z);
    float x = glm::simplex(p2 * 0.2f) + glm::simplex(p2 * 0.3f);
		x *= 0.5; // normalize

		const float EDGE = 0.45;
		const float RAMP_EDGE = 0.60;
		if (x > EDGE)
		{
			float linear = (x - EDGE) / (1.0 - EDGE);
			linear = library::hermite(linear);

			// ramp up the value
			float power = std::pow(linear, 0.25 - linear * 0.15);
			// apply height
			float height = power * 0.35 + glm::simplex(p2 * 0.7f) * 0.01;

			if (x > RAMP_EDGE)
			{
		    //return noise-0.15f;
        return noise-height;
			}
			else
			{
				x = (x - EDGE) / (RAMP_EDGE - EDGE);
				return noise - x * x * height * 0.6;
			}
    }
    return noise;
  }

  static glm::vec3 getground_desert(vec2 p, float height)
  {
    p *= 0.01f;
    float n1d = 0.5f + 0.5f * Simplex::noise(p.y*0.1f);
    float s = barchans(p.x + n1d, p.y * 0.3f);
		float n = s * 0.05f;
    float h = (WATERLEVEL_FLT + height) * 0.5f;

		return {h + n, 0.0f, 0.0f};
  }
  static float getnoise_desert(vec3 p, vec3 value)
	{
		p.x *= 0.01f;
		p.z *= 0.01f;

		return pillarize(p, p.y - value.x);
	}

  static block_t DESERT_BLOCK;
  static block_t CACTUS_BLOCK;
  static block_t CROSS_CACTUS_ID;
	static int process_desert(gendata_t* gdata, int x, int z, const int MAX_Y, const int)
	{
		const int wx = gdata->wx * BLOCKS_XZ + x;
		const int wz = gdata->wz * BLOCKS_XZ + z;

    int soil = 0;
		// start counting from top
		int air = BLOCKS_Y - MAX_Y;

		for (int y = MAX_Y; y > 0; y--)
		{
			Block& block = gdata->getb(x, y, z);

			if (block.getID() == SOIL_BLOCK)
			{
          soil++;
          if (soil >= 4)
              block.setID(STONE_BLOCK);
          else
              block.setID(DESERT_BLOCK);
      }
      else soil = 0;

      // remove beach/water
      if (block.getID() == WATER_BLOCK)
      {
        block.setID(_AIR);
      }
      if (block.getID() == BEACH_BLOCK)
      {
        block.setID(DESERT_BLOCK);
      }

			// place greenery when enough air
			if (air > 8)
			{
				///-////////////////////////////////////-///
				///- create objects, and litter crosses -///
				///-////////////////////////////////////-///
				if (block.getID() == DESERT_BLOCK)
				{
          /// terrain specific objects ///
					const float rand = randf(wx, y, wz);
					if (rand < 0.0005 && air > 16 && y < 140)
					{
            // TODO: cactus
            for (int h = 1; h < 4; h++)
              gdata->getb(x, y+h, z).setID(CACTUS_BLOCK);
            // TODO: oasis with palms
					}
					else if (rand > 0.997)
					{
						gdata->getb(x, y+1, z).setID(CROSS_CACTUS_ID);
					}
				}
			}
      // count air
      if (block.isAir()) air++; else air = 0;
		} // y
    return 1;
	}

	void terrain_desert_init()
	{
    DESERT_BLOCK = db::getb("desert_sand");
    CACTUS_BLOCK = db::getb("cactus_block");
    CROSS_CACTUS_ID = db::getb("cactus_flower");

		auto& terrain =
		terrains.add("desert", "Desert Dunes", Biome::biome_t{0.8f, 0.2f, 0.4f},
        getground_desert, DESERT_HEIGHT3D, getnoise_desert, process_desert);

    terrain.setFog(glm::vec4(0.87f, 0.83f, 0.7f, 1.0f), 180);
		terrain.on_tick =
		[] (double, GridWalker&)
		{
			// ... particles here & there
		};

		// Grass color
		terrain.setColor(Biomes::CL_GRASS,
		[] (uint16_t, uint8_t, glm::vec2 p)
		{
			float v = glm::simplex(p * 0.004f) + glm::simplex(p * 0.008f);
			return RGBA8(22 + v * 10.0f, 127 - v * 30.0f, 7, 255);
		});
		// Trees
		terrain.setColor(Biomes::CL_TREES_A,
		[] (uint16_t, uint8_t, glm::vec2 p)
		{
			float v = glm::simplex(p * 0.007f) * glm::simplex(p * 0.005f);
			return RGBA8(62 + v * 32.0f, 127 + v * 30.0f, 37 + v * 37, 255);
		});
    terrain.setColor(Biomes::CL_TREES_B,
		[] (uint16_t, uint8_t, glm::vec2)
		{
			return RGBA8(50, 84, 20, 255);
		});
		// Stone color
		terrain.setColor(Biomes::CL_STONE,
		[] (uint16_t, uint8_t, glm::vec2)
		{
			return RGBA8(128, 128, 128, 255);
		});
    // Wasser color
		terrain.setColor(Biomes::CL_WATER,
		[] (uint16_t, uint8_t, glm::vec2)
		{
			return RGBA8(62, 127, 107, 255);
		});

	} // _init();
}
