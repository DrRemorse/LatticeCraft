#include "player.hpp"

#include <library/log.hpp>
#include "camera.hpp"
#include "chat.hpp"
#include "game.hpp"
#include "player_actions.hpp"
#include "player_logic.hpp"
#include "player_physics.hpp"
#include "spiders.hpp"
#include "threading.hpp"
#include <cmath>

using namespace glm;

namespace cppcraft
{
	// player object
	PlayerClass player;

	void PlayerClass::initPlayer()
	{
		plogic.movestate = PMS_Normal;
		plogic.sector = nullptr;
		plogic.block = &air_block;

		// initialize block selection system
		plogic.selection = playerselect_t();

		// initialize action/interaction system
		paction.init();

		player.snapStage = 0;
		player.snap_pos = vec3(0.0f);
	}

	bool PlayerClass::busyControls() const
	{
		return chatbox.isOpen();
	}

	glm::vec3 PlayerClass::getLookVector() const
	{
		float dx =  sinf(player.rot.y) * cosf(player.rot.x);
		float dy = 		                -sinf(player.rot.x);
		float dz = -cosf(player.rot.y) * cosf(player.rot.x);

		return glm::vec3(dx, dy, dz);
	}

	block_t PlayerClass::getBlockFacing() const
	{
		vec2 sightXZ( sinf(player.rot.y), -cosf(player.rot.y) );

		block_t facing = 0;

		if (sightXZ.x < -0.707) // left
			facing = 2;
		else if (sightXZ.x > 0.707) // right
			facing = 3;

		if (sightXZ.y < -0.707) // front -> back
			facing = 0;
		else if (sightXZ.y > 0.707) // back -> front
			facing = 1;

		return facing;
	}

	bool PlayerClass::fullySubmerged() const
	{
		return plogic.FullySubmerged;
	}

	void PlayerClass::handlePlayerTicks(double time)
	{
		// acceleration & movement inputs
		playerPhysics.handleMomentum();
		// jumping, big enough to get its own module
		plogic.handlePlayerJumping();
		// translate player (gravity & acceleration)
		plogic.translatePlayer();
		// play some sounds
		plogic.playerSounds();
	}

	void PlayerClass::handleActions(double frametime)
	{
		// if the player has moved from his snapshot, we need to synch with renderer
		mtx.playermove.lock();
		{
			// check if the player moved enough to signal to networking to update movement
			// update networking if position changed by a reasonable amount
			const double net_change = 0.01;
			player.changedPosition = distance(snap_pos, pos) > net_change;
			snap_pos = pos;

			// true if the player is moving (on purpose)
			// used to modulate player camera giving the effect of movement
			JustMoved = (plogic.Moved == true) && (plogic.freefall == false || plogic.Ladderized);
			//logger << "JustMoved: " << JustMoved << Log::ENDL;

		}
		mtx.playermove.unlock();

		// get player skylight & torchlight
		// NOTE: we are using snap_pos for this, since its closest to the render position
		plogic.light = Spiders::getLightNow(snap_pos.x, snap_pos.y, snap_pos.z);

		// handle player selection, actions and building
		paction.handle(frametime);
	}

	void PlayerClass::handleRotation()
	{
		// measure closeness
		float dx = fabsf(player.rot.x - game.input().rotation().x);
		float dy = fabsf(player.rot.y - game.input().rotation().y);

		// rotate if too far apart (NOTE: possible bug with calculating angle distance)
		player.changedRotation = (dx > 0.0001 || dy > 0.0001);
		if (player.changedRotation)
		{
			// set new rotation, and update camera
			player.rot = game.input().rotation();
			camera.recalc  = true; // rebuild visibility set
			camera.rotated = true; // resend all rotation matrices
		}
	}

}
