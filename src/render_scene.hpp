/**
 *
 *
 *
**/
#pragma once
#include <glm/vec3.hpp>

namespace cppcraft
{
	class Camera;
  class Column;
	class DrawQueue;
	class Renderer;
	class SkyRenderer;
	class WorldManager;

	class SceneRenderer {
	public:
		SceneRenderer(Renderer& renderer);

    void render();

		char isUnderwater() const
		{
			return underwater;
		}

    // return renderers current world position
    int snap_wx() const noexcept { return snapWX; }
    int snap_wz() const noexcept { return snapWZ; }
    int snap_delta_x() const noexcept { return m_delta_x; }
    int snap_delta_z() const noexcept { return m_delta_z; }

	private:
    Renderer& renderer;
		double lastTime = 0.0;

		// snapshots
		glm::vec3 snapPlayerPos;
		// used when rendering
		glm::vec3 playerPos;
		int playerSectorX, playerSectorZ;
		int snapWX = 0, snapWZ = 0;
    int m_delta_x = 0, m_delta_z = 0;
		bool playerMoved = true;
		char underwater = 0;

		// camera bobbing
		double motionTimed = 0.0;
		double lastCameraDeviation = 0.0;

		// camera bobbing computatron
		double cameraDeviation(double frameCounter, double dtime);

    void rebuild_scene(Renderer&);
		// juggernauts
		void initTerrain();
		void recalculateFrustum();
		void recalculateFrustum(Camera& camera, DrawQueue& drawq, const glm::vec3& look);
		void compressRenderingQueue();
		// main scene rendering function
		void renderScene(cppcraft::Camera& camera);
		void renderReflectedScene(cppcraft::Camera& camera);
		void renderSceneWater();

    static void renderColumn(Column*, int i);
    void renderColumnSet(int i);

		friend class SkyRenderer;
		friend class GUIRenderer;
	};
}
