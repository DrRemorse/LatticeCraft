#include "render_fs.hpp"

#include <library/log.hpp>
#include <library/opengl/opengl.hpp>
#include <library/opengl/vao.hpp>
#include <library/opengl/window.hpp>
#include "camera.hpp"
#include "gameconf.hpp"
#include "player.hpp"
#include "render_sky.hpp"
#include "sun.hpp"
#include "shaderman.hpp"
#include "textureman.hpp"
#include <glm/vec2.hpp>
#include <library/math/matrix.hpp>

using namespace library;
using namespace glm;

namespace cppcraft
{
	extern VAO screenVAO;
	glm::mat4 lensMatrix;

	void FSRenderer::initFlare(Renderer& renderer)
	{
		lensMatrix = biasMatrix() * camera.getProjection();
		glGenFramebuffers(1, &flareFBO);

    // lens flare
    renderer.on_resize(
    [this] (Renderer& renderer) {
      // set texture sizes
      int factor = gameconf.highq_lens ? 2 : 4;
  		this->flareTxW = renderer.width() / factor;
  		this->flareTxH = renderer.height() / factor;
    });
	}

	glm::vec2 FSRenderer::getSunVector(const glm::mat4& matsun)
	{
		// create sun coordinate 2-vector
		vec4 sunproj = lensMatrix * matsun * vec4(0.0, 0.0, 0.0, 1.0);
    if (sunproj.w > 0.0f) {
		  sunproj.x /= sunproj.w;
		  sunproj.y /= sunproj.w;
    }
		return vec2(sunproj);
	}

	void FSRenderer::renderLensflare(Renderer& renderer)
	{
		textureman.unbind(0);

		//  render sun to draw buffer 0
		glBindFramebuffer(GL_FRAMEBUFFER, flareFBO);
		glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, textureman.get(Textureman::T_LENSFLARE), 0);

		//  need to clear buffer before we exit, so that the lensflare buffer is valid!
		glViewport(0, 0, flareTxW, flareTxH);
		glClear(GL_COLOR_BUFFER_BIT);

		// render sun-to-be-flare to buffer 0
		glm::mat4 matsun = skyrenderer.renderSunProj();
		glm::vec2 sunproj = getSunVector(matsun);

		// check that the sun is in fact in the camera
		if (dot(thesun.getRealtimeAngle(), player.getLookVector()) < 0.5)
		{
			// NOTE: We need to exit here, so that the flare buffer is cleared properly
			// go back to main framebuffer & restore viewport
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			glViewport(0, 0, renderer.width(), renderer.height());
			return;
		}

		//  LOW blur sun sphere horizontally (low)
		glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, textureman.get(Textureman::T_LENSFLARE3), 0);

		textureman.bind(0, Textureman::T_LENSFLARE);

		Shader& blur = shaderman[Shaderman::BLUR];
		blur.bind();
		blur.sendInteger("Width", 2);
		blur.sendVec2("dir", vec2(1.0 / flareTxW, 0.0));

		//  render
		screenVAO.render(GL_QUADS);

		//  LOW blur sun sphere vertically (low) (final result = texture 1)
		glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, textureman.get(Textureman::T_LENSFLARE2), 0);
		textureman.bind(0, Textureman::T_LENSFLARE3);

		blur.sendVec2("dir", vec2(0.0, 1.0 / flareTxH));

		//  render
		screenVAO.render(GL_QUADS);

		//  HIGH blur sun sphere horizontally (high)
		glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, textureman.get(Textureman::T_LENSFLARE), 0);
		textureman.bind(0, Textureman::T_LENSFLARE2);

		blur.sendVec2("dir", vec2(1.0 / flareTxW, 0.0));

		//  render
		screenVAO.render(GL_QUADS);

		//  blur sun sphere vertically (high) (final result = 0)
		glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, textureman.get(Textureman::T_LENSFLARE3), 0);
		textureman.bind(0, Textureman::T_LENSFLARE);

		blur.sendVec2("dir", vec2(0.0, 1.0 / flareTxH));

		//  render
		screenVAO.render(GL_QUADS);

		//  blur sun sphere radially, calculate lens flare and halo
		glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, textureman.get(Textureman::T_LENSFLARE), 0);
		textureman.bind(2, Textureman::T_LENSDIRT);
		textureman.bind(1, Textureman::T_LENSFLARE3);
		textureman.bind(0, Textureman::T_LENSFLARE2);

		Shader& lens = shaderman[Shaderman::LENSFLARE];
		lens.bind();
		lens.sendVec2("sunproj", sunproj);

		//  render final lens image
		screenVAO.render(GL_QUADS);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// restore viewport
		glViewport(0, 0, renderer.width(), renderer.height());

#ifdef OPENGL_DO_CHECKS
		if (OpenGL::checkError()) {
			logger << Log::ERR << "FSRenderer::renderLensFlare(): Error after rendering lens flare" << Log::ENDL;
			throw std::runtime_error("Error after rendering lens flare");
		}
#endif
	}
}
