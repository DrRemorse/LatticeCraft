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
  static const float COSN_HEIGHT = 0.15f;

  static glm::vec3 getground_jungle(vec2 p, float height)
  {
    p *= 0.001f;
    height = (WATERLEVEL_FLT + height) * 0.5f - 0.1f;

		const float land = simplex(p)         * 0.3f
          + simplex(p * vec2(2.7f, 2.8f)) * 0.2f
          + simplex(p * vec2(5.8f, 5.6f)) * 0.5f;

    const float EDGE = 0.3f;
    const float FACT = (land - EDGE) / (1.0f - EDGE);
    if (land > EDGE) height += powf(FACT, 0.5f) * 0.2f;

		return {height, 0.0f, 0.0f};
  }
  static float getnoise_jungle(vec3 p, glm::vec3 under)
	{
    vec3 N = p * vec3(0.005f, 4.0f, 0.005f);
    float n1 = Simplex::fBm(N);
    float noise = (cosnoise(N, n1, 1.0f, 1.0f, 2.0f + n1, 1.0f, 1.0f) + 1.0f) * 0.5f;
		return p.y - under.x - COSN_HEIGHT + COSN_HEIGHT * noise;
	}

  static block_t GRASS_BLOCK;
  static block_t PUMPKIN_BLOCK;
  static block_t CROSS_GRASS_ID;
	static int process_jungle(gendata_t* gdata, int x, int z, const int MAX_Y, const int)
	{
		const int wx = gdata->wx * BLOCKS_XZ + x;
		const int wz = gdata->wz * BLOCKS_XZ + z;

		// count current form of dirt/sand etc.
		int soilCounter = 0;
    // start counting from top
    block_t lastID    = _AIR;
    int     lastCount = BLOCKS_Y;

		for (int y = MAX_Y; y > 0; y--)
		{
			Block& block = gdata->getb(x, y, z);

			// we only count primary blocks produced by generator,
			// which are specifically greensoil & sandbeach
			if (block.getID() == SOIL_BLOCK || block.getID() == BEACH_BLOCK)
			{
				soilCounter++;

				// making stones under water level has priority!
				if (y < WATERLEVEL && soilCounter > PostProcess::STONE_CONV_UNDER)
				{
					block.setID(STONE_BLOCK);
				}
				else if (soilCounter > PostProcess::STONE_CONV_OVERW)
				{
					block.setID(STONE_BLOCK);
				}
			}
      else if (block.getID() == STONE_BLOCK && lastID == _AIR)
      {
        // we are done
        return y;
      }
			else soilCounter = 0;

			// place greenery when enough air
			if (lastID == _AIR && lastCount > 4)
			{
				///-////////////////////////////////////-///
				///- create objects, and litter crosses -///
				///-////////////////////////////////////-///
				if (block.getID() == SOIL_BLOCK)
				{
					block.setID(GRASS_BLOCK);

					// TODO: use poisson disc here
					const float rand = randf(wx, y, wz);
          const glm::vec2 p = gdata->getBaseCoords2D(x, z);

					/// terrain specific objects ///
					if (rand < 0.01 && lastCount > 40)
					{
						if (glm::simplex(p * 0.01f) < -0.2)
						{
							unsigned height = 40 + randf(wx, y-1, wz) * 15;
							if (y + height < WATERLEVEL + 80)
							{
								gdata->add_object("jungle_tree", wx, y+1, wz, height);
							}
						}
					}
					else if (rand > 0.65)
					{
						// note: this is an inverse of the forest noise
            float n = glm::simplex(p * 0.005f);
            if (n > 0.9) {
              if (rand > 0.99)
                gdata->getb(x, y+1, z).setID(PUMPKIN_BLOCK);
            } else if (n > -0.1) {
							gdata->getb(x, y+1, z).setID(CROSS_GRASS_ID);
						}
					}
				}
			}
      if (block.getID() != lastID)
      {
        lastID = block.getID();
        lastCount = 1;
      }
      else lastCount++;
		} // y
    return 1;
	}

	void terrain_jungle_init()
	{
    GRASS_BLOCK = db::getb("grass_block");
    PUMPKIN_BLOCK = db::getb("pumpkin_block");
    CROSS_GRASS_ID = db::getb("cross_grass");

		auto& terrain =
		terrains.add("jungle", "Jungle", Biome::biome_t{0.8f, 0.8f, 0.3f},
        getground_jungle, COSN_HEIGHT, getnoise_jungle, process_jungle);

    terrain.setFog(glm::vec4(0.9f, 1.0f, 0.9f, 0.5f), 130);
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
