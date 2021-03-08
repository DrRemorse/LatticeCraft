#include "render_player_selection.hpp"

#include <library/log.hpp>
#include <library/math/vector.hpp>
#include <library/opengl/opengl.hpp>
#include <library/opengl/vao.hpp>
#include "block.hpp"
#include "blockmodels.hpp"
#include "vertex_block.hpp"
#include "camera.hpp"
#include "player.hpp"
#include "player_actions.hpp"
#include "player_logic.hpp"
#include "renderconst.hpp"
#include "shaderman.hpp"
#include "textureman.hpp"
#include "threading.hpp"

using namespace library;

namespace cppcraft
{
	class PlayerSelection
	{
		VAO vao;
		bool renderable;

	public:
		void render();
	};
	PlayerSelection playerSelection;

	void renderPlayerSelection()
	{
		playerSelection.render();
	}

	int PlayerLogic::determineSelectionFacing(const Block& block, glm::vec3& ray, glm::vec3& fracs, float stepSize)
	{
		// determine selection-box facing value
		if (block.isLadder())
		{
			return 10;
		}
		/*else if (id == _WOODPOLE)
		{
			if (fracs.y >= 1.0 - stepSize)
			{
				return 2; // top of pole
			}
			else return 9; // select entire pole
		}*/
		else if (block.isCross())
		{
			return 6; // crosses box
		}
		else
		{
			// determine best-guess facing value for cube & halfblock

			float minv = fracs.x;
			if (minv > fracs.y) minv = fracs.y;
			if (minv > fracs.z) minv = fracs.z;

			if (fracs.x == minv)
			{
				if (fracs.x <= stepSize)
					return 5; // left side of a cube
			}
			else if (fracs.y == minv)
			{
				if (fracs.y <= stepSize)
					return 3; // bottom of a cube
			}
			else
			{
				if (fracs.z <= stepSize)
					return 1; // backside of cube
			}

			float maxv = fracs.x;
			if (maxv < fracs.y) maxv = fracs.y;
			if (maxv < fracs.z) maxv = fracs.z;

			if (fracs.x == maxv)
			{
				if (fracs.x >= 1.0 - stepSize)
					return 4; // right side of a cube
			}
			else if (fracs.y == maxv)
			{
				if (fracs.y >= 1.0 - stepSize)
					return 2; // top of a cube
			}
			if (fracs.z >= 1.0 - stepSize)
				return 0; // front of cube

			// since we didn't find any hits yet, it could be a halfblock
			// or a lowblock, which are at 1/2 and 1/8 respectively in height
			if (block.isHalfblock() || block.isLowblock())
				return 2; // half/low-block top
		}
		// no facing detected, return "something"
		return 0;

	} // selection box result

	void PlayerSelection::render()
	{
		// determine selection
		plogic.selection_mtx().lock();

		// exit if we have no selection
		if (plogic.hasSelection() == false)
		{
			plogic.selection_mtx().unlock();
			return;
		}

		int selection = plogic.selection.facing;
		int vx = (int)plogic.selection.pos.x;
		int vy = (int)plogic.selection.pos.y;
		int vz = (int)plogic.selection.pos.z;

		int model  = 0; //Block::blockModel(plogic.selection.block.getID());
		int bits = plogic.selection.block.getBits();

		bool updated = plogic.selection.updated;
		plogic.selection.updated = false;

		plogic.selection_mtx().unlock();

		if (updated)
		{
			// build vertices
			std::vector<selection_vertex_t> vertices;

			// determine selection mesh
			if (selection < 6)
			{
				// regular cube face
				blockmodels.selectionCube[model].copyTo(selection, vertices);
			}
			else if (selection == 6) // cross selection box
			{
				// cross selection box
				blockmodels.selecionCross.copyTo(0, vertices);
			}
			else if (selection == 9) // pole selection box
			{
				// cross selection box
				blockmodels.selectionPole.copyTo(0, vertices);
			}
			else if (selection == 10) // ladders selection box
			{
				// ladder selection box
				blockmodels.selectionLadder.copyTo(bits, vertices);
			}

			// upload only if renderable
			this->renderable = vertices.empty() == false;
			if (this->renderable)
			{
				// upload selection rendering data
				vao.begin(sizeof(selection_vertex_t), vertices.size(), vertices.data());
				vao.attrib(0, 3, GL_FLOAT, GL_FALSE, offsetof(selection_vertex_t, x));
				vao.attrib(1, 2, GL_FLOAT, GL_FALSE, offsetof(selection_vertex_t, u));
				vao.end();
			}
		}

		if (this->renderable == false) return;

		// enable blending
		glEnable(GL_BLEND);
		glDepthMask(GL_FALSE);
		// --> fixes sun visible through selection
		glColorMask(1, 1, 1, 0);

		/// render player selection ///
		Shader* shd = nullptr;

		// mining
		if (paction.getAction() == PlayerActions::PA_Mineblock)
		{
			textureman.bind(0, Textureman::T_MINING);

			// selection shader
			shd = &shaderman[Shaderman::SELECTION_MINING];
			shd->bind();

			// mine tile id
			int tileID = paction.getMiningLevel() * 9.0;
			shd->sendFloat("miningTile", tileID);
		}
		else
		{
			textureman.bind(0, Textureman::T_SELECTION);

			// normal selection shader
			shd = &shaderman[Shaderman::SELECTION];
			shd->bind();
		}

		shd->sendMatrix("matmvp", camera.getMVP());
		// position in space
		if (updated)
			shd->sendVec3("vtrans", glm::vec3(vx, vy, vz));

		// render quad(s)
		vao.render(GL_QUADS);

		glDisable(GL_BLEND);
		glDepthMask(GL_TRUE);

		glColorMask(1, 1, 1, 1);
	}
}
