#include "camera.hpp"

#include <library/log.hpp>
#include <library/opengl/window.hpp>
#include <library/math/matrix.hpp>
#include "drawq.hpp"
#include "gameconf.hpp"
#include "sectors.hpp"
#include "world.hpp"
#include <cmath>

using namespace library;

namespace cppcraft
{
	// Viewing camera
	cppcraft::Camera camera;
	cppcraft::Camera reflectionCamera;

	void Camera::init(Renderer& renderer)
	{
		logger << Log::INFO << "* Initializing frustum" << Log::ENDL;

		if (sectors.getXZ() >= 80)
			this->gridsize = 32;
		else if (sectors.getXZ() >= 40)
			this->gridsize = 16;
		else
			this->gridsize = 8;

		// sectors view depth-range
		this->cameraViewSectors = sectors.getXZ() / 2 - DrawQueue::VISIBILITY_BORDER;
		// unit blocks depth-range
		this->FOV   = config.get("frustum.fov", 61.0f);
		this->znear = 0.136;
		this->zfar  = cameraViewSectors * Sector::BLOCKS_XZ;

    renderer.on_resize({this, &Camera::update});
	}
  void Camera::update(Renderer& renderer)
  {
    setProjection(this->FOV, renderer.aspect(), this->znear, this->zfar);

		// long projection matrix
		matproj_long = perspectiveMatrix(this->FOV, renderer.aspect(), this->znear, this->zfar * 1.6);
  }

	void Camera::setWorldOffset(double posX, double posY, double posZ)
	{
		// calculate sectoral offsets
		int halfWorld = sectors.getXZ() * Sector::BLOCKS_XZ / 2;

		int dwx = World::WORLD_STARTING_X - world.getWX();
		dwx = dwx * Sector::BLOCKS_XZ - halfWorld;

		int dwz = World::WORLD_STARTING_Z - world.getWZ();
		dwz = dwz * Sector::BLOCKS_XZ - halfWorld;

		// position.xz - center of local area (posX and posZ is NEGATIVE)
		worldOffset.x = dwx - posX;
		worldOffset.y = 0   - posY;
		worldOffset.z = dwz - posZ;
	}

}
