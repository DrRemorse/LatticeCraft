#ifndef RENDERSKY_HPP
#define RENDERSKY_HPP

#include <glm/mat4x4.hpp>

namespace cppcraft
{
	class Camera;
	
	class SkyRenderer
	{
	private:
		void createClouds();
		void renderSun();
		void renderMoon(Camera& camera);
		
	public:
		void init();
		void render(Camera& camera, float cloudLevel, double time, int mode);
		void renderClouds(float dy, Camera& camera, double frameCounter);
		
		glm::mat4 renderSunProj();
		
	};
	extern SkyRenderer skyrenderer;
}

#endif
