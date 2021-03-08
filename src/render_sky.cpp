#include "render_sky.hpp"

#include <library/opengl/opengl.hpp>
#include <library/opengl/vao.hpp>
#include "atmosphere.hpp"
#include "camera.hpp"
#include "gameconf.hpp"
#include "renderconst.hpp"
#include "renderman.hpp"
#include "shaderman.hpp"
#include "sun.hpp"
#include "textureman.hpp"
#include <library/math/matrix.hpp>
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtx/transform.hpp>
#include <cmath>

using namespace library;

namespace cppcraft
{
	SkyRenderer skyrenderer;
	VAO satteliteVAO;
	const double PI = 4.0 * atan(1);

	void SkyRenderer::init()
	{
		/// initialize atmosphere ///
		Atmosphere::init();
		/// initialize clouds ///
		createClouds();

		/// create VAO for all satellites ///
		struct satellite_t
		{
			float vx, vy, vz;

			satellite_t() {}
			satellite_t(float X, float Y, float Z) :
				vx(X), vy(Y), vz(Z) {}
		};
		satellite_t mv[4];

		mv[0] = satellite_t ( -0.5, -0.5, 0.0 );
		mv[1] = satellite_t (  0.5, -0.5, 0.0 );
		mv[2] = satellite_t (  0.5,  0.5, 0.0 );
		mv[3] = satellite_t ( -0.5,  0.5, 0.0 );

		satteliteVAO.begin(sizeof(satellite_t), 4, mv);
		satteliteVAO.attrib(0, 3, GL_FLOAT, GL_FALSE, 0);
		satteliteVAO.end();
	}

	// render background / atmosphere, sun and moon
	void SkyRenderer::render(cppcraft::Camera& camera, float cloudLevel, double time, int mode)
	{
		// render sky dome
		Atmosphere::render(camera, mode);

		glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glColorMask(1, 1, 1, 0);

		// render regular sun
		if (mode == 2)
		{
			renderSun();
		}

		// render moon texture
		renderMoon(camera);

		// avoid rendering clouds for reflections
		if (mode != 1 && gameconf.clouds == true)
		{
			// render clouds
			skyrenderer.renderClouds(cloudLevel, camera, time);
		}

		glDisable(GL_BLEND);
		glColorMask(1, 1, 1, 1);
	}

	void SkyRenderer::renderSun()
	{
		//if ogl.lastsun(1) < -0.15 then return

		Shader& sunshader = shaderman[Shaderman::SUN];
		sunshader.bind();

		// view matrix
		sunshader.sendMatrix("matview", thesun.getSunMatrix());
		// rotation matrix
		sunshader.sendMatrix("matrot", camera.getRotationMatrix());

		textureman.bind(0, Textureman::T_SUN);

		// rendering call
		satteliteVAO.render(GL_QUADS);
	}

	glm::mat4 SkyRenderer::renderSunProj()
	{
		Shader& sunshader = shaderman[Shaderman::SUNPROJ];
		sunshader.bind();

		glm::mat4 matsun = thesun.getSunMatrix();

		// view matrix
		sunshader.sendMatrix("matview", matsun);
		// rotation matrix
		sunshader.sendMatrix("matrot", camera.getRotationMatrix());

		// depth used as stencil buffer
		textureman[Textureman::T_RENDERBUFFER].bind(1);
		textureman[Textureman::T_SUN].bind(0);

		// rendering call
		satteliteVAO.render(GL_QUADS);

		return matsun;
	}

	void SkyRenderer::renderMoon(Camera& camera)
	{
		//if (thesun.getRealtimeAngle().y > 0.2) return;

		Shader& moon = shaderman[Shaderman::MOON];
		moon.bind();

		glm::mat4 matmoon = rotationMatrix(0.0, -PI / 2, 0.0);
		matmoon *= rotationMatrix(thesun.getRealtimeRadianAngle() + PI, 0.0, 0.0);
		matmoon *= glm::translate(glm::vec3(0.0, 0.0, -2.0));

		matmoon = camera.getRotationMatrix() * matmoon;

		// view matrix
		moon.sendMatrix("matview", matmoon);
		// rotation matrix
		moon.sendMatrix("matrot", camera.getRotationMatrix());
		// daylight level
		moon.sendFloat("daylight", thesun.getRealtimeDaylight());

		textureman.bind(0, Textureman::T_MOON);

		// rendering call
		satteliteVAO.render(GL_QUADS);
	}

}
