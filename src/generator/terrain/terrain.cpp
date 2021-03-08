#include "terrain.hpp"

#include <common.hpp>
#include "../terragen.hpp"
#include "../blocks.hpp"
#include "terrains.hpp"
#include <glm/gtc/noise.hpp>
#include <library/math/toolbox.hpp>
#include <array>
#include <cassert>

using namespace cppcraft;
using namespace library;

namespace terragen
{
	static const float WATERHEIGHT = WATERLEVEL / float(BLOCKS_Y);

	inline float mix(float a, float b, float level)
	{
		return a * (1.0f - level) + b * level;
	}

	// produces basic blocks based on some input properties
	// such as world y-value, the variable beach-height, density of the point and whether we are inside caves
	Block Terrain::getBlock(float y, float in_beachhead, float density, float caves)
	{
		// FOR TESTING CAVES:
		//return (caves < 0.0) ? db::getb("stone") : _AIR;
		float cavetresh = 0.0f; // distance from air/dense barrier
		if (density > -0.15 && density <= -0.05) cavetresh = (density + 0.05) / -0.1;
		if (density > -0.025 && density <= 0.0) cavetresh = 1.0f;

		// caves
		const float cave_lower = 0.0; // underworld cave density treshold
		const float cave_upper = 0.0; // overworld  cave density treshold

		// lower = 0.0 to waterlevel + beachhead
		const float stone_lower = -0.1;
		const float stone_upper = -0.05; // density treshold for stone upper / lower hemisphere

		const float lava_height = 0.025;

		// middle = waterlevel + beachhead
		float beachhead  = in_beachhead * 0.025; // sand above water (0.0075 + ...)

		// upper = waterlevel + beachhead + lower_to_upper
		const float lower_to_upper  = 0.1;  // transition length from lower to upper

		if (density < 0.0)
		{
			if (y <= WATERHEIGHT + beachhead)
			{
				// lower hemisphere, dense

				if (UNLIKELY(caves + cavetresh < cave_lower))
				{
					// lower caves
					if (y < lava_height) return LAVA_BLOCK;
					return _AIR;
				}

				if (LIKELY(density < stone_lower))
				{
					// lower stone
					return STONE_BLOCK;
				}

				// remaining density = sand
				// pp will turn into oceanfloor with water pressure
				return BEACH_BLOCK;
			}
			else if (y <= WATERHEIGHT + beachhead + lower_to_upper)
			{
				// middle hemisphere, dense

				// transitional density for lower to upper
				float deltay = ((WATERHEIGHT + beachhead + lower_to_upper) - y) / lower_to_upper;

				// cave transition lower/upper
				if (caves < cave_upper * (1.0 - deltay) + cave_lower * deltay)
					return _AIR;

				// tone down soil deposits the higher up we get
				if (density < stone_upper * (1.0 - deltay) + stone_lower * deltay)
					return STONE_BLOCK;

				return SOIL_BLOCK;
			}

			// upper hemisphere, dense

			if (caves < cave_upper)
				return _AIR;

			if (density < stone_upper)
				return STONE_BLOCK;

			return SOIL_BLOCK;
		}
		else
		{
			// lower hemisphere, dense
			if (y < WATERHEIGHT)
				return WATER_BLOCK;

			// upper hemisphere, clear
			return _AIR;
		}
	} // Terrain::getBlock()

	#define ALIGN_AVX   __attribute__((aligned(32)))

	// the main generator!
	void Terrain::generate(gendata_t* data)
	{
		// interpolation grid dimensions
    static const int GRID2D = BLOCKS_XZ / 4;
    static const int FACTOR_2D = BLOCKS_XZ / GRID2D;
		static const int y_step = 4;
		static const int y_points = BLOCKS_Y / y_step + 3;

		// terrain heightmaps
    glm::vec3 heightmap_und[GRID2D+1][GRID2D+1] ALIGN_AVX;
    float     heightmap_gnd[GRID2D+1][GRID2D+1] ALIGN_AVX;
		float     beachhead    [GRID2D+1][GRID2D+1] ALIGN_AVX;
		// noise (terrain density) values
		float noisearray[GRID2D+1][GRID2D+1][y_points] ALIGN_AVX;
		// 3D caves densities
		float cave_array[GRID2D+1][GRID2D+1][y_points] ALIGN_AVX;

    // precalculate heightmap
    for (int x = 0; x <= GRID2D; x++)
		for (int z = 0; z <= GRID2D; z++)
		{
			const glm::vec2 p2 = data->getBaseCoords2D(x * FACTOR_2D, z * FACTOR_2D);
			const auto& weights = data->getWeights(x * FACTOR_2D, z * FACTOR_2D);

			// set underground heightmap
      glm::vec3& underground_ref = heightmap_und[x][z];
      float& ground_ref = heightmap_gnd[x][z];
      underground_ref = glm::vec3(0.0f);
      ground_ref = 0.0f;
			for (const auto& v : weights.terrains)
			{
        underground_ref += terrains[v.first].hmap_und(p2, weights.height) * v.second;
        ground_ref += terrains[v.first].height3d * v.second;
			}
      ground_ref = glm::clamp(underground_ref.x + ground_ref, 0.f, TOP_BLOCK_FLT);
      underground_ref.x = glm::clamp(underground_ref.x, 0.f, TOP_BLOCK_FLT);

      // beach height/level variance
			beachhead[x][z] = glm::simplex(p2 * 0.005f);
    }

		// retrieve data for noise biome interpolation
		for (int x = 0; x <= GRID2D; x++)
		for (int z = 0; z <= GRID2D; z++)
		{
      const int bx = x * FACTOR_2D;
      const int bz = z * FACTOR_2D;
			auto& weights = data->getWeights(bx, bz);

      const glm::vec3& HVALUE_UND = heightmap_und[x][z];
      const int MAX_UND = HVALUE_UND.x * BLOCKS_Y;

      const float HVALUE_GND = heightmap_gnd[x][z];
      // we need this to interpolate properly under water
      const int MAX_GND = std::max(int(HVALUE_GND * BLOCKS_Y), WATERLEVEL);
      // retrieve flatland for inside-area
      cppcraft::Flatland::caveland_t* cavedata;
      if (x < GRID2D && z < GRID2D) {
        cavedata = &data->flatl.cave(bx, bz);
        std::memset(cavedata->underworld.data(), 0, sizeof(cavedata->underworld));
      }

			// create unprocessed 3D volumes
			glm::vec3 p = data->getBaseCoords3D(x * FACTOR_2D, 0.0, z * FACTOR_2D);

      // the worst slope can add alot more than just MAX_GND to to have
      // smooth terrain we need to go quite a bit more up, still efficient
      const int WORST_SLOPE_Y = MAX_GND + y_step + 8;
      //const int WORST_SLOPE_Y = BLOCKS_Y + y_step - 1;

      // let's zero out everything above the max value to avoid interp. errors
      for (int y = WORST_SLOPE_Y; y < BLOCKS_Y; y += y_step)
      {
        cave_array[x][z][y / y_step] = 0.0f;
        noisearray[x][z][y / y_step] = 0.0f;
      }

      for (int y = 0; y <= WORST_SLOPE_Y; y += y_step)
			{
				p.y = y / float(BLOCKS_Y);

        float& cave_noise = cave_array[x][z][y / y_step];
        {
          auto res = Biome::first(Biome::underworldGen(p * UNDERGEN_SCALE), cave_terrains);
          // cave density function
          cave_noise = cave_terrains[res.first].func3d(p, HVALUE_UND) * res.second;

          // store terrain ID in flatland array
          if (x < GRID2D && z < GRID2D) {
            cavedata->underworld[y / y_step] = res.first;
          }
        }

        if (y >= MAX_UND - y_step)
        {
  				// terrain density functions
  				float& noise = noisearray[x][z][y / y_step];
  				noise = 0.0f;

  				for (auto& value : weights.terrains)
  				{
            auto& terrain = terrains[value.first];
  					noise += terrain.func3d(p, HVALUE_UND) * value.second;
				  } // weights
        } // ground level
			} // for(y)
		}

		// generating from top to bottom, not including y == 0
		for (int x = 0; x < BLOCKS_XZ; x++)
		{
			const float fx = x / float(BLOCKS_XZ) * GRID2D;
			const int bx = fx; // start x
			const float frx = fx - bx;

			for (int z = 0; z < BLOCKS_XZ; z++)
			{
				const float fz = z / float(BLOCKS_XZ) * GRID2D;
				const int bz = fz;  // integral
				const float frz = fz - bz;

				float w0, w1;
				// heightmap weights //
				w0 = mix( heightmap_gnd[bx][bz  ], heightmap_gnd[bx+1][bz  ], frx );
				w1 = mix( heightmap_gnd[bx][bz+1], heightmap_gnd[bx+1][bz+1], frx );
				int MAX_GND = mix( w0, w1, frz ) * BLOCKS_Y;
				MAX_GND = std::max(MAX_GND, WATERLEVEL);

        w0 = mix( heightmap_und[bx][bz  ].x, heightmap_und[bx+1][bz  ].x, frx );
				w1 = mix( heightmap_und[bx][bz+1].x, heightmap_und[bx+1][bz+1].x, frx );
				const int MAX_UND = mix( w0, w1, frz ) * BLOCKS_Y;
				// heightmap weights //

				// beachhead weights //
				w0 = mix( beachhead[bx][bz  ], beachhead[bx+1][bz  ], frx );
				w1 = mix( beachhead[bx][bz+1], beachhead[bx+1][bz+1], frx );
				const float beach = mix( w0, w1, frz );
				// beachhead weights //

				Block* block = &data->getb(x, 0, z);

				for (int y = 0; y < MAX_GND; y++)
				{
					int   iy  = y / y_step;
					float fry = (y % y_step) / (float) y_step;
          float density;

          if (y >= MAX_UND)
          {
  					// density weights //
  					// mix all Y-variants
float noise00 = mix( noisearray[bx  ][bz  ][iy], noisearray[bx  ][bz  ][iy+1], fry );
float noise10 = mix( noisearray[bx+1][bz  ][iy], noisearray[bx+1][bz  ][iy+1], fry );
float noise01 = mix( noisearray[bx  ][bz+1][iy], noisearray[bx  ][bz+1][iy+1], fry );
float noise11 = mix( noisearray[bx+1][bz+1][iy], noisearray[bx+1][bz+1][iy+1], fry );
  					// mix all X-variants
  					w0 = mix( noise00, noise10, frx );
  					w1 = mix( noise01, noise11, frx );
  					// final density from Z-variant
  					density = mix( w0, w1, frz );
  					// density weights //
          }
          else {
            density = -1.0f;
          }

					if (y <= WATERLEVEL || density < 0.0f)
					{
						float caves = 0.0f;
						if (density < 0.0f)
						{
float noise00 = mix( cave_array[bx  ][bz  ][iy], cave_array[bx  ][bz  ][iy+1], fry );
float noise10 = mix( cave_array[bx+1][bz  ][iy], cave_array[bx+1][bz  ][iy+1], fry );
float noise01 = mix( cave_array[bx  ][bz+1][iy], cave_array[bx  ][bz+1][iy+1], fry );
float noise11 = mix( cave_array[bx+1][bz+1][iy], cave_array[bx+1][bz+1][iy+1], fry );
							// caves density (high precision) //
							w0 = mix( noise00, noise10, frx );
							w1 = mix( noise01, noise11, frx );
							caves = mix( w0, w1, frz );
							// caves density //
						}
            // calculate and set basic type
						block[y] = getBlock(y / float(BLOCKS_Y), beach, density, caves);
					}
					else
					{
						new (&block[y]) Block(_AIR);
					}

				} // y

				// fill the rest with skylight air
				for (int y = MAX_GND; y < BLOCKS_Y; y++)
				{
					new (&block[y]) Block(_AIR, 0, 0, 15);
				}

        // setting early skylevel to optimize post-processing
				data->flatl(x, z).skyLevel = MAX_GND;

			} // z

		} // x

	} // generateTerrain()

}
