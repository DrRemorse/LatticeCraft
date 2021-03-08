#ifndef ATMOSPHERE_HPP
#define ATMOSPHERE_HPP

namespace cppcraft
{
	class Camera;
	
	class Atmosphere
	{
	public:
		static void init();
		static void render(Camera& camera, int mode);
		
	};
}

#endif
