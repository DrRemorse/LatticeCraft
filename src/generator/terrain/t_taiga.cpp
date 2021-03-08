#include "../blocks.hpp"

#include "terrains.hpp"
#include "../terragen.hpp"
#include "../blocks.hpp"
#include "../random.hpp"
#include "../biomegen/biome.hpp"
#include "../postproc.hpp"
#include <glm/gtc/noise.hpp>
#include <library/bitmap/colortools.hpp>
#include <library/math/toolbox.hpp>
#include <Simplex.h>

#include "../../player.hpp"
#include "../../sector.hpp"
#include "../../particles.hpp"
#include "../../tiles.hpp"
#include "../../sun.hpp"

using namespace glm;
using namespace cppcraft;
using namespace library;

namespace terragen
{
  static const float HEIGHT_3D = 0.2f;

  static glm::vec3 getground_taiga(vec2 p, float height)
  {
    p *= 0.0025f;
		float n = 0.5f + 0.5f * glm::simplex(p);
		return {WATERLEVEL_FLT + powf(n, 0.55) * 0.1f, 0.0f, 0.0f};
  }
	static float getnoise_taiga(vec3 p, const vec3 under)
	{
    p *= vec3(0.006f, 1.0f, 0.006f);
    float noise = 0.5f + 0.5f * Simplex::worleyfBm(p, 4, 1.2f);
		return p.y - under.x + HEIGHT_3D * (noise - 1.0f);
	}

  static block_t SNOW_ID = 0;
  static block_t ICE_ID = 0;
	static int taiga_process(gendata_t* gdata, int x, int z, const int MAX_Y, const int)
	{
		const int wx = gdata->wx * BLOCKS_XZ + x;
		const int wz = gdata->wz * BLOCKS_XZ + z;

		// count current form of dirt/sand etc.
		int soilCounter = 0;
		// start counting from top (pretend really high)
		int air = BLOCKS_Y; // simple _AIR counter

		for (int y = MAX_Y; y > 0; y--)
		{
			Block& block = gdata->getb(x, y, z);
      // exit early if we reached stone
      if (block.getID() == STONE_BLOCK) return y;

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
				else if (block.getID() != BEACH_BLOCK)
				{
					// from soil to full-snow
					block.setID(SNOW_ID);
				}
			}
			else soilCounter = 0;

			// check if ultradifferent
			if (air >= BLOCKS_Y)
			{
				///-////////////////////////////////////-///
				///- create objects, and litter crosses -///
				///-////////////////////////////////////-///
				if (block.getID() == SOIL_BLOCK)
					    block.setID(SNOW_ID);

				/// terrain specific objects ///
				// TODO: use poisson disc here
				float rand = randf(wx, y, wz);
				if (rand < 0.1)
				{
					// set some bs winter-cross
				}
			}
			if (air > 0 && block.getID() == WATER_BLOCK)
			{
				block.setID(ICE_ID);
			}
      // count air
      if (block.isAir()) air++; else air = 0;
		} // y
    return 1;
	}

	void terrain_taiga_init()
	{
    auto& terrain =
		  terrains.add("taiga", "Taiga", Biome::biome_t{0.15f, 0.2f, 0.5f},
      getground_taiga, HEIGHT_3D, getnoise_taiga, taiga_process);

    SNOW_ID = db::getb("snow");
    ICE_ID  = db::getb("ice");

		terrain.setFog(glm::vec4(1.0f, 1.0f, 1.0f, 0.7f), 140);
    //terrain.music_name = "amb_winter";

		// snow particle
		const int P_SNOW = particleSystem("snowflake");

		terrain.on_tick =
    [P_SNOW] (double, GridWalker& walker)
		{
      glm::vec3 position(walker.getX(), walker.flat().skyLevel, walker.getZ());

			// every time we tick this piece of shit, we create some SNOW YEEEEEEEEEEEEEE
			for (int i = 0; i < 5; i++)
			{
				// use skylevel as particle base height
				glm::vec3 p = position + glm::vec3(rndNorm(16), 14 + rndNorm(10), rndNorm(16));

				// now create particle
				particleSystem.newParticle(p, P_SNOW);
			}
		};

		// Terrain reddish
		terrain.setColor(Biomes::CL_GRASS,
		[] (uint16_t, uint8_t, glm::vec2)
		{
			return RGBA8(146, 90, 0, 255);
		});
		// Trees
		terrain.setColor(Biomes::CL_TREES_A,
		[] (uint16_t, uint8_t, glm::vec2)
		{
			return RGBA8(60, 86, 0, 255);
		});
    terrain.copyColor(Biomes::CL_TREES_B, Biomes::CL_TREES_A);
		// Light-gray stones
		terrain.colors[Biomes::CL_STONE] =
		[] (uint16_t, uint8_t, glm::vec2)
		{
			return RGBA8(180, 180, 180, 255);
		};
    // Wasser color
    terrain.setColor(Biomes::CL_WATER,
		[] (uint16_t, uint8_t, glm::vec2)
		{
			return RGBA8(42, 73, 87, 255);
		});

	}
}
