#ifndef RENDER_GUI_HPP
#define RENDER_GUI_HPP

#include <glm/mat4x4.hpp>
#include "items/item.hpp"

namespace cppcraft
{
	class Renderer;

	class GUIRenderer
	{
	public:
		struct gui_vertex_t
		{
			float x, y, z;
			float u, v;
			unsigned int color;
		};

		void init(Renderer& renderer);
		void render(Renderer& renderer);

		const glm::mat4& getOrtho() const
		{
			return ortho;
		}

	private:
		float width;
		float height;
		// the GUIs orthographic projection matrix
		glm::mat4 ortho;

		void renderPlayerhand(double frameCounter);
		void renderMinimap(glm::mat4& ortho);
		void renderCrosshair(glm::mat4& ortho);
		void renderMenuSystem(glm::mat4& ortho, double frameCounter);
	};
	extern GUIRenderer rendergui;
}
#endif
