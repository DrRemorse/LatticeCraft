#include "render_gui.hpp"

#include <library/log.hpp>
#include <library/opengl/opengl.hpp>
#include <library/opengl/vao.hpp>
#include "camera.hpp"
#include "minimap.hpp"
#include "player.hpp"
#include "shaderman.hpp"
#include "spiders.hpp"
#include "textureman.hpp"
#include "world.hpp"
#include <glm/gtx/transform.hpp>
#include <library/math/matrix.hpp>
#include <cmath>

using namespace library;

namespace cppcraft
{
	glm::mat4 minimapMVP;
	glm::mat4 compassMVP;
	library::VAO compassVAO;

	void GUIRenderer::renderMinimap(glm::mat4& ortho)
	{
		float compx = this->width * 0.86;
		float compy = this->height * 0.24;
		const float minimapScale = 0.16;
		const float compassScale = 0.26;

		/// create location MVP matrix for minimap & compass ///

		if (camera.rotated)
		{
			// move to position
			glm::mat4 matview(ortho);
			matview *= glm::translate(glm::vec3(compx, compy, 0.0f));

			// rotate properly
			matview *= rotationMatrix(0.0, 0.0, -player.rot.y);

			// orthograhic projection * view
			compassMVP = matview * glm::scale(glm::vec3(compassScale, compassScale, 1.0));
			// orthograhic projection * view
			minimapMVP = matview * glm::scale(glm::vec3(minimapScale, minimapScale, 1.0));
		}

		/// minimap ///

		// render minimap
		minimap.render(minimapMVP);

		/// compass ///

		Shader& shd = shaderman[Shaderman::COMPASS];
		shd.bind();

		if (camera.rotated)
		{
			shd.sendMatrix("mvp", compassMVP);
		}
		if (true) //camera.ref)
		{
			// absolute spawn position
			glm::vec3 spawn = Spiders::distanceToWorldXZ(World::WORLD_STARTING_X, World::WORLD_STARTING_Z);
			shd.sendVec3("spawn", spawn);
		}

		// compass texture
		textureman.bind(0, Textureman::T_COMPASS);

		if (compassVAO.good() == false)
		{
			gui_vertex_t compassData[4] =
			{
				{ -0.5, -0.5, 0,   1, 0,   0 },
				{  0.5, -0.5, 0,   0, 0,   0 },
				{  0.5,  0.5, 0,   0, 1,   0 },
				{ -0.5,  0.5, 0,   1, 1,   0 }
			};

			compassVAO.begin(sizeof(gui_vertex_t), 4, &compassData);
			compassVAO.attrib(0, 3, GL_FLOAT, false, 0);
			compassVAO.attrib(1, 2, GL_FLOAT, false, offsetof(gui_vertex_t, u));
			compassVAO.attrib(2, 4, GL_UNSIGNED_BYTE, true, offsetof(gui_vertex_t, color));
			compassVAO.end();
		}

		// render compass
		compassVAO.render(GL_QUADS);
	}

}
