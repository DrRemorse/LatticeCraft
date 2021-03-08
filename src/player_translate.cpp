#include "player_logic.hpp"

#include <library/log.hpp>
#include <library/math/toolbox.hpp>
#include "game.hpp"
#include "player.hpp"
#include "player_physics.hpp"
#include "sectors.hpp"
#include "spiders.hpp"
#include "soundman.hpp"
#include <cmath>

using namespace library;

namespace cppcraft
{
	void PlayerLogic::translatePlayer()
	{
		bool moved = false;
		bool jumpKey = (game.input().key(keyconf.k_jump) || keyconf.jbuttons[keyconf.joy_btn_jump]) && player.busyControls() == false;

		const double PLAYER_GROUND_LEVEL = 1.51;
		const double fw_feettest   = 1.45; // feet level, used for gravity and landing tests
		const double fw_downtest   = 1.20; // axx movement test downwards
		const double fw_headtest   = 0.11; // head-height
		const double fw_crouchtest = 1.58; // downwards test for crouch
		const double fw_autojump   = 2.20; // downwards test for auto-jump

		//----------------------//
		//   flying up / down   //
		//----------------------//

#ifdef DBG_FLYING
		if (player.busyControls() == false)
		{
			if (game.input().key(keyconf.k_flydown))
			{
				// fly faster when sprint key is held down
				if (game.input().key(keyconf.k_sprint))
					player.pos.y -= PlayerPhysics::spdUpDown;
				else
					player.pos.y -= PlayerPhysics::spdUpDown * 0.25;
				moved = true;
			}
			if (game.input().key(keyconf.k_flyup))
			{
				// fly faster when sprint key is held down
				if (game.input().key(keyconf.k_sprint))
					player.pos.y += PlayerPhysics::spdUpDown;
				else
					player.pos.y += PlayerPhysics::spdUpDown * 0.25;
				moved = true;
			}
		}
#endif

		//////////////////////////
		/// Integrated gravity ///
		//////////////////////////

		// player directional vector
		glm::vec3 lookVector = player.getLookVector();

		// if player is climbing a ladder
		if (Ladderized)
		{
			// move player up and down ladder
			if (Motion == 1) // forward motion
			{
				if (movestate == PMS_Sprint)
					player.accel.y = lookVector.y * PlayerPhysics::spdLadderSprint;
				else
					player.accel.y = lookVector.y * PlayerPhysics::spdLadder;
				moved = true;
			}
			else if (Motion == 2) // backwards motion
			{
				if (movestate == PMS_Sprint)
					player.accel.y = -lookVector.y * PlayerPhysics::spdLadderSprint;
				else
					player.accel.y = -lookVector.y * PlayerPhysics::spdLadder;
				moved = true;
			}
			else if (Motion == 3) // jumping
			{
				// check for a ladder at a certain height under player
				// if there is a ladder there, prevent jumping
				if (Spiders::testArea(player.pos.x, player.pos.y - 0.40 - player.accel.y, player.pos.z).isLadder())
				{
					// not sure about this
					player.accel.y = 0.0;
					freefall = false;
				}
			}	// not sure about this
			else player.accel.y = 0.0;

			// if player is looking strongly up/down
			if (fabs(lookVector.y) > 0.75)
			{
				// lock x/z when sprinting or holding crouch key
				if (movestate == PMS_Sprint ||
					game.input().key(keyconf.k_crouch) != 0 ||
					keyconf.jbuttons[3])
				{
					player.accel.x = 0.0;
					player.accel.z = 0.0;

				}	// slow x/z when moving, but not jumping
				else if (plogic.Motion != 3)
				{
					// 60% directional player control to ladders
					player.accel.x *= 0.25 + fabs(lookVector.x) * 0.75;
					player.accel.z *= 0.25 + fabs(lookVector.z) * 0.75;
				}
			}
		}
		else if (Submerged != PS_None)
		{
			//////////////////////////////
			////    WATER GRAVITY     ////
			//////////////////////////////

			// player water-gravity accumulation
			#define PLAYER_WATER_GRAVITY()  \
				player.accel.y = player.accel.y * 0.85 + -PlayerPhysics::gravity_fluid * 0.15;

			// when fully submerged, we can just re-enable escaping
			// so that the player can continue to try to escape water
			if (FullySubmerged) EscapeAttempt = true;

			if (jumpKey)
			{
				// if the jump key isnt held yet
				if (this->jumplock == false)
				{
					// check if player is allowed to escape water
					Block blk = Spiders::testAreaEx(player.pos.x,player.pos.y-1.6,player.pos.z);
					EscapeAttempt = blk.blocksMovement();
					// if the player is currently escaping, start "holding" this key, preventing bouncing
					if (EscapeAttempt)
					{
						// hold it
						this->jumplock = true;
					}
				}

				if (Spiders::testArea(player.pos.x,player.pos.y-0.45,player.pos.z).isAir())
				{
					// apply slight downward gravity when not too far out of water
					// basically, as long as not able to escape water
					if (plogic.EscapeAttempt == false)
						player.accel.y = -0.05;
				}
				else if (Spiders::testArea(player.pos.x, player.pos.y-0.35, player.pos.z).isAir())
				{
					// really far out of water, but able to rest on waves (i think)
					if (EscapeAttempt == false)
						player.accel.y = 0.0;
				}
				else
				{
					PLAYER_WATER_GRAVITY()
				}
			}
			else
			{
				PLAYER_WATER_GRAVITY()
			}
		}
		else if (plogic.freefall)
		{
			/// adding normal gravity ///
			player.accel.y -= PlayerPhysics::gravity;

			// clamp to max gravity
			if (player.accel.y < -PlayerPhysics::gravity_max)
				player.accel.y = -PlayerPhysics::gravity_max;
		}
		// slow-falling reduces ANY y-force
		if (Slowfall)
		{
			player.accel.y *= 0.5;
		}

		//
		Block s[4];

		// when player is determined to be falling, do the fallowing:
		if (plogic.freefall && player.accel.y != 0)
		{
			// setup incremental gravity test
			const double integratorStepsize = 0.125;
			int integratorSteps = fabs(player.accel.y) / integratorStepsize;
			if (integratorSteps < 2) integratorSteps = 2;

			int moregravity = integratorSteps;
			double grav     = player.accel.y / integratorSteps;

			const double PLAYER_GRAVITY_REDUCTION_FACTOR = 0.6;

			// gravity integrator
			while (moregravity > 0 && grav != 0.0)
			{
				if (grav < 0)
					s[0] = Spiders::testAreaEx(player.pos.x, player.pos.y-fw_feettest+grav, player.pos.z);
				else
					s[0] = Spiders::testAreaEx(player.pos.x, player.pos.y+fw_headtest+grav, player.pos.z);

				if (s[0].isAir())
				{
					/// move player on Y-axis ///
					player.pos.y += grav;
					// causes timedMotion to increase just after landing, because moved = true and freefall == false
					//if (fabs(grav) > PlayerPhysics::movement_determinator) moved = true;
				}
				else
				{
					// check if theres a trampoline there somewhere
					// then, check minimum bounce gravity requirement
					/*if (s[0] == _TRAMPOLINE)
					{
						if (fabs(player.accel.y) > PlayerPhysics::bump_requirement)
						{
							#ifdef USE_SOUND
								// play bounce sound
								sndPlayCustom(S_FX_BOUNCE);
							#endif
							// reverse player Y-force (and stop gravitizing)
							player.accel.y = -player.accel.y;
							// extra bounce power under 1.6 Y-force when player is holding jump key
							if (fabs(player.accel.y) < PlayerPhysics::bump_maxjump && jumpKey)
							{
								player.accel.y += player.accel.y * PlayerPhysics::bump_jumpiness;
							}
							// exit loop
							break;
						}
						else
						{
							player.accel.y *= PLAYER_GRAVITY_REDUCTION_FACTOR;
							grav *= PLAYER_GRAVITY_REDUCTION_FACTOR;
						}
					}
					else*/ if (Ladderized == false)
					{
						// deceleration of gravity when not in-air
						player.accel.y *= PLAYER_GRAVITY_REDUCTION_FACTOR;
						grav *= PLAYER_GRAVITY_REDUCTION_FACTOR;
					}
				}

				// redo gravity as long as moregravity > 0
				moregravity -= 1;
			}
			// while (moregravity > 0)

		} // freefall

		const double MINIMUM_PLAYER_ACCELERATION_SPEED = 0.002;

		/// player acceleration X ///

		if (fabs(player.accel.x) > MINIMUM_PLAYER_ACCELERATION_SPEED)
		{
			float dx = player.pos.x + player.accel.x;

			// regular movement test
			s[0] = Spiders::testAreaEx(dx, player.pos.y - fw_downtest, player.pos.z);
			s[1] = Spiders::testAreaEx(dx, player.pos.y - 0.45, player.pos.z);
			s[2] = Spiders::testAreaEx(dx, player.pos.y + 0.00, player.pos.z);
			s[3] = Spiders::testAreaEx(dx, player.pos.y + fw_headtest,  player.pos.z);

			// will move upwards
			if (s[0].isAir() && s[1].isAir() && s[2].isAir() && s[3].isAir())
			{
				// not crouching, or crouching AND free-falling
				if (movestate != PMS_Crouch || freefall == true)
				{
					player.pos.x = dx;
					// the player moved if the movement itself was bigger than the minimum
					if (fabs(player.accel.x) >= PlayerPhysics::movement_determinator) moved = true;
				}
				else  // crouching
				{
					Block blk = Spiders::testAreaEx(dx, player.pos.y-fw_crouchtest, player.pos.z);
					if (!blk.isAir() && !blk.isHalfblock())
					{
						player.pos.x = dx;
						if (fabs(player.accel.x) >= PlayerPhysics::crouch_determinator) moved = true;
					}
				}

				if (freefall == false && player.Flying == false && moved)
				{
					if (Spiders::testAreaEx(player.pos.x, player.pos.y - fw_autojump, player.pos.z).isAir())
					{
						// ground level must also be air
						if (Spiders::testAreaEx(player.pos.x, player.pos.y - PLAYER_GROUND_LEVEL, player.pos.z).isAir())
						//---------------------------//
						//         AUTO JUMP		 //
						player.accel.y = PlayerPhysics::spdAutoJump;
						//---------------------------//
					}
				}
			}
			else
			{
				// not moving into air, so do trampoline tests
				/*if (fabs(player.accel.x) > PlayerPhysics::bump_requirement &&
					(s[0] == _TRAMPOLINE || s[1] == _TRAMPOLINE || s[2] == _TRAMPOLINE))
				{
					#ifdef USE_SOUND
						// play bounce sound
						sndPlayCustom(S_FX_BOUNCE)
					#endif
					// reverse player X-force
					player.accel.x = -player.accel.x;
				}
				else*/ if (fabs(player.accel.x) > PlayerPhysics::bump_reflect_treshold)
				{
					player.accel.x = -player.accel.x * PlayerPhysics::bump_reflection;
				}
				else player.accel.x *= PlayerPhysics::bump_friction;
			}
		} // player.accel.x

		/// player acceleration Z ///

		if (fabs(player.accel.z) > MINIMUM_PLAYER_ACCELERATION_SPEED)
		{
			float dz = player.pos.z + player.accel.z;

			// regular movement test
			s[0] = Spiders::testAreaEx(player.pos.x, player.pos.y - fw_downtest, dz);
			s[1] = Spiders::testAreaEx(player.pos.x, player.pos.y - 0.45, dz);
			s[2] = Spiders::testAreaEx(player.pos.x, player.pos.y + 0.00, dz);
			s[3] = Spiders::testAreaEx(player.pos.x, player.pos.y + fw_headtest,  dz);

			if (s[0].isAir() && s[1].isAir() && s[2].isAir() && s[3].isAir())
			{
				// player is not crouching, or crouching and free-falling
				if (movestate != PMS_Crouch || freefall == true)
				{
					player.pos.z = dz;
					if (fabs(player.accel.z) >= PlayerPhysics::movement_determinator) moved = true;
				}
				else  // crouching
				{
					Block blk = Spiders::testAreaEx(player.pos.x, player.pos.y - fw_crouchtest, dz);
					if (!blk.isAir() && !blk.isHalfblock())
					{
						player.pos.z = dz;
						if (fabs(player.accel.z) >= PlayerPhysics::crouch_determinator) moved = true;
					}
				}

				if (freefall == false && player.Flying == false && moved)
				{
					if (Spiders::testAreaEx(player.pos.x, player.pos.y - fw_autojump, player.pos.z).isAir())
					{
						// ground level must also be air
						if (Spiders::testAreaEx(player.pos.x, player.pos.y - PLAYER_GROUND_LEVEL, player.pos.z).isAir())
						//---------------------------//
						//         AUTO JUMP		 //
						player.accel.y = PlayerPhysics::spdAutoJump;
						//---------------------------//
					}
				}
			}
			else
			{
				// player was not moving into air, so do trampoline tests
				/*if (fabs(player.accel.z) > PlayerPhysics::bump_requirement &&
					(s[0] == _TRAMPOLINE || s[1] == _TRAMPOLINE || s[2] == _TRAMPOLINE))
				{
					#ifdef USE_SOUND
						// play bounce sound
						sndPlayCustom(S_FX_BOUNCE)
					#endif
					// reverse player X-force
					player.accel.z = -player.accel.z;
				}
				else*/ if (fabs(player.accel.z) > PlayerPhysics::bump_reflect_treshold)
				{
					player.accel.z = -player.accel.z * PlayerPhysics::bump_reflection;
				}
				else player.accel.z *= PlayerPhysics::bump_friction;
			}
		} // player.accel.z

		/// set player moved variable ONCE ///
		plogic.Moved = moved;

		/// set player sector ///
		plogic.sector = sectors.sectorAt(player.pos.x, player.pos.z);
		/// set player terrain id ///
    plogic.detect_terrain();

		// if the player is flying, set certain flags and just exit
		if (player.Flying)
		{
			Slowfall   = false;
			Ladderized = false;
			Submerged  = PS_None;
			FullySubmerged = PS_None;
			block = &air_block; // in-air

			freefall = false; // never falling
			player.accel.y = 0.0; // and, zero gravity!
			return; // exit because flying is on
		} // flying

		// when not flying, make sure player is never below 0.0 on Y-axis
		if (player.pos.y < 0.0)
		{
			player.pos.y = 0.0;
			if (player.accel.y < 0.0) player.accel.y = 0.0;
		}

		const int maxtries = 10;
		int tries = 0;

		std::function<bool(const Block&)> gravityTest =
		[] (const Block& b)
		{
			return b.isAir() || b.isFluid() || b.isCross() || b.isLadder() || b.isSlowing();
		};

		while (true) // raytrace correct player Y-position
		{
			s[0] = Spiders::testArea(player.pos.x,player.pos.y-fw_feettest,player.pos.z);
			s[1] = Spiders::testArea(player.pos.x,player.pos.y-0.50,player.pos.z);
			s[2] = Spiders::testArea(player.pos.x,player.pos.y+0.00,player.pos.z);

			bool gravtest =
				gravityTest(s[0]) ||
				gravityTest(s[1]) ||
				gravityTest(s[2]);

			// break immediately if the player is not determined to be stuck
			if (gravtest)
			{
				// player is ladderized if ANY of the results show a ladder
				Ladderized = (s[0].isLadder() || s[1].isLadder() || s[2].isLadder());
				// player is slowfalling is ANY of the results show a vine
				Slowfall   = (s[0].isSlowing() || s[1].isSlowing() || s[2].isSlowing());

				bool psbb = Submerged;
				//bool psb  = FullySubmerged;

				// set fully submerged status, and submerged along with it
				if (s[2].isFluid())
				{
					FullySubmerged = Submerged = PS_Water;
				}
				/*else if (s[2] == _LAVABLOCK)
				{
					FullySubmerged = Submerged = PS_Lava;
				}*/
				else
				{
					FullySubmerged = PS_None;
					// see if the player is "otherwise" submerged
					if (s[0].isFluid() || s[1].isFluid())
					{	// in a water block at some level
						Submerged = PS_Water;
					}
					/*else if (s[0] == _LAVABLOCK || s[1] == _LAVABLOCK)
					{	// in a lava block at some level
						Submerged = PS_Lava;
					}*/
					else Submerged = PS_None;
				}

				// if the player was originally above water, but just entered...
				if (psbb == PS_None && Submerged != PS_None)
				{
					// play splash sound if the player had enough force
					//#ifdef USE_SOUND
						if (player.accel.y <= -PlayerPhysics::splash_big_fall)
						{
							soundman.playSound("splash_big"); // big splash
						}
						else if (player.accel.y <= -PlayerPhysics::splash_min_fall)
						{
							soundman.playSound("splash"); // small splash
						}
					//#endif
				}
				else if (psbb == true && Submerged == PS_None)
				{
					// the player just left the water, reset escape
					EscapeAttempt = false;
				}

				// player was not stuck, exit
				break;
			}

			//logger << "fixing player: " << player.pos.y << " -> " << player.pos.y + 0.02 << Log::ENDL;

			// move player a little bit upwards
			player.pos.y += 0.02;

			// increase number of tries
			tries += 1;
			// exit if too many tries
			if (tries == maxtries) break;
		}

		// if any raycasting tries have been expended, the player must have been moved
		// in which case, he could be moved again at next interval, and so we shouldn't move further
		if (tries) return;

		// determine player block (block that the player is standing on
		block = &Spiders::getBlock(player.pos.x, player.pos.y - PLAYER_GROUND_LEVEL, player.pos.z, PlayerPhysics::PLAYER_SIZE);

		/// manage falling player //
		// determing if player is falling, then set appropriate flags (DONT USE getBlock!)
		// note that the player is "falling" also when gravitating up
		Block fallTest = Spiders::testAreaEx(player.pos.x, player.pos.y - PLAYER_GROUND_LEVEL, player.pos.z);
		if (fallTest.isAir() || player.accel.y > 0)
		{
			plogic.freefall = true;
		}
		else if (plogic.Submerged != PS_None)
		{
			EscapeAttempt = false;
			freefall = true;
			player.accel.y = 0;
			// special case for landing in water
			// we can release the jump key (but only if its locked)
			this->jumplock = false;
		}
		else if (freefall)
		{
			freefall = false;
			// when ending falling, disable gravitation
			player.accel.y = 0;

			// play a landing sound
			if (block->hasSound())
			{
				int sound = rnd(4);
				soundman.playMaterial(block->getSound(), sound,
            {player.pos.x, player.pos.y - PLAYER_GROUND_LEVEL, player.pos.z});
			}

		} // freefalling

	} // translatePlayer()

	void PlayerLogic::handlePlayerJumping()
	{
		bool jumpKey = (game.input().key(keyconf.k_jump) || keyconf.jbuttons[keyconf.joy_btn_jump]) && player.busyControls() == false;

		if (jumpKey)
		{
			// it's disallowed to try to continue to jump when the eye-height + 0.10 is non-air
			Block blk = Spiders::testAreaEx(player.pos.x,player.pos.y+0.10,player.pos.z);
			if (blk.isFluid() || blk.isCross() || blk.isAir())
			{
				// if submerged in something, but not on a ladder
				if (Submerged != PS_None && Ladderized == false)
				{
					// swim upwards, or escape water
					player.accel.y = PlayerPhysics::spdJumpFluid;
					freefall = true;
				}
				else if (freefall == false || Ladderized == true)
				{
					Motion = 3;
					freefall = true;
					player.accel.y = PlayerPhysics::spdJump;

					#ifdef USE_JETPACK
						Jetpacking = false;
					#endif

					this->jumplock = true; // enable jump trigger
				}
				// if the player is free-falling, we can check if he wants to jetpack
				else if (freefall == true)
				{
					// jump key pressed, but not held
					if (jumpKey)
					{
						#ifdef USE_JETPACK
						if (jetpackFuel != 0)
						{
							Jetpacking = true;
							// accelerate up
							player.accel.y += PlayerPhysics::JETPACK_POWER;

							// smoke particles
							for (int i = 0; i < 4; i++)
							{
								int p = newParticle(player.pos.x, player.pos.y, player.pos.z, PARTICLE_SMOKE);
								if (p < 0) break;
								particleSpeed(p, -player.accel.x, 0.0, -player.accel.z);
							}

							// play sound if its time
							if (lastjet == 0)
							{
								soundman.playSound(Soundman::SND_JETPACK);
								lastjet = JETPACK_SOUNDWAIT;
							}
							else lastjet -= 1;

							// decrease fuel level
							jetpackFuel -= 1;
						}
						else
						{
							// stop jetpacking
							Jetpacking = false;
						}
						#endif

					} // jetpacking

				} // player is free-falling

			} // jump test
		}
		else // not trying to jump -->
		{
			// no longer able to escape water
			EscapeAttempt = false;
			// no longer jumplocked
			this->jumplock = false;

			#ifdef USE_JETPACK
				// jetpack replenish
				if (jetpackFuel < MAX_JETPACK) jetpack += 1;
				if (lastjet) lastjet -= 1;
				jetpacking = false;
			#endif
		}

	} // handlePlayerJumping()

}
