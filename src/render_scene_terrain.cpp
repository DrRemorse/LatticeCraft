#include "render_scene.hpp"

#include <library/log.hpp>
#include <library/opengl/fbo.hpp>
#include <library/opengl/opengl.hpp>
#include <library/opengl/texture_buffer.hpp>
#include "columns.hpp"
#include "drawq.hpp"
#include "camera.hpp"
#include "gameconf.hpp"
#include "player.hpp"
#include "player_logic.hpp"
#include "renderman.hpp"
#include "shaderman.hpp"
#include "sun.hpp"
#include "tiles.hpp"
#include "textureman.hpp"
#include <cmath>
#include <glm/gtx/transform.hpp>

using namespace library;

namespace cppcraft
{
	static const double PI = 4 * atan(1);
  static std::unique_ptr<BufferTexture> m_buffer_texture = nullptr;
  // setup the scene buffer texture array
  static std::unique_ptr<glm::vec3[]> m_bt_data = nullptr;

	void SceneRenderer::initTerrain()
	{
		// initialize column drawing queue
		drawq.init();

    // column translation array
    m_bt_data = std::make_unique<glm::vec3[]> (columns.size());
    m_buffer_texture.reset(
      new BufferTexture(columns.size() * 3 * sizeof(float), GL_RGB32F));
	}

	void SceneRenderer::recalculateFrustum()
	{
		glm::vec3 look = player.getLookVector();

		// recalculate view & mvp matrix
		// view matrix (rotation + translation)
		camera.setTranslation(-playerPos.x, -playerPos.y, -playerPos.z);

		recalculateFrustum(camera, drawq, look);

		if (gameconf.reflections)
		{
			// set reflection camera view
			glm::mat4 matref = camera.getViewMatrix();
			matref *= glm::translate(glm::vec3(0.f, WATERLEVEL * 2.0f, 0.f));
			matref *= glm::scale(glm::vec3(1.0f, -1.0f, 1.0f));

			reflectionCamera.setRotationMatrix(camera.getRotationMatrix());
			reflectionCamera.setViewMatrix(matref);

			if (gameconf.reflectTerrain)
			{
				look.y = -look.y;
				recalculateFrustum(reflectionCamera, reflectionq, look);
			}
		}
	}

	void SceneRenderer::recalculateFrustum(cppcraft::Camera& camera, DrawQueue& drawq, const glm::vec3& look)
	{
		// recalculate camera frustum
		camera.calculateFrustum();
		#define VISIBILITY_BORDER  DrawQueue::VISIBILITY_BORDER

    DrawQueue::rendergrid_t rg;
    // set frustum culling settings
		rg.playerY  = playerPos.y;
    rg.wdx = this->snap_delta_x();
    rg.wdz = this->snap_delta_z();
		rg.frustum = &camera.getFrustum();
		rg.gridSize = camera.getGridsize();

		// major direction scheme
		int x0, x1;
		if (look.x >= 0.0f)
		{
			x0 = VISIBILITY_BORDER;
			x1 = sectors.getXZ()-1-VISIBILITY_BORDER;
      rg.xstp = 1;
		}
		else
		{
			x1 = VISIBILITY_BORDER;
			x0 = sectors.getXZ()-1-VISIBILITY_BORDER;
      rg.xstp = -1;
		}

		int z0, z1;
		if (look.z >= 0.0f)
		{
			z0 = VISIBILITY_BORDER;
			z1 = sectors.getXZ()-1-VISIBILITY_BORDER;
      rg.zstp = 1;
		}
		else
		{
			z1 = VISIBILITY_BORDER;
			z0 = sectors.getXZ()-1-VISIBILITY_BORDER;
      rg.zstp = -1;
		}

		rg.ystp = (look.y < 0.0f) ? -1 : 1;

		// determine iteration direction
		if (fabsf(look.x) > fabsf(look.z))
		{
			if (look.x >= 0)
				rg.majority = 0; // +x
			else
				rg.majority = 1; // -x
		}
		else if (look.z >= 0)
			rg.majority = 2; // +z
		else
			rg.majority = 3; // -z

		// reset drawing queue
		drawq.reset();

		// start at roomsize 1, avoiding "everything"
		drawq.uniformGrid(rg, x0, x1, z0, z1, 1);
	}

	void SceneRenderer::compressRenderingQueue()
	{
		if (camera.needsupd == 0) return;
		// reset after 2nd run
		else if (camera.needsupd == 2) camera.needsupd = 0;

		for (int i = 0; i < RenderConst::MAX_UNIQUE_SHADERS; i++)
		{
			// loop through this shader line
			for (auto* cv : drawq[i])
			{
        // if the column is still rising up, let it rise
        if (cv->pos.y < 0.0) {
          cv->pos.y += 0.25f * renderer.delta_time();
          if (cv->pos.y > 0.0f) cv->pos.y = 0.0f;
        }
        // set position for column
        m_bt_data.get()[cv->index()] = cv->pos;
			}
		} // next shaderline

    // upload array of vec3
    m_buffer_texture->upload(m_bt_data.get(), columns.size() * 3 * sizeof(float));
	}

	void SceneRenderer::renderColumn(Column* cv, int i)
	{
		glBindVertexArray(cv->vao);
		//glDrawElements(GL_TRIANGLES, cv->indices[i], GL_UNSIGNED_SHORT, (GLvoid*) (intptr_t) cv->indexoffset[i]);
		glDrawArrays(GL_QUADS, cv->bufferoffset[i], cv->vertices[i]);
	}

	void SceneRenderer::renderColumnSet(int i)
	{
		// direct render
		for (auto* column : drawq[i])
		{
			renderColumn(column, i);
		}
	}

	void handleSceneUniforms(
			double frameCounter,
			Shader& shd,
			cppcraft::Camera& camera
		)
	{
		// bind appropriate shader
		shd.bind();

		// camera updates
		if (camera.ref)
		{
			// modelview matrix
			shd.sendMatrix("matview", camera.getViewMatrix());
			// mvp matrix
			shd.sendMatrix("matmvp", camera.getMVP());
		}

		// common stuff
		shd.sendVec3 ("lightVector", thesun.getRealtimeAngle());
		shd.sendFloat("daylight",    thesun.getRealtimeDaylight());
		shd.sendFloat("frameCounter", frameCounter);

		shd.sendFloat("modulation", 1.0f); //torchlight.getModulation(frameCounter));
	}

	void SceneRenderer::renderScene(cppcraft::Camera& renderCam)
	{
		textureman.bind(2, Textureman::T_SKYBOX);
    // translation buffer texture
    m_buffer_texture->bind(8);

		// bind standard shader
		handleSceneUniforms(renderer.time(),
							shaderman[Shaderman::STD_BLOCKS],
							renderCam);

#ifdef OPENGL_DO_CHECKS
		if (UNLIKELY(OpenGL::checkError()))
		{
			logger << Log::ERR << "Renderer::renderScene(): OpenGL error. Line: " << __LINE__ << Log::ENDL;
			throw std::string("Renderer::renderScene(): OpenGL state error");
		}
#endif

		// render all nonwater shaders

		for (int i = 0; i < (int) RenderConst::TX_WATER; i++)
		{
			switch (i) {
			case RenderConst::TX_REPEAT: // repeatable solids (most terrain)

				// big repeatable textures
        tiledb.bigtiles.diff_texture().bind(0);
        tiledb.bigtiles.tone_texture().bind(1);
				break;

			case RenderConst::TX_SOLID: // solid stuff (most blocks)

				// change to small, repeatable textures
				tiledb.tiles.diff_texture().bind(0);
        tiledb.tiles.diff_texture().setWrapMode(GL_REPEAT);
        tiledb.tiles.tone_texture().bind(1);
        tiledb.tiles.tone_texture().setWrapMode(GL_REPEAT);
				break;

			case RenderConst::TX_TRANS_1SIDED: // see-through (tree leafs etc.)

				// change shader-set
				handleSceneUniforms(renderer.time(),
									shaderman[Shaderman::CULLED_BLOCKS],
									renderCam);
				break;

			case RenderConst::TX_TRANS_2SIDED: // 2-sided faces (torches, vines etc.)

        // change shader-set
        {
          auto& shd = shaderman[Shaderman::ALPHA_BLOCKS];
          handleSceneUniforms(renderer.time(), shd, renderCam);
        }

        // disable face culling for 2-sidedness
        glDisable(GL_CULL_FACE);

				// enable texture clamping
        tiledb.tiles.diff_texture().setWrapMode(GL_CLAMP_TO_EDGE);
				glActiveTexture(GL_TEXTURE0);
        tiledb.tiles.diff_texture().setWrapMode(GL_CLAMP_TO_EDGE);

				// safe to increase step from this -->
				if (drawq[i].size() == 0) continue;
				// <-- safe to increase step from this
				break;
			}

			// render it all
			renderColumnSet(i);

		} // next shaderline
	}

	void SceneRenderer::renderReflectedScene(cppcraft::Camera& renderCam)
	{
    // translation buffer texture
    m_buffer_texture->bind(8);
		// bind standard shader
		handleSceneUniforms(renderer.time(),
							shaderman[Shaderman::BLOCKS_REFLECT],
							renderCam);

#ifdef OPENGL_DO_CHECKS
		if (OpenGL::checkError())
		{
			logger << Log::ERR << "Renderer::renderReflectedScene(): OpenGL error. Line: " << __LINE__ << Log::ENDL;
			throw std::string("Renderer::renderReflectedScene(): OpenGL state error");
		}
#endif

		for (int i = 0; i < (int) RenderConst::TX_WATER; i++)
		{
			switch (i)
			{
			case RenderConst::TX_REPEAT: // repeatable solids (most terrain)

				// change to big tile textures
        tiledb.bigtiles.diff_texture().bind(0);
        tiledb.bigtiles.tone_texture().bind(1);
				break;

			case RenderConst::TX_SOLID: // solid stuff (most blocks)

				// change to normal tile textures
        tiledb.tiles.diff_texture().bind(0);
        tiledb.tiles.tone_texture().bind(1);
				break;

			case RenderConst::TX_TRANS_2SIDED: // 2-sided faces (crosses, vines etc.)

				// disable face culling (for 2-sidedness)
				glDisable(GL_CULL_FACE);
				break;
			}

			// direct render
      for (auto* cv : reflectionq[i])
			{
				renderColumn(cv, i);
			}
		} // next shaderline

	} // renderReflectedScene()

	void SceneRenderer::renderSceneWater()
	{
    // translation buffer texture
    m_buffer_texture->bind(8);

#ifdef OPENGL_DO_CHECKS
		if (UNLIKELY(OpenGL::checkError()))
		{
			logger << Log::ERR << "Renderer::renderSceneWater(): OpenGL error @ top. Line: " << __LINE__ << Log::ENDL;
			throw std::string("Renderer::renderSceneWater(): OpenGL state error");
		}
#endif

		// bind underwater scene
		textureman.bind(0, Textureman::T_UNDERWATERMAP);
		textureman.bind(1, Textureman::T_UNDERWDEPTH);

		if (this->isUnderwater())
		{
			// underwater shader-set
			handleSceneUniforms(renderer.time(),
								shaderman[Shaderman::BLOCKS_DEPTH],
								camera);
			// cull only front water-faces inside water
			glCullFace(GL_FRONT);
		}
		else
		{
			if (gameconf.reflections)
			{
				// bind world-reflection
				textureman.bind(2, Textureman::T_REFLECTION);
			}
		}

#ifdef OPENGL_DO_CHECKS
    if (UNLIKELY(OpenGL::checkError()))
		{
			logger << Log::ERR << "Renderer::renderSceneWater(): OpenGL error @ middle. Line: " << __LINE__ << Log::ENDL;
			throw std::string("Renderer::renderSceneWater(): OpenGL state error");
		}
#endif

		// FIXME: need to render water running_water and lava separately instead of "accepting fully submerged"
		// as not rendering anything but depth values

		if (this->isUnderwater() == false)
		{
			// fluid shaders
			for (int i = RenderConst::TX_WATER; i < RenderConst::MAX_UNIQUE_SHADERS; i++)
			{
				switch (i)
				{
				case RenderConst::TX_WATER:

					// safe to increase step from this -->
					if (drawq[i].size() == 0) continue;
					// <-- safe to increase step from this

					// water shader-set
					handleSceneUniforms(renderer.time(),
										shaderman[Shaderman::BLOCKS_WATER],
										camera);
					// sun view angle
					shaderman[Shaderman::BLOCKS_WATER].sendVec3 ("v_ldir", thesun.getRealtimeViewAngle());
					// update world offset
					if (camera.ref)
						shaderman[Shaderman::BLOCKS_WATER].sendVec3("worldOffset", camera.getWorldOffset());
					break;

				case RenderConst::TX_LAVA:

					// safe to increase step from this -->
					if (drawq[i].size() == 0) continue;
					// <-- safe to increase step from this

					textureman.bind(2, Textureman::T_MAGMA);
					// lava shader-set
					handleSceneUniforms(renderer.time(),
										shaderman[Shaderman::BLOCKS_LAVA],
										camera);
					// update world offset
					if (camera.ref)
						shaderman[Shaderman::BLOCKS_LAVA].sendVec3("worldOffset", camera.getWorldOffset());
					break;
				}

				// render it all
				renderColumnSet(i);

			} // each shader

		} // underwater
		else
		{
			if (this->isUnderwater() == 1)
			{
				// render water meshes
				renderColumnSet(RenderConst::TX_WATER);
			}
			else
			{
				// render lava meshes
				renderColumnSet(RenderConst::TX_LAVA);
			}
			// restore cullface setting
			glCullFace(GL_BACK);
		}

#ifdef OPENGL_DO_CHECKS
    if (UNLIKELY(OpenGL::checkError()))
		{
			logger << Log::ERR << "Renderer::renderSceneWater(): OpenGL error @ bottom. Line: " << __LINE__ << Log::ENDL;
			throw std::string("Renderer::renderSceneWater(): OpenGL state error");
		}
#endif

	} // renderSceneWater()

}
