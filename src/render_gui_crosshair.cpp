#include "render_gui.hpp"

#include <library/opengl/opengl.hpp>
#include <library/opengl/vao.hpp>
#include <glm/mat4x4.hpp>
#include "shaderman.hpp"
#include "textureman.hpp"
#include <cmath>

using namespace library;

namespace cppcraft
{
	class GUICrosshair
	{
	public:
		VAO vao;

	};
	GUICrosshair crosshair;

	void GUIRenderer::renderCrosshair(glm::mat4& ortho)
	{
		Shader& shd = shaderman[Shaderman::GUI];
		shd.bind();

		shd.sendMatrix("mvp", ortho);

		textureman.bind(0, Textureman::T_CROSSHAIR);

		if (crosshair.vao.good() == false)
		{
			float x = this->width  * 0.5;
			float y = this->height * 0.5;

			float chs = 0.02;

			gui_vertex_t crosshairData[4] =
			{
			//    x      y    z    u  v  color
				{ x-chs, y-chs, 0,   0, 0,   0 },
				{ x+chs, y-chs, 0,   1, 0,   0 },
				{ x+chs, y+chs, 0,   1, 1,   0 },
				{ x-chs, y+chs, 0,   0, 1,   0 }
			};

			// create crosshair VAO
			crosshair.vao.begin(sizeof(gui_vertex_t), 4, &crosshairData);
			crosshair.vao.attrib(0, 3, GL_FLOAT, false, 0);
			crosshair.vao.attrib(1, 2, GL_FLOAT, false, offsetof(gui_vertex_t, u));
			crosshair.vao.attrib(2, 4, GL_UNSIGNED_BYTE, true, offsetof(gui_vertex_t, color));
			crosshair.vao.end();
		}

		crosshair.vao.render(GL_QUADS);
	}
}
