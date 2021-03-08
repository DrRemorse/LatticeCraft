#include "terrains.hpp"

#include <library/noise/cosnoise.hpp>
#include <library/noise/simplex1234.h> // snoise1
#include <library/math/toolbox.hpp>
#include "../biomegen/biome.hpp"
#include "../blocks.hpp"
#include "../terragen.hpp"
#include "../random.hpp"
#include "noise.hpp"
#include "helpers.hpp"
#undef NDEBUG
#include <cassert>

#include <glm/gtc/noise.hpp>
#include <Simplex.h>

using namespace glm;
using namespace library;

namespace terragen
{
	Terrains terrains;
  Terrains cave_terrains;
	static float getnoise_caves(vec3 p, vec3);
  static float getnoise_test(vec3 p, vec3);
  static float getnoise_basin(vec3 p, vec3);

  static int process_caves(gendata_t* gdata, int x, int z, const int MAX_Y, const int MIN_Y)
  {
    const int wx = gdata->wx * BLOCKS_XZ + x;
		const int wz = gdata->wz * BLOCKS_XZ + z;
    auto& cave = gdata->flatl.cave(x, z);

    int air = 0;
    int lastID = STONE_BLOCK;
		for (int y = MAX_Y; y >= MIN_Y; y--)
		{
			Block& block = gdata->getb(x, y, z);

      if (block.getID() == STONE_BLOCK && air > 0)
      {
        int uid = cave.underworld[y / 4];
        if (uid == cave_terrains["basin"])
        {
          // Basin
          if (y > 8)
          {
            block.setID(db::getb("basin_murksoil"));
            if (air > 18)
            {
              float rand = randf(wx, y, wz);
              if (rand < 0.0025) {
                gdata->add_object("mushroom_wild", wx, y+1, wz, 12);
              }
            }
          }
        } // basin

        float rand = randf(wx, y, wz);
        if (rand < 0.02 && air > 1)
        {
          // create some cavey fauna
          gdata->getb(x, y+1, z).setID(
            (rand < 0.01) ? db::getb("mushroom_blue") : db::getb("mushroom_flat"));
        }
      }
      if (block.isAir()) air++; else air = 0;
      lastID = block.getID();
    }
    return 0;
  } // process_caves()

	void Terrains::init()
	{
    auto& test = cave_terrains.add("test", "(empty)", Biome::biome_t{0.5f, 0.5f, 0.0f},
                 nullptr, 0.0f, getnoise_test, nullptr);
		test.setFog(glm::vec4(0.0f, 0.0f, 0.0f, 0.8f), 96);

		auto& cave = cave_terrains.add("caves", "Caves", Biome::biome_t{0.25f, 0.0f, 0.25f},
                 nullptr, 0.0f, getnoise_caves, process_caves);
		cave.setFog(glm::vec4(0.0f, 0.0f, 0.0f, 0.8f), 96);

    auto& basin = cave_terrains.add("basin", "Basin", Biome::biome_t{0.75f, 0.0f, 0.5f},
                 nullptr, 0.0f, getnoise_basin, process_caves);
		basin.setFog(glm::vec4(0.0f, 0.0f, 0.0f, 0.8f), 96);

		extern void terrain_icecap_init();
		terrain_icecap_init();

    extern void terrain_taiga_init();
		terrain_taiga_init();

		extern void terrain_grass_init();
		terrain_grass_init();

    extern void terrain_jungle_init();
    terrain_jungle_init();

    extern void terrain_desert_init();
    terrain_desert_init();
	}

	///////////////////////////////////////////////////
	///////////////////////////////////////////////////
	#define sfreq(v, n) glm::simplex(v * float(n))
	#define sfreq2d(v, n) glm::simplex(glm::vec2(v.x, v.z) * float(n))

	inline float ramp(float x, float p)
	{
		if (x >= 0.0f && x <= 1.0f)
			return std::pow(x, p);
		if (x < 0.0) return 0.0;
		return 1.0;
	}

  float getnoise_test(vec3 p, glm::vec3 under)
  {
    return 0.1f;
  }
  float getnoise_basin(vec3 p, glm::vec3 under)
  {
    vec3 npos = p * vec3(0.005f, 5.0f, 0.005f);
    float updown = 0.1f * Simplex::worleyfBm(npos);
    return updown + Simplex::fBm(npos) * ((under.x - p.y) / under.x);
  }

	float getnoise_caves(vec3 p, glm::vec3)
	{
		vec3 npos = p * vec3(0.01, 2.5, 0.01);

		float n1 = glm::simplex(npos);

		const float CAVE_TRESHOLD = 0.25f;
		const float EDGE = CAVE_TRESHOLD * 0.2f;

		if (n1 > -CAVE_TRESHOLD && n1 < CAVE_TRESHOLD)
		{
			npos = p * vec3(0.01, 6.0, 0.01);

			// cross noise
			float n2 = glm::simplex(npos);
			float n3 = glm::simplex(npos + vec3(0.0, 3.5, 0.0));
			float n4 = glm::simplex(npos + vec3(0.2, 7.0, 0.2));

			// caves increase in density as we go lower
			float DEPTH_DENSITY = 0.08 + (1.0 - p.y * p.y) * 0.2;
			float cavenoise = std::abs(n2 + n3 + n4);

			if (cavenoise < DEPTH_DENSITY)
			{
				// find edge
				if (std::abs(n1) > CAVE_TRESHOLD - EDGE)
				{
					n1 -= CAVE_TRESHOLD - EDGE;
					n1 /= EDGE;
				}
				else n1 = 1.0f;

				float t = 1.0 - cavenoise / DEPTH_DENSITY;
				return -t * 0.1;
			}
		}
		return 0.1;
	}

	float getnoise_snow(vec3 p, float hvalue)
	{
		p.x *= 0.006;
		p.z *= 0.006;
		float n1 = sfreq(p, 3.0);

		// peaks
		float n3 = sfreq2d(p, 2.0);
		float n4 = sfreq2d(p, 1.7);

		float c1 = sfreq2d(p, 0.5);
		float c2 = sfreq2d(p, 0.25);

		vec3 npos = p / 4.0f; // the relationship between n1 and npos is 4 / 3

		const float COSN_CURVE = 0.5; // sharper waves at higher value, crested waves at values < 1.0
		const float COSN_FAT   = 0.0;
		float COSN_CUTS  = 0.5 - p.y * 0.5;

		#define COSN cosnoise(npos, n1, 0.5, p.y * 2.0, COSN_CURVE, COSN_FAT, COSN_CUTS)

		n1 = (p.y - 0.25) * p.y - n3 * n3 * 0.25 + n4 * 0.1 + COSN * 0.15;

		// reduce height by contintental noise
		n1 += c1 * 0.2 + c2 * 0.2;

		// create me some bridges
		const float bridge_height = 0.3;
		float dist = 1.0 - std::sqrt(p.y * p.y + bridge_height*bridge_height);
		n1 -= dist * dist * (1.0 - c2 + c1) * 0.75;
		return n1;
	}

	float getnoise_autumn(vec3 p, float hvalue)
	{
		p.x *= 0.004; p.z *= 0.004;

		// land level
		float n1 = p.y - 0.2 - ridgedmultifractal(vec2(p.x, p.z), 4, 2.5, p.y, 0.0, 1.0) * 0.3;
		return n1;
	}

	float getnoise_islands(vec3 p, float hvalue)
	{
		p.x *= 0.008;
		p.z *= 0.008;

		float n0 = sfreq2d(p, 0.25); // continental

		float landy = (0.5 + 0.5 * n0) * 2.0;
		float n1 = glm::simplex(p + vec3(0.0, landy, 0.0));   // island ring
		float n2 = sfreq(p, 8.0);   // carve
		float landscape = sfreq2d(p, 0.5);

		const float noise_rel1 = 4.0;
		const float noise_rel2 = 16.0;

		vec3 npos = p * noise_rel1;
		npos.y *= landy;

		const float COSN_CURVE = 4.0;
		const float COSN_FAT   = 0.0;
		const float COSN_CUTS  = 0.5; //p.y * 0.2 + 0.4;

		//const float cliffscale = 0.005; // higher = narrower
		float landx = 0.05; // + fabs(landscape - n0) * cliffscale;
		float landz = 0.05; // + fabs(n0 - landscape) * cliffscale;

		#define COSN_isl1 cosnoise(npos,  n1, landx, landz, COSN_CURVE, COSN_FAT, COSN_CUTS)
		#define COSN_isl2 cosnoise(npos2, n2,  0.1,  2.0, COSN_CURVE, COSN_FAT, COSN_CUTS)

		// lower height + compr noise    + continental
		n1 = p.y * p.y + COSN_isl1 * 0.25 + std::abs(n0) * 0.15 + 0.05;

		// create me some pillars
		pillars(p, landscape, n1);

		// ultra-scale down density above clouds, and severely low down-under
		const float scaledown = 0.90;
		const float scaleup   = 0.25;

		if (p.y > scaledown) {
			float dy = (p.y - scaledown) / (1.0 - scaledown);
			n1 += dy * dy * 1.0;
		} else
		if (p.y < scaleup) {
			float dy = (scaleup - p.y) / scaleup;
			n1 -= dy * dy * 0.5;
		}

		const float C_DEPTH    = 0.2;
		const float C_SHARP    = 3.0;
		const float C_STRENGTH = 0.04;

		// add cracks after scaling bottom / top
		if (n1 < 0.0 && n1 > -C_DEPTH)
		{
			vec3 npos2 = p * noise_rel2;
			npos2.y *= 0.5;

			float cracks = std::abs(landscape) * C_STRENGTH;

			n1 += ramp(1.0 - n1 / -C_DEPTH, C_SHARP) * (0.5f + COSN_isl2 * 0.5f) * cracks;
		}
		return n1;
	}

	float getnoise_marsh(vec3 p, float hvalue)
	{
		p.x *= 0.002;
		p.z *= 0.002;
		float n1 = sfreq(p, 0.3) * 0.025 + sfreq(p, 2.5) * 0.0125;
		float n2 = sfreq(p, 0.4) * 0.025 + sfreq(p, 2.6) * 0.0125;

		// river //
		const float river_delta = 0.1;

		float river = std::abs(sfreq2d(p, 2.22));
		river = (river < river_delta) ? 1.0 - river / river_delta : 0.0;
		if (n1 + n2 > 0.0 && p.y <= 0.25) river -= (n1 + n2) * 105.0;
		if (river < 0.0) river = 0.0;

		// river //

		// 3d hills
		float hills = 0.25 + sfreq(p, 0.9) * 0.1 + sfreq(p, 1.9) * 0.04;
		float slope = hills - 0.1;

		// hill placement
		float n3 = sfreq(p, 1.4);
		// hill height
		float height = 0.05 + n1 * 8.0;

		// noise value
		n1 = p.y - 0.25 + n1 + n2;

		if (n3 > hills)
		{
			n3 = (n3 - hills) / 0.25;
			n3 = std::pow(n3, 0.1);
			n1 -= height * n3;
			// remove river
			river = 0.0;
		}
		else if (n3 > slope)
		{
			// slope
			n3 = 1.0 - (hills - n3) / (hills - slope);
			n1 -= (height - 0.03) * std::pow(n3, 0.85);
			// fade river
			river *= 1.0 - n3 * n3 * n3;
		}

		// final value + river
		return n1 + river * 0.04; //fabsf(n2) * 8.0;
	}

}
