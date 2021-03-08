#include "particles.hpp"

#include <library/log.hpp>
#include <library/opengl/opengl.hpp>
#include <library/opengl/vao.hpp>
#include "camera.hpp"
#include "shaderman.hpp"
#include "textureman.hpp"
#include "tiles.hpp"
#include "sector.hpp"
#include "sun.hpp"
#include "world.hpp"
#include <glm/gtx/transform.hpp>

using namespace library;

namespace cppcraft
{
  VAO vao;

	void Particles::renderUpdate()
	{
		this->mtx.lock();
    if (physics.updated == true)
    {
      snapshot = std::move(physics);
      physics.updated = false;
    }
    this->mtx.unlock();

    // exit when nothing to do
		if (snapshot.vertices.empty()) return;

		if (snapshot.updated)
		{
			snapshot.updated = false;
      auto* data = snapshot.vertices.data();
      int   len  = snapshot.vertices.size();

			if (vao.good())
			{
        vao.upload(sizeof(particle_vertex_t), len, data);
			}
      else
      {
        vao.begin(sizeof(particle_vertex_t), len, data, GL_STREAM_DRAW);
    		vao.attrib(0, 3, GL_FLOAT, GL_FALSE, 0);
        vao.attrib(1, 4, GL_UNSIGNED_BYTE, GL_FALSE, 12);
        vao.attrib(2, 4, GL_UNSIGNED_BYTE, GL_TRUE,  16);
        vao.attrib(3, 4, GL_UNSIGNED_BYTE, GL_TRUE,  20);
    		vao.end();
      }
		}
	}

	void Particles::render(int snapWX, int snapWZ, double timeElapsed)
	{
		// start particles shader
		// BEFORE we potentially exit because there are no particles to render
		// this is done to get the first-frame update.. considering there's
		// always particles to render, we'll just go ahead and always bind the shader

		Shader& shd = shaderman[Shaderman::PARTICLE];
		shd.bind();
		shd.sendFloat("timeElapsed", timeElapsed);

		if (camera.ref)
		{
			float tx = (snapshot.currentWX - snapWX) * Sector::BLOCKS_XZ;
			float tz = (snapshot.currentWZ - snapWZ) * Sector::BLOCKS_XZ;

			glm::mat4 matview = camera.getViewMatrix();
			matview *= glm::translate(glm::vec3(tx, 0.0f, tz));

			// send view matrix
			shd.sendMatrix("matview", matview);
		}

		if (snapshot.vertices.empty()) return;

		glEnable(GL_POINT_SPRITE);

		// bind 2d array of textures/tiles
    tiledb.particles.diff_texture().bind(0);

		// render particles
    vao.render(GL_POINTS, 0, snapshot.vertices.size());

		glDisable(GL_POINT_SPRITE);
	}

}
