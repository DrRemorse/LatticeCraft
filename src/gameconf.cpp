#include "gameconf.hpp"

#include <library/log.hpp>
#include <library/config.hpp>
#include <common.hpp>
#include "tiles.hpp"

using namespace library;
library::Config config;

namespace cppcraft
{
	GameConfig gameconf;

	void GameConfig::init()
	{
		// rendering quality
		supersampling = config.get("render.supersampling", 1);
		multisampling = config.get("render.multisampling", 0);
		anisotropy    = config.get("render.anisotropy", 1.0f);

		// minimum 1x supersampling
		if (supersampling < 1) supersampling = 1;
		// "disable" 1x multisampling
		if (multisampling < 2) multisampling = 0; else supersampling = 1;
		// minimum 1x anisotropic filter size
		if (anisotropy < 1.0f) anisotropy = 1.0f;

		// renderer settings
		highq_water   = config.get("render.highq_water", true);
		highq_sky     = config.get("render.highq_sky",   false);
		clouds        = config.get("render.clouds",      false);

		lensflare     = config.get("render.lensflare",  true);
		highq_lens    = config.get("render.highq_lens", false);
		distance_blur = config.get("render.distance_blur", true);

		reflections    = config.get("render.reflections", true);
		hq_reflections = config.get("render.hq_reflections", false);
		reflectTerrain = config.get("render.reflect_terrain", false);

		playerhand    = config.get("playerhand", true);

		// audio settings
		sounds   = config.get("sound.sounds", true);
		music    = config.get("sound.music", true);
		ambience = config.get("sound.ambient", true);

    // initialize tile database
		tiledb.init();
	}
}
