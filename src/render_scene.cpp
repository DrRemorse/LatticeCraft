#include "render_scene.hpp"

#include <library/log.hpp>
#include <library/opengl/fbo.hpp>
#include <library/opengl/opengl.hpp>
#include "camera.hpp"
#include "drawq.hpp"
#include "gameconf.hpp"
#include "minimap.hpp"
#include "netplayers.hpp"
#include "particles.hpp"
#include "player.hpp"
#include "renderman.hpp"
#include "render_fs.hpp"
#include "render_player_selection.hpp"
#include "render_sky.hpp"
#include "renderconst.hpp"
#include "sector.hpp"
#include "shaderman.hpp"
#include "spiders.hpp"
#include "sun.hpp"
#include "textureman.hpp"
#include "threading.hpp"
#include "world.hpp"
#include <cmath>

//#define TIMING
#ifdef TIMING
#include <library/timing/timer.hpp>
#endif

using namespace library;

namespace cppcraft
{
	static FBO skyFBO;
	static FBO sceneFBO;
	static FBO reflectionFBO;
	static FBO underwaterFBO;
	static FBO fboResolveColor;
	static FBO fogFBO, finalFBO;

	SceneRenderer::SceneRenderer(Renderer& renderer)
    : renderer(renderer)
	{
		// initialize members
		this->snapPlayerPos = player.pos;
    renderer.on_resize({this, &SceneRenderer::rebuild_scene});

    // initialize sky renderer
		skyrenderer.init();

    // inverted camera (mirrored on water plane)
    reflectionCamera.init(renderer);

    // initialize terrain renderer
		initTerrain();
  }

  void SceneRenderer::rebuild_scene(Renderer& renderer)
  {
		Texture& sceneTex = textureman[Textureman::T_SCENEBUFFER];

		// the FBO we render the sky to
		skyFBO.create();
		skyFBO.bind();
		skyFBO.attachColor(0, textureman[Textureman::T_SKYBUFFER]);

		// the FBO we render the main scene to
		sceneFBO.create();
		sceneFBO.bind();
		sceneFBO.attachColor(0, sceneTex);
		sceneFBO.attachDepth(textureman[Textureman::T_DEPTHBUFFER]);

		// the FBO we copy the main scene to before rendering water
		underwaterFBO.create();
		underwaterFBO.bind();
		underwaterFBO.attachColor(0, textureman[Textureman::T_UNDERWATERMAP]);
		underwaterFBO.attachDepth(textureman[Textureman::T_UNDERWDEPTH]);

		// multisampling resolver
		if (gameconf.multisampling)
		{
			logger << Log::INFO << "* Multisampling: " << gameconf.multisampling << "x" << Log::ENDL;
			fboResolveColor.create();
			fboResolveColor.bind();
			fboResolveColor.attachColor(0, textureman[Textureman::T_FINALBUFFER]);
			fboResolveColor.attachDepth(textureman[Textureman::T_FINALDEPTH]);
		}

		fogFBO.create();
		fogFBO.bind();
		fogFBO.attachColor(0, textureman[Textureman::T_RENDERBUFFER]);

		finalFBO.create();
		finalFBO.bind();
		finalFBO.attachColor(0, textureman[Textureman::T_FINALBUFFER]);

		if (gameconf.multisampling)
			finalFBO.attachDepth(textureman[Textureman::T_FINALDEPTH]);
		else
			finalFBO.attachDepth(textureman[Textureman::T_DEPTHBUFFER]);

		// reflection FBO & init
		if (gameconf.reflections)
		{
			Texture& reflections = textureman[Textureman::T_REFLECTION];

			// the FBO we render reflection to
			reflectionFBO.create();
			reflectionFBO.bind();
			reflectionFBO.attachColor(0, reflections);
			reflectionFBO.createDepthRBO(reflections.getWidth(), reflections.getHeight(), GL_DEPTH_COMPONENT24);
		}

		// initialize some shaders with (late) texture sizes
		glm::vec3 sceneSize = glm::vec3(sceneTex.getWidth(), sceneTex.getHeight(), renderer.aspect());

		shaderman[Shaderman::BLOCKS_WATER].bind();
		shaderman[Shaderman::BLOCKS_WATER].sendVec3("screendata", sceneSize);
		shaderman[Shaderman::BLOCKS_LAVA].bind();
		shaderman[Shaderman::BLOCKS_LAVA].sendVec3("screendata", sceneSize);

		shaderman[Shaderman::BLOCKS_DEPTH].bind();
		shaderman[Shaderman::BLOCKS_DEPTH].sendVec3("screensize", sceneSize);

		if (OpenGL::checkError())
		{
			logger << Log::ERR << "SceneRenderer::init(): OpenGL error. Line: " << __LINE__ << Log::ENDL;
			throw std::string("SceneRenderer::init(): General openGL error");
		}
	}

	// render normal scene
	void SceneRenderer::render()
	{
		bool frustumRecalc = false;

		Texture& skyTex = textureman[Textureman::T_SKYBUFFER];
		Texture& sceneTex = textureman[Textureman::T_SCENEBUFFER];
		Texture& renderBuffer = textureman[Textureman::T_RENDERBUFFER];

		glDisable(GL_CULL_FACE); // because the lower half hemisphere is rotated
		glDisable(GL_DEPTH_TEST);
		glDepthMask(GL_FALSE);

		/// update frustum rotation matrix ///
		if (camera.rotated)
		{
			camera.setRotation(player.rot.x, player.rot.y, 0.0f);
		}
		/// set sun view-direction (rotated)
		thesun.setRealtimeSunView(camera.getRotationMatrix());

		// bind the FBO that we are rendering the entire scene into
		skyFBO.bind();
		glViewport(0, 0, skyTex.getWidth(), skyTex.getHeight());

		/////////////////////////////////////////
		///   render atmosphere, moon, etc.   ///
		/////////////////////////////////////////
		skyrenderer.render(camera, -playerPos.y, renderer.time(), underwater ? 2 : 0);

		// copy sky to scene
		skyFBO.blitTo(sceneFBO,
					skyTex.getWidth(),
					skyTex.getHeight(),
					sceneTex.getWidth(),
					sceneTex.getHeight(),
					GL_COLOR_BUFFER_BIT, GL_NEAREST);


		////////////////////////////////////////////////////
		/// take snapshots of player state               ///
		/// and recalculate rendering queue if necessary ///
		////////////////////////////////////////////////////

		//if (mtx.sectorseam.try_lock())
		mtx.sectorseam.lock();
		{
			mtx.playermove.lock();
			{
				//------------------------------------//
				//          player snapshot           //
				//------------------------------------//

				double WEIGHT = 1.0;
        if (renderer.fps() > 0.0) {
          WEIGHT = std::min(1.0, 0.15 / (renderer.fps() / 120.0));
        }
				//logger << Log::INFO << WEIGHT << " <-- " << 120.0 / renderer.fps << Log::ENDL;

				// calculate distance to where we should be
				float dist = distance(snapPlayerPos, player.snap_pos);

				// (cheap) movement interpolation
				snapPlayerPos.y = mix(snapPlayerPos.y, player.snap_pos.y, WEIGHT);

				int dx = snapWX - world.getWX();
				int dz = snapWZ - world.getWZ();
				if (abs(dx) + abs(dz) < 3)
				{
					snapPlayerPos.x = mix(snapPlayerPos.x + dx * BLOCKS_XZ, player.snap_pos.x, WEIGHT);
					snapPlayerPos.z = mix(snapPlayerPos.z + dz * BLOCKS_XZ, player.snap_pos.z, WEIGHT);
				}
				else
				{
					snapPlayerPos.x = player.snap_pos.x;
					snapPlayerPos.z = player.snap_pos.z;
				}
				this->playerMoved = player.JustMoved;

				// position & sector snapshots
				this->playerPos = snapPlayerPos;
				this->playerSectorX = (int)playerPos.x / BLOCKS_XZ;
				this->playerSectorZ = (int)playerPos.z / BLOCKS_XZ;

				// underwater snapshot
				Block blk = Spiders::testArea(playerPos.x, playerPos.y - camera.getZNear(), playerPos.z);
				if (blk.isFluid())
				{
					//if (blockID == _WATER)
						this->underwater = 1;
					//else // _LAVABLOCK
					//	this->underwater = 2;

					// cheap hack to fix seeing into water
					this->playerPos.y -= camera.getZNear();
				}
				else this->underwater = 0;

				// update everything if we are too far from where we should be,
				// or something new has shown up and we need to force-update
				frustumRecalc = (dist > 0.01) || camera.recalc;
				camera.recalc = false;
			}
			mtx.playermove.unlock();

			if (frustumRecalc)
			{
				/// update matview matrix using player snapshot ///
				camera.setWorldOffset(playerPos.x, playerPos.y, playerPos.z);
				reflectionCamera.setWorldOffset(playerPos.x, playerPos.y, playerPos.z);
			}

			/// world coordinate snapshots ///
			this->snapWX = world.getWX();
			this->snapWZ = world.getWZ();
      this->m_delta_x = world.getDeltaX();
			this->m_delta_z = world.getDeltaZ();

			/// update minimap ///
			minimap.update(playerPos.x, playerPos.z);

			/// set player positions ///
			netplayers.positionSnapshots(snap_wx(), snap_wz(), renderer.delta_time());

			/// camera deviations ///
			double camDev = cameraDeviation(renderer.time(), renderer.delta_time());
			// modulate playerY when delta is high enough
			playerPos.y += camDev;
			// update frustum if there was a change
			frustumRecalc |= (fabs(camDev) > 0.001);

			//---------------------------------//
			// Start of frustum recalculations //
			//---------------------------------//

			// frustum.ref: run occlusion tests
			// - is never automatically disabled
			// frustum.needsupd = occlusion data gathering & processing
			// - once an update has happened, it is disabled

			if (frustumRecalc)
			{
				// -= recalculate frustum =-

				// do as little as possible this frame
				camera.needsupd = 1;
				// frustum was updated
				camera.ref = true;

#ifdef TIMING
				Timer timer;
#endif

				// process columns & modify occlusion
				recalculateFrustum();

#ifdef TIMING
				printf("Time spent recalculate frustum: %f\n", timer.getTime());
#endif
			}
			else
			{
				// not recalculating frustum
				camera.ref = false;

				// if last frame was an occlusion test
				if (camera.needsupd == 1)
				{
					// gather occlusion results
					camera.needsupd = 2;
				}
			}
			mtx.sectorseam.unlock();
		}

		// compress rendering queue to minimal size by occlusion culling
    {
#ifdef TIMING
		  Timer timer;
#endif
      compressRenderingQueue();
#ifdef TIMING
			printf("Time spent on draw queue: %f\n", timer.getTime());
#endif
    }
		/// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// ///


		// reflections only happen on the exact water-plane anyways,
		// so we just disable them completely when the player is below it
		if (gameconf.reflections)
		{
			if (playerPos.y >= WATERLEVEL && drawq[RenderConst::TX_WATER].size() != 0)
			{
				reflectionCamera.ref = camera.ref;
				reflectionCamera.rotated = camera.rotated;

				Texture& reflections = textureman[Textureman::T_REFLECTION];

				reflectionFBO.bind();
				// render at texture size
				glViewport(0, 0, reflections.getWidth(), reflections.getHeight());

				glDisable(GL_DEPTH_TEST);
				glDepthMask(GL_FALSE);

				// render sky (atmosphere, sun, moon, clouds)
				skyrenderer.render(reflectionCamera, playerPos.y - WATERLEVEL, renderer.time(), 1);

				if (gameconf.reflectTerrain)
				{
          // using clip plane to discard stuff under water
					glEnable(GL_CLIP_DISTANCE0);

          glEnable(GL_DEPTH_TEST);
					glDepthMask(GL_TRUE);
					glClear(GL_DEPTH_BUFFER_BIT);

					glEnable(GL_CULL_FACE);
					glCullFace(GL_FRONT);
					renderReflectedScene(reflectionCamera);
					glCullFace(GL_BACK);

					glDisable(GL_CLIP_DISTANCE0);
				}
			}
		}

		/// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// ///
		/// render physical scene w/depth
		/// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// ///

		sceneFBO.bind();
		glViewport(0, 0, sceneTex.getWidth(), sceneTex.getHeight());

		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);
		glDepthMask(GL_TRUE);

		// clear depth texture (or depth renderbuffer)
		glClear(GL_DEPTH_BUFFER_BIT);

		// disable double-sided faces
		glEnable(GL_CULL_FACE);

#ifdef TIMING
		Timer timerScene;
#endif

		// scene
		renderScene(camera);

		glEnable(GL_CULL_FACE);

		// render networked players
		netplayers.renderPlayers(renderer.time(), renderer.delta_time());

		// render player selection
		renderPlayerSelection();

		// blit terrain to (downsampled) underwatermap
		;{
			Texture& underwaterTex = textureman[Textureman::T_UNDERWATERMAP];
			sceneFBO.blitTo(underwaterFBO,
						sceneTex.getWidth(), sceneTex.getHeight(),
						underwaterTex.getWidth(), underwaterTex.getHeight(),
						GL_COLOR_BUFFER_BIT, GL_LINEAR);
			sceneFBO.blitTo(underwaterFBO,
						sceneTex.getWidth(), sceneTex.getHeight(),
						underwaterTex.getWidth(), underwaterTex.getHeight(),
						GL_DEPTH_BUFFER_BIT, GL_NEAREST);
		}
		sceneFBO.bind();

		glEnable(GL_CULL_FACE);

		// finally, render scene water
		renderSceneWater();

#ifdef TIMING
    printf("Time spent on scene: %f\n", timerScene.getTime());
#endif

		glDisable(GL_CULL_FACE);
		glDisable(GL_DEPTH_TEST);
		glDepthMask(GL_FALSE);

		/////////////////////////////////
		// resolve super/multisampling
		/////////////////////////////////
		if (gameconf.supersampling > 1)
		{
			glViewport(0, 0, renderBuffer.getWidth(), renderBuffer.getHeight());
			screenspace.renderSuperSampling(textureman[Textureman::T_SCENEBUFFER], textureman[Textureman::T_FINALBUFFER]);
			textureman.bind(0, Textureman::T_FINALBUFFER);
			textureman.bind(1, Textureman::T_SKYBUFFER);
			textureman.bind(2, Textureman::T_DEPTHBUFFER);
		}
		else if (gameconf.multisampling)
		{
			sceneFBO.blitTo(fboResolveColor, sceneTex.getWidth(), sceneTex.getHeight(), GL_COLOR_BUFFER_BIT, GL_NEAREST);
			sceneFBO.blitTo(fboResolveColor, sceneTex.getWidth(), sceneTex.getHeight(), GL_DEPTH_BUFFER_BIT, GL_NEAREST);

			textureman.bind(0, Textureman::T_FINALBUFFER);
			textureman.bind(1, Textureman::T_SKYBUFFER);
			textureman.bind(2, Textureman::T_FINALDEPTH);
		}
		else
		{
			textureman.bind(0, Textureman::T_SCENEBUFFER);
			textureman.bind(1, Textureman::T_SKYBUFFER);
			textureman.bind(2, Textureman::T_DEPTHBUFFER);
		}

		/////////////////////////////////
		// create fog based on depth
		/////////////////////////////////
		// --> inputs  T_SCENEBUFFER (or T_FINALBUFFER)
		// --> outputs T_RENDERBUFFER
		fogFBO.bind();
		screenspace.terrainFog(renderer.time());

		if (gameconf.distance_blur)
		{
			/////////////////////////////////
			// apply blur to background
			/////////////////////////////////
			renderBuffer.bind(0);

			// render to final buffer from renderbuffer
			// --> inputs  T_RENDERBUFFER
			// --> outputs T_FINALBUFFER
			finalFBO.bind();
			screenspace.terrainBlur();
		}
		else
		{
			// just copy from one texture to another
			// --> outputs T_FINALBUFFER
			fogFBO.blitTo(finalFBO, renderBuffer.getWidth(), renderBuffer.getHeight(), GL_COLOR_BUFFER_BIT, GL_NEAREST);
		}

		///  render clouds & particles  ///

		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);

		glEnable(GL_BLEND);
		glColorMask(1, 1, 1, 0);

		/// update particles ///
		particleSystem.renderUpdate();
		/// render particles ///
		particleSystem.render(snap_wx(), snap_wz(), renderer.time());

		/// render netplayer nametags ///
		netplayers.renderNameTags();

		glColorMask(1, 1, 1, 1);
		glDisable(GL_BLEND);

		finalFBO.unbind();

	} // render scene

}
