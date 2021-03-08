#include "render_gui.hpp"

#include <library/log.hpp>
#include <library/bitmap/colortools.hpp>
#include <library/opengl/opengl.hpp>
#include <library/opengl/vao.hpp>
#include <library/math/toolbox.hpp>
#include "blockmodels.hpp"
#include "camera.hpp"
#include "game.hpp"
#include "player.hpp"
#include "player_actions.hpp"
#include "player_logic.hpp"
#include "sun.hpp"
#include "shaderman.hpp"
#include "textureman.hpp"
#include "voxelmodels.hpp"
#include <cmath>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtx/transform.hpp>
#include <library/math/matrix.hpp>

using namespace library;

namespace cppcraft
{
	static const double PI = 4 * atan(1);

	class PlayerHand
	{
		VAO vao;
		VAO cubeVAO;
    glm::vec3 currentHand;
		glm::vec3 lastHand;
		double    lastTime;
		int       lastMode;

		struct hvertex_t
		{
			float x, y, z;
			float nx, ny, nz;
			float u, v, w;
		};

		std::array<hvertex_t, 8> vertices;

  public:
		PlayerHand();
		void render(double frameCounter);
		void renderItem();
		void renderHandItem(const glm::vec2& light, float modulation);
	};
	PlayerHand playerHand;

	void GUIRenderer::renderPlayerhand(double frameCounter)
	{
		playerHand.render(frameCounter);
	}

	PlayerHand::PlayerHand()
	{
		lastMode = -1;
		lastTime = 0.0;
    // hand scale
    const glm::vec3 scale {0.25, 0.25, 1.0};

              //   x        y        z          norm        uvw
              // top face
vertices[0] = { scale.x, scale.y, scale.z,   0, 1, 0,   0, 1.5, 9 };
vertices[1] = { scale.x, scale.y,       0,   0, 1, 0,   0, 0.0, 9 };
vertices[2] = {       0, scale.y,       0,   0, 1, 0,   1, 0.0, 9 };
vertices[3] = {       0, scale.y, scale.z,   0, 1, 0,   1, 1.5, 9 };
              // left face
vertices[4] = {       0, scale.y, scale.z,  -1, 0, 0,   0, 1.5, 9 };
vertices[5] = {       0, scale.y,       0,  -1, 0, 0,   0, 0.0, 9 };
vertices[6] = {       0,       0,       0,  -1, 0, 0,   1, 0.0, 9 };
vertices[7] = {       0,       0, scale.z,  -1, 0, 0,   1, 1.5, 9 };
	}

	void PlayerHand::render(double frameCounter)
	{
		const double DEFAULT_INTERPOLATION = 0.75;

		// manipulate hand
		double period = (frameCounter - lastTime) * 0.25 / PI;
		glm::vec3 hand(0.6, -0.5, -1.0);
		int mode = 0;

		if (paction.getAction() == PlayerActions::PA_Mineblock)
		{
			// mining animation
			hand.x -= fabs(sin(period * 1.5) * 0.5);
			hand.y += sin(period * 3) * 0.1;
			if (false) //helditem.count)
				hand.z = -0.5 - fabs(sin(period * 1.5)) * 0.5;
			else
				hand.z = -0.5 - fabs(sin(period * 1.5)) * 0.5;

			mode = 1;
		}
		else if (paction.getAction() == PlayerActions::PA_Swingtool)
		{
			// swinging animation
			hand.x -= fabs(sin(period * 1.0)) * 0.75;
			hand.y = -0.5 - period * 0.1;

			hand.z = -0.25 - fabs(sin(period * 1.0)) * 0.6;
			mode = 2;
		}
		else if (paction.getAction() == PlayerActions::PA_Cooldown)
		{
			// cooldown animation
			hand.y += sin(period / PI * 2) * 0.1;
			if (false) //helditem.count)
				hand.z = -0.5 - fabs(sin(period)) * 1.0;
			else
				hand.z = -0.5 - fabs(sin(period)) * 1.0;

			if (lastMode != mode) lastTime = frameCounter - paction.cooldownTime;
			mode = 3;
		}
		else
		{
			if (plogic.FullySubmerged != PlayerLogic::PS_None)
			{
				hand.y += sin(period * 1.0) * 0.1;
				hand.z -= sin(period * 0.5) * 0.1;
				mode = 15;
			}
			else if (plogic.freefall)
			{
				float dy = clamp(-0.4f, 0.4f, -player.accel.y);
				if (plogic.Submerged != PlayerLogic::PS_None) dy = 0.0;

				hand.y += dy + sin(period * 1.0) * 0.1;
				hand.z -=      sin(period * 0.5) * 0.1;
				mode = 16;
			}
			else if (player.JustMoved)
			{
				// normal animation (based on player speed)

				if (player.Flying)
				{
					hand.y += sin(period * 2.0) * 0.1;
					hand.z -= sin(period * 1.0) * 0.1;
					mode = 18;
				}
				else if (plogic.movestate == PMS_Sprint)
				{
					hand.y += sin(period * 3.00) * 0.1;
					hand.z -= sin(period * 1.50) * 0.1;
					mode = 19;
				}
				else
				{
					hand.y += sin(period * 2.0) * 0.1;
					hand.z -= sin(period * 1.0) * 0.1;
					mode = 20;
				}
			}
			else if (plogic.movestate == PMS_Crouch)
			{
				// resting crouch
				hand.y += sin(period * 1.0) * 0.05;
				hand.z -= sin(period * 0.5) * 0.05;
				mode = 21;
			}
			else
			{
				// resting normal
				hand.y += sin(period * 0.25) * 0.1;
				hand.z -= sin(period * 0.5) * 0.05;
				mode = 22;
			}

		} // player action

		if (lastMode == -1 || lastMode == mode)
		{
			lastHand = mix(hand, lastHand, 0.5f);

			if (lastMode == -1)
			{
				lastMode = mode;
				lastTime = frameCounter;
			}
		}
		else
		{
			// interpolate slowly to move the hand to the new position
			lastHand = mix(hand, lastHand, DEFAULT_INTERPOLATION);
			// only set new mode once we get close enough to the new position
			if (fabsf(hand.y - lastHand.y) < 0.01 &&
				fabsf(hand.z - lastHand.z) < 0.01)
			{
				lastMode = mode;
				lastTime = frameCounter;
			}
		}
    // set the hand actually used this frame
    this->currentHand = hand;

		// convert shadow & torchlight color to 4-vectors
		glm::vec2 light = plogic.getLight();
		float modulation = 1.0f; //torchlight.getModulation(frameCounter);

		renderHandItem(light, modulation);

		// render player hand
		Shader& shd = shaderman[Shaderman::PLAYERHAND];
		shd.bind();
		// lighting & ambience
		shd.sendVec3("lightVector", thesun.getRealtimeAngle());
		shd.sendFloat("daylight", thesun.getRealtimeDaylight());
		// player shadow & torchlight color
		shd.sendVec2("lightdata", light);
		// torchlight modulation
		shd.sendFloat("modulation", modulation);

		// send rotation matrix when camera has been rotated
		if (camera.rotated)
		{
			shd.sendMatrix("matrot", camera.getRotationMatrix());
		}

		// view translation-matrix
		glm::mat4 matview =
        glm::translate(glm::vec3(lastHand.x, lastHand.y, lastHand.z));

		shd.sendMatrix("matview", matview);

		// bind player models texture
		textureman.bind(0, Textureman::T_PLAYERMODELS);

		// update vertex data
		if (vao.good() == false)
		{
			vao.begin(sizeof(hvertex_t), 8, &vertices[0]);
			vao.attrib(0, 3, GL_FLOAT, GL_FALSE, offsetof(hvertex_t, x));
			vao.attrib(1, 3, GL_FLOAT, GL_FALSE, offsetof(hvertex_t, nx));
			vao.attrib(2, 3, GL_FLOAT, GL_FALSE, offsetof(hvertex_t, u));
			vao.end();
		}

		// finally, render...
		vao.render(GL_QUADS);
	}

	void PlayerHand::renderHandItem(const glm::vec2& light, float modulation)
	{
		// render held item
		const auto& helditem = game.gui().hotbar_item();
		// no item, no render
		if (helditem.isNone()) return;

		glEnable(GL_DEPTH_TEST);
		glDepthMask(GL_TRUE);

		if (helditem.isVoxel())
		{
			Shader& shd = shaderman[Shaderman::VOXEL];
			shd.bind();
			// player shadow & torchlight color
			shd.sendVec2("lightdata", light);
			// torchlight modulation
			shd.sendFloat("modulation", modulation);
      // rotation to make it LookGood(TM)
      const glm::vec3 hr = helditem.itemdb().getHandRotation() * 3.14159f / 180.0f;
      const glm::vec3& ht = helditem.itemdb().getHandTranslation();
			// view matrix
			glm::mat4 matview = glm::translate(lastHand + ht);
      matview *= glm::scale(glm::vec3(0.75f));
			glm::mat4 matrot = rotationMatrix(hr.x, hr.y + PI / 2.0f, hr.z);
			matview *= matrot;

			shd.sendMatrix("matnrot", matrot);
			shd.sendMatrix("matmvp", camera.getProjection() * matview);

			// rotated lighting & ambience
			shd.sendVec3("lightVector", thesun.getRealtimeAngle());
			shd.sendFloat("daylight", thesun.getRealtimeDaylight());
			// send rotation matrix when camera has been rotated
			if (camera.rotated)
			{
				shd.sendMatrix("matrot", camera.getRotationMatrix());
			}

			VoxelModels::render(helditem.getVoxelModel());
		}
		else
		{
			// bind blocks diffuse texture
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D_ARRAY, helditem.getTexture());
			// helditem block mesh shader
			Shader& shd = shaderman[Shaderman::PHAND_HELDITEM];
			shd.bind();
			// lighting & ambience
			shd.sendVec3("lightVector", thesun.getRealtimeAngle());
			shd.sendFloat("daylight", thesun.getRealtimeDaylight());
			// player shadow & torchlight color
			shd.sendVec2("lightdata", light);
			// torchlight modulation
			shd.sendFloat("modulation", modulation);

			// send rotation matrix when camera has been rotated
			if (camera.rotated)
			{
				shd.sendMatrix("matrot", camera.getRotationMatrix());
			}

			// view matrix
			glm::mat4 matview;

			// generate held item from item type
			static std::vector<vertex_t> vertices;
      vertices.clear();

			/*if (id == _LANTERN)
			{
				// lantern model
				count = blockmodels.lanterns.totalCount();
				vertices = new vertex_t[count];
				blockmodels.lanterns.copyAll(vertices);

				// texture id
				for (int i = 0; i < count; i++)
					vertices[i].w = Block::cubeFaceById(helditem.getID(), (i / 24) * 2, 0);

				// translation & scaling
				matview = glm::scale(glm::vec3(0.5f));
				matview *= glm::translate(glm::vec3(lastHand.x - 0.35f, lastHand.y, lastHand.z - 0.6f));
			}
			else*/
			{
        // convert item to a block
				Block held = helditem.toBlock();
				// block model index
				const int model = held.db().model();

				blockmodels.cubes[model].copyAll(vertices);

				held.setBits(3); // assuming bits are used to determine direction
				// texture id
				for (size_t i = 0; i < vertices.size(); i++)
				{
					vertices[i].w = held.getTexture(i / 4);
				}

				// translation & scaling
        matview = glm::translate(glm::vec3(-0.25, -0.7, -1));
        matview *= glm::translate(this->lastHand);
			}

			if (!vertices.empty())
			{
				shd.sendMatrix("matview", matview);

				cubeVAO.begin(sizeof(vertex_t), vertices.size(), vertices.data());
				cubeVAO.attrib(0, 3, GL_SHORT, GL_FALSE, offsetof(vertex_t, x));
				cubeVAO.attrib(1, 3, GL_BYTE,  GL_TRUE,  offsetof(vertex_t, nx));
				cubeVAO.attrib(2, 3, GL_SHORT, GL_FALSE, offsetof(vertex_t, u));
				cubeVAO.end();

				// finally, render...
				cubeVAO.render(GL_QUADS);
			}

		}

		glDisable(GL_DEPTH_TEST);
		glDepthMask(GL_FALSE);
	}

}
