#ifndef GAMECONF_HPP
#define GAMECONF_HPP

#include <library/config.hpp>

namespace cppcraft
{
	class GameConfig
	{
	public:
		void init();
		
		bool sounds;
		bool music;
		bool ambience;
		float master_volume;
		
		int supersampling;
		int multisampling;
		float anisotropy;
		
		bool highq_water;
		bool highq_sky;
		bool clouds;
		
		bool lensflare;
		bool highq_lens;
		
		bool distance_blur;
		
		bool reflections;
		bool hq_reflections;
		bool reflectTerrain;
		
		bool playerhand;
		
	};
	extern GameConfig gameconf;
}
extern library::Config config;

#endif
