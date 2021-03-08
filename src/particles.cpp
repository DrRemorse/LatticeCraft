#include "particles.hpp"

#include <library/log.hpp>
#include <library/math/toolbox.hpp>
#include <library/opengl/opengl.hpp>
#include "player.hpp"
#include "player_logic.hpp"
#include "renderconst.hpp"
#include "sectors.hpp"
#include "world.hpp"
#include <glm/geometric.hpp>

using namespace library;

namespace cppcraft
{
	Particles particleSystem;

	void Particles::init()
	{
		logger << Log::INFO << "* Initializing particles" << Log::ENDL;

		srand(time(0));

		for (size_t i = 0; i < particles.size(); i++)
			deadParticles.push_back(i);

    // initialize world position properly
    physics.currentWX = world.getWX();
    physics.currentWZ = world.getWZ();
    snapshot = physics;
	}

	// execute one update-tick
	void Particles::update(double timeElapsed)
	{
		int lastAlive = -1;
    // particle vertices
    std::vector<particle_vertex_t> vertices;

		// player look vector & position
		glm::vec3 look = player.getLookVector();
		glm::vec3 playerPos = player.pos;

		for (int i = 0; i < this->count; i++)
		{
			Particle& p = particles[i];

			// skip to next particle, if this one was dead
			if (p.alive == false) continue;
			// decrease time-to-live
			if (p.ttl > 0)
			{
				p.ttl -= 1;
				// if the particle is dead, kill it
				if (p.ttl <= 0)
				{
					p.alive = false;
					deadParticles.push_back(i);
					continue;
				}
			}

			// find delta between worldpos and particle worldpos
			int dx = (p.wx - world.getWX()) * BLOCKS_XZ;
			int dz = (p.wz - world.getWZ()) * BLOCKS_XZ;

			// accelerate speed
			p.spd += p.acc;
			// move particle
			p.position += p.spd;

			// particle rendering position
			glm::vec3 fpos = p.position + glm::vec3(dx, 0.0f, dz);

			// direction to particle
			glm::vec3 direction = glm::normalize(fpos - playerPos);

			// check that the particle is somewhat in camera
			if (dot(direction, look) > 0.5f)
			{
        particle_vertex_t pv;

				// rendering position
				pv.x = fpos.x;
				pv.y = fpos.y;
				pv.z = fpos.z;

				ParticleType& type = types[p.id];
				type.on_tick(p, pv);

        vertices.push_back(pv);
			}
			// helper for particle count
			lastAlive = i;
		}
		this->count = lastAlive + 1;

		this->mtx.lock();
		{
      physics.updated = true;
			physics.currentWX = world.getWX();
			physics.currentWZ = world.getWZ();
      physics.vertices = std::move(vertices);
		}
		this->mtx.unlock();
	}

	int Particles::newParticleID()
	{
		if (deadParticles.empty()) return -1;

		int i = deadParticles.front();
		deadParticles.pop_front();
		return i;
	}

	int Particles::newParticle(glm::vec3 position, short id)
	{
		int index = newParticleID();
		if (index == -1) return index;
		// update particle count
		if (this->count <= index) this->count = index + 1;

		Particle& p = particles.at(index);
		// set common values
		p.alive = true;
		p.id = id;
		p.position = position;
		p.wx = world.getWX();
		p.wz = world.getWZ();

		types[id].on_create(p, position);
		return index;
	}
}
