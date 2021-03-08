#include "player_physics.hpp"

#include <library/log.hpp>
#include <library/math/toolbox.hpp>
#include "game.hpp"
#include "player.hpp"
#include "player_logic.hpp"
#include <cmath>

using namespace library;

namespace cppcraft
{
	PlayerPhysics playerPhysics;

	#define OSF    0.5  // overall speed factor

	// PlayerPhysics class
	const double PlayerPhysics::PLAYER_SIZE = 0.2;

	const double PlayerPhysics::bump_requirement  = 0.2     * OSF; // required momentum for bump
	const double PlayerPhysics::bump_reflect_treshold = 0.3 * OSF; // if abs(pax) > treshold
	const double PlayerPhysics::bump_reflection = 0.35      * OSF; //   pax = -pax * factor
	const double PlayerPhysics::bump_friction   = 0.9       * OSF; // else pax *= friction
	const double PlayerPhysics::bump_jumpiness  = 0.1       * OSF; // trampoline jump increase
	const double PlayerPhysics::bump_maxjump    = 1.6       * OSF; // trampoline max player-made bounce speed

	const double PlayerPhysics::movement_determinator = 0.050 * OSF; // smallest speed that animates movement
	const double PlayerPhysics::crouch_determinator   = 0.025 * OSF; // smallest crouching speed that animates movement

	const double PlayerPhysics::spdForward = 0.15 * OSF;
	const double PlayerPhysics::spdCrouch  = 0.08 * OSF;
	const double PlayerPhysics::spdSprint  = 0.30 * OSF;

	const double PlayerPhysics::spdLadder       = 0.08 * OSF;
	const double PlayerPhysics::spdLadderSprint = 0.12 * OSF;

	const double PlayerPhysics::spdJump     = 0.35   * OSF;
	const double PlayerPhysics::spdAutoJump = 0.20   * OSF;
	const double PlayerPhysics::spdFluid    = 0.15   * OSF;
	const double PlayerPhysics::spdJumpFluid = 0.12  * OSF;
	const double PlayerPhysics::gravity     = 0.02   * OSF;
	const double PlayerPhysics::gravity_max = 4.0    * OSF;
	const double PlayerPhysics::gravity_fluid = 0.06 * OSF;

	const double PlayerPhysics::spdFlying = 4.0   * OSF;
	const double PlayerPhysics::spdUpDown = 1.0   * OSF;

	const double PlayerPhysics::spdJetpack      = 0.50 * OSF;
	const double PlayerPhysics::spdJetpackBurst = 1.30 * OSF;
	const float  PlayerPhysics::JETPACK_POWER   = 0.05;

	const double PlayerPhysics::splash_min_fall = 0.18;
	const double PlayerPhysics::splash_big_fall = 0.55;
	const double PlayerPhysics::INT_SoundWalk     = 0.35;
	const double PlayerPhysics::INT_SoundCrouch   = 0.5;
	const double PlayerPhysics::INT_SoundSprint   = 0.25;
	const double PlayerPhysics::INT_WalkSoundBias = 0.01;

	PlayerPhysics::PlayerPhysics()
	{
		this->curspeed = 0;
	}

	void PlayerPhysics::handleMomentum()
	{
		// ---- in-game ---- //
		//////////////////////////////////////////////////////////
		// Only go here if the player can control his character //
		//////////////////////////////////////////////////////////

		movestate_t movestate = plogic.movestate;

		if (player.Flying)
		{
			movestate = PMS_Normal;
		}
		else if (plogic.Ladderized)
		{
			// change back to normal state if on a ladder
			if (movestate == PMS_Crouch)
			{
				movestate = PMS_Normal;
			}
		}
		else
		{
			if (game.input().key(keyconf.k_crouch) || keyconf.jbuttons[keyconf.joy_btn_crouch])
			{
				// as long as currently not in freefall
				if (plogic.freefall == false)
				{
					// we can crouch only if we are moving normally to begin with
					if (movestate == PMS_Normal)
					{
						//cooldownTime = actionTime;
						movestate = PMS_Crouch;
					}
				}
			}
			else if (movestate == PMS_Crouch)
			{
				movestate = PMS_Normal;
			}
		}

		if (plogic.Submerged == false && player.Flying == false)
		{
			// allowed: jetpacking, jumping, ladderwalk (probably not slowfall)
			if (movestate != PMS_Crouch)
			{
				if (game.input().key(keyconf.k_sprint) || keyconf.jbuttons[keyconf.joy_btn_sprint])
				{
					movestate = PMS_Sprint;
				}
				else if (movestate == PMS_Sprint)
				{
					movestate = PMS_Normal;
				}
			}
		}

		///  set player movestate once  ///
		plogic.movestate = movestate;

		const double SPRINT_ERPOLATION = 0.05;

		double intrpol = 0.1;
		double invintr = 1.0 - intrpol;

		if (player.Flying)
		{
			if (game.input().key(keyconf.k_sprint) || keyconf.jbuttons[keyconf.joy_btn_sprint])
			{
				this->curspeed = (this->curspeed * intrpol + PlayerPhysics::spdFlying * invintr);
			}
			else
			{
				this->curspeed = (this->curspeed * intrpol + PlayerPhysics::spdFlying * 0.25 * invintr);
			}
		}
		else
		{
			if (plogic.Submerged == true)
			{
				this->curspeed = (this->curspeed * intrpol + PlayerPhysics::spdFluid * invintr);
			}
	#ifdef USE_JETPACK
			else if (plogic.jetpacking == true)
			{
				if (plogic.movestate == movestate_t.PMS_Sprint)
				{
					this->curspeed = (this->curspeed * intrpol + PlayerPhysics::spdJetpackBurst * invintr);
				}
				else
				{
					this->curspeed = (this->curspeed * intrpol + PlayerPhysics::spdJetpack * invintr);
				}
			}
	#endif
			else if (plogic.movestate == PMS_Crouch)
			{
				this->curspeed = (this->curspeed * intrpol + PlayerPhysics::spdCrouch * invintr);
			}
			else if (plogic.movestate == PMS_Sprint)
			{
				intrpol = SPRINT_ERPOLATION;
				invintr = 1.0 - intrpol;

				this->curspeed = (this->curspeed * intrpol + PlayerPhysics::spdSprint * invintr);
			}
			else
			{
				this->curspeed = (this->curspeed * intrpol + PlayerPhysics::spdForward * invintr);
			}
		}

		// halve the speed if slowed
		if (plogic.Slowfall) this->curspeed *= 0.5;

		// player movement vectors
		plogic.Motion = 0;
		float dx = 0.0;
		float dz = 0.0;

		if (player.busyControls() == false)
		{
			/// JOYSTICK MOVEMENT
			// left/right
			if (std::abs(keyconf.jaxis[keyconf.joy_axis_sidestep]) > keyconf.joy_deadzone)
			{
				float vz = tresholdValue(keyconf.jaxis[keyconf.joy_axis_sidestep]);

				dx += cosf(player.rot.y) * vz;
				dz += sinf(player.rot.y) * vz;
			}
			// forward/backward
			if (std::abs(keyconf.jaxis[keyconf.joy_axis_forward]) > keyconf.joy_deadzone)
			{
				float vz = tresholdValue(keyconf.jaxis[keyconf.joy_axis_forward]);

				dx -= sinf(player.rot.y) * vz;
				dz += cosf(player.rot.y) * vz;
				plogic.Motion = (vz < 0.0) ? 1 : 2;
			}

			/// KEYBOARD MOVEMENT
			if (game.input().key(keyconf.k_left))   // left
			{
				dx -= cosf(player.rot.y);
				dz -= sinf(player.rot.y);
			}
			if (game.input().key(keyconf.k_right))  // right
			{
				dx += cosf(player.rot.y);
				dz += sinf(player.rot.y);
			}
			if (game.input().key(keyconf.k_forward)) // forward
			{
				dx += sinf(player.rot.y);
				dz -= cosf(player.rot.y);
				plogic.Motion = 1;
			}
			if (game.input().key(keyconf.k_backward)) // backward
			{
				dx -= sinf(player.rot.y);
				dz += cosf(player.rot.y);
				plogic.Motion = 2;
			}

			float length = std::sqrt(dx*dx + dz*dz);
			if (length > 0.000001)
			{
				dx *= this->curspeed / length;
				dz *= this->curspeed / length;
			}
		}

		// preserve momentum, build player acceleration

		if (plogic.Ladderized || plogic.Slowfall)
		{
			// half-decent momentum change
			player.accel.x = mix(player.accel.x, dx, 0.125);
			player.accel.z = mix(player.accel.z, dz, 0.125);
		}
		else if (plogic.freefall == true || player.Flying == true)
		{
			if (plogic.Submerged)
			{
				// swimming power
				player.accel.x = mix(player.accel.x, dx, 0.0125);
				player.accel.z = mix(player.accel.z, dz, 0.0125);
			}
			else if (player.Flying)
			{
				// flying momentum
				player.accel.x = mix(player.accel.x, dx, 0.03);
				player.accel.z = mix(player.accel.z, dz, 0.03);
			}
	#ifdef USE_JETPACK
			else if (plogic.jetpacking)
			{
				// jetpack power
				player.accel.x = mix(player.accel.x, dx, 0.02);
				player.accel.z = mix(player.accel.z, dz, 0.02);
			}
	#endif
			else
			{
				// air momentum change, REALLY low
				player.accel.x = mix(player.accel.x, dx, 0.011);
				player.accel.z = mix(player.accel.z, dz, 0.011);
			}
		}
		else
		{
			// normal momentum, except on ground with varying friction

			// player block can affect momentum
			if (plogic.block->isLowFriction())
			{
				// weakest momentum change
				player.accel.x = mix(player.accel.x, dx, 0.01);
				player.accel.z = mix(player.accel.z, dz, 0.01);
			}
			else
			{
				// all other blocks, including air
				player.accel.x = mix(player.accel.x, dx, 0.125);
				player.accel.z = mix(player.accel.z, dz, 0.125);
			}

		}

	} // handleMomentum()

}
