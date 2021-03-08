#ifndef PARTICLES_HPP
#define PARTICLES_HPP

#include "delegate.hpp"
#include <glm/vec3.hpp>
#include <cstdint>
#include <array>
#include <deque>
#include <map>
#include <mutex>
#include <string>
#include <vector>

using namespace glm;

namespace cppcraft
{
	struct particle_vertex_t
	{
		float x, y, z;
		uint8_t size;    // size of particle (0-255)
		uint8_t tileID;  // tile ID based on texture
		uint8_t uvscale; // 0-100% texture scale
		uint8_t shiny;   // 1 = apply shiny magical effect

		uint8_t alpha;  // translucency value
		uint8_t bright; // brightness factor (emulating darkness and as a daylight multiplier)
		uint8_t offsetX; // texture offset X
		uint8_t offsetY; // texture offset Y

		uint32_t color;

	} __attribute__((packed));

	struct Particle
	{
		bool alive;
		uint16_t id;

		int wx, wz;
		int ttl;
		glm::vec3 position;
		glm::vec3 acc;
		glm::vec3 spd;
	};

	// Particle ID points to this class
	struct ParticleType
	{
		// creation function, called by any of the terrain-tick functions
		// the vec3 is the players position, with the y value set to groundlevel
		typedef delegate<void(Particle&, glm::vec3)> create_func_t;
		// this function is called every tick until ttl reaches zero
		typedef delegate<void(Particle&, particle_vertex_t&)> tick_func_t;

		ParticleType(create_func_t cfunc, tick_func_t tfunc)
			: on_create(cfunc), on_tick(tfunc) {}

		create_func_t on_create;
		tick_func_t   on_tick;
	};

	class Particles
	{
	public:
		static const int MAX_PARTICLES = 1024;

		// initialize system
		void init();
		// one round of updates, as an integrator
		void update(double timeElapsed);
		// rendering
		void renderUpdate();
		void render(int snapWX, int snapWZ, double time);

		// add a new named particle type
		template <typename... Args>
		int add(const std::string& name, Args... args)
		{
			types.emplace_back(args...);
			int index = types.size()-1;
			names[name] = index;
			return index;
		}
		ParticleType& operator[] (size_t i)
		{
			return types[i];
		}
		int operator() (const std::string& name) const
		{
			return names.at(name);
		}

		// returns -1 if there isnt enough room to create more particles
		int newParticle(glm::vec3 position, short id);

	private:
    // returns a new particle ID from queue, or -1
		int newParticleID();

    std::mutex mtx;
    // database
		std::map<std::string, size_t> names;
		std::vector<ParticleType>     types;

    struct info_t {
  		bool updated = true;
		  int currentWX, currentWZ;
      std::vector<particle_vertex_t> vertices;
    };
    info_t physics;
    info_t snapshot;

    // simulation
    std::array<Particle, MAX_PARTICLES> particles;
    int count = 0;
		std::deque<int> deadParticles;
	};
	extern Particles particleSystem;
}

#endif
