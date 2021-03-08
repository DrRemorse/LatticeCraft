#include "player_actions.hpp"

#include "game.hpp"
#include "particles.hpp"
#include "player.hpp"
#include "player_logic.hpp"
#include "sectors.hpp"
#include "soundman.hpp"
#include "spiders.hpp"
#include <glm/glm.hpp>
#include <cmath>

/**
 * In this module we ACT on the actions that the player is taking
 * Most actions rely on the players selected block in the world
 * Other actions are specific only to the item held by the player
 *
 * With no action being performed we instead try to select a block in the world
 *
**/

namespace cppcraft
{
	// activates the block in the world selected by the player, if applicable
	// held_item is the item the player is currently holding
	void PlayerActions::activate(Item& held_item)
	{
		// with no selection in the world, we can't activate anything
		if (plogic.hasSelection() == false) return;
		playerselect_t& selection = plogic.selection;

		// local grid coordinates
		int ddx = int(selection.pos.x);
		int ddy = int(selection.pos.y);
		int ddz = int(selection.pos.z);

		Block& selected = Spiders::getBlock(ddx, ddy, ddz);
		(void) held_item;
		(void) selected;
		/*
		if (isDoor(id))
		{
			block_t newState = (selectedBlock.getExtra() >> 1) xor 1;

			// modulate bitfield
			selectedBlock.setExtra( (selectedBlock.getExtra() & 1) + (newState << 1) );

			// collect into 6 bits
			block_t bitfield = selectedBlock.getFacing() + (selectedBlock.getExtra() << 2);

			Spiders::updateBlock(ddx, ddy, ddz, bitfield);

			//NetworkBlock nblock(ddx, ddy, ddz, selectedBlock, NetworkBlock::BSET);
			//network.addBlock(Network::OUTGOING, nblock);

			const Block::bfield_t door_ul_bit = 1 << 2;

			if (selectedBlock.getExtra() & 1) // lower
			{
				// update upper also
				Spiders::updateBlock(ddx, ddy+1, ddz, bitfield - door_ul_bit);

				// send network update
				//NetworkBlock nblock(ddx, ddy+1, ddz, Block(id, bitfield - door_ul_bit), NetworkBlock::BSET);
				//network.addBlock(Network::OUTGOING, nblock);

			}
			else
			{
				// update lower also
				Spiders::updateBlock(ddx, ddy-1, ddz, bitfield + door_ul_bit);

				// send network update
				//NetworkBlock nblock(ddx, ddy-1, ddz, Block(id, bitfield + door_ul_bit), NetworkBlock::BSET);
				//network.addBlock(Network::OUTGOING, nblock);

			}

			#ifdef USE_SOUND
				if newstate = 0 then
					inetSoundEx(S_FX_DOOR_OPEN, 1.0, true)
				else
					inetSoundEx(S_FX_DOOR_CLOSE, 1.0, true)
				endif
			#endif
		}*/

	}

	// performs any item action that the item being passed to the function can perform
	void PlayerActions::itemAction(Item& item)
	{
    // items you can shoot with
		if (false)
		{
			int FIXME_player_shoot;
			//PlayerShoot(actionitem);
		}
		else
		{
			// items you can't build with, but you can swing with
			swingTool(item);
		}
	}

	// builds using the players selected (held) item and try to place it
	// at the correct position relative to the players selected block (in the world)
	void PlayerActions::build(Item& item)
	{
    // must be non-zero count on item
    if (item.getCount() == 0) return;
    // the item must be either a block or an item that builds
    Block buildingBlock;
    if (item.isItem()) {
      if (item.itemdb().getActivationMode() == db::ItemData::ACT_BUILD) {
        block_t id = db::getb(item.itemdb().getActivationResult());
        buildingBlock = Block(id);
      }
      else {
        // don't accept any other activation modes
        return;
      }
    }
    else {
      buildingBlock = item.toBlock();
    }

		// add a block to the opposite of the selected face
		playerselect_t& selection = plogic.selection;

		// the cube facing that would face player
		block_t facing = player.getBlockFacing();

		// additional test for whether the destination face is correct for this block
		// eg. cant place ladders on top and bottom faces, torches on bottom faces etc.
		glm::vec3 fpos = glm::fract(selection.pos);
		bool placement_test = item.toBlock().placeToFace(selection.facing, fpos.x, fpos.y, fpos.z);

		// make sure we can place <here>
		int ddx = selection.pos.x;
		int ddy = selection.pos.y;
		int ddz = selection.pos.z;
		Block& selectedBlock = Spiders::getBlock(ddx, ddy, ddz);

		// now that we know we are allowed to place our block with the specificed facing,
		// we have to check if we are similarly allowed to place something onto the same facing
		// of the destination block (ddx, ddy, ddz)
		placement_test &= selectedBlock.placeOntoThis(facing);

		if (placement_test)
		{
			// move to where we are to place the new block
			switch (selection.facing)
			{
				case 0: ddz += 1; break; // +z
				case 1: ddz -= 1; break; // -z
				case 2: ddy += 1; break; // +y
				case 3: ddy -= 1; break; // -y
				case 4: ddx += 1; break; // +x
				case 5: ddx -= 1; break; // -x
			}

			// check if we are allowed to place a block in the selected position
			Block& newBlock = Spiders::getBlock(ddx, ddy, ddz);
			if (newBlock.triviallyOverwriteable())
			{
				// add block to world
				bool placed = Spiders::setBlock(ddx, ddy, ddz, buildingBlock);
				if (placed)
				{
					//item.setCount(item.getCount() - 1); //decrease count (directly)!
					//inventory.setChanged(true);
					printf("Placing item %d with block ID %d\n",
							item.getID(), item.toBlock().getID());

					// play placement sound
					if (item.toBlock().hasSound())
						soundman.playMaterial(item.toBlock().getSound(),
                                  Soundman::sound_place,
                                  {ddx, ddy, ddz});
					else
						printf("Missing placement sound for %s\n", item.toBlock().getName().c_str());

					// send update to network
					//NetworkBlock nblock(ddx, ddy, ddz, Block(id, bfield), NetworkBlock::BADD);
					//network.addBlock(Network::OUTGOING, nblock);

          // TODO: do something more meaningful, like setViewDirty();
          plogic.Moved = true;
				}
			}

		} // placement tests

	} // build

	// checks which action the player is performing (if any) and execute on it
	// if no action is being performed, try to select a block (in the world) instead
	void PlayerActions::handleActions(double frametime)
	{
		//block_t facing  = player.getBlockFacing();
		glm::vec3 lookVector = player.getLookVector();

		playerselect_t& selection =  plogic.selection;

		/////////////////////////////////////
		/// set up raycaster / integrator ///
		/////////////////////////////////////
		#ifdef DBG_SUPERFOCAL
			const double action_range = 16;
		#else
			const double action_range = 5;
		#endif
		const float action_step  = 0.005;
		const float action_bigstep = 0.2;

		glm::vec3 rayBigStep = lookVector * action_bigstep;
		glm::vec3 rayStep    = lookVector * action_step;

		glm::vec3 playerPos = player.pos;
		// account for crouching
		if (plogic.movestate == PMS_Crouch) playerPos.y -= 0.25;

		/////////////////////////////////////
		/// item currently held by player ///
		/////////////////////////////////////
		Item& helditem = game.gui().hotbar_item();

		if (action == playeraction_t::PA_Addblock)
		{
			// we need to check if we can perform an action instead of "placing a block"
			// so to do that we see if the current item is an actionable item
			// we also have the ability to activate blocks in the world, if so that block hasActivation()
			if (false) //item.isActionItem())
			{
				itemAction(helditem);
			}
			else if (plogic.hasSelection())
			{
				if (selection.block.hasActivation())
				{
					activate(helditem);
					return;
				}
				else // otherwise just 'build'
				{	// aka place block
					build(helditem);
				}
			}

			// handle hand animation & cooldown timing
			if (action != PA_Nothing)
			{
				cooldown();
				return;
			}
		}
		else if (action == PA_Remblock)
		{
			// remove selected block
			if (plogic.hasSelection())
			{
				int ddx = int(selection.pos.x);
				int ddy = int(selection.pos.y);
				int ddz = int(selection.pos.z);

				// now, remove the block we wanted to remove to begin with
				Block removed = Spiders::removeBlock(ddx, ddy, ddz);

				if (!removed.isAir())
				{
          const glm::vec3 center(ddx + 0.5f, ddy + 0.5f, ddz + 0.5f);
					// play material 'removed' sound
					if (removed.hasSound())
            soundman.playMaterial(removed.getSound(),
                                  Soundman::sound_remove,
                                  center);
					else
						printf("Missing removal sound for %s\n", removed.getName().c_str());

					// send update to network
					//NetworkBlock nblock(ddx, ddy, ddz, Block(), NetworkBlock::BREM);
					//network.addBlock(Network::OUTGOING, nblock);

					// create particles using fractionals
					//particleSystem.newParticle(center, PARTICLE_M_GENER, 16);

				} // block was removed
        // TODO: do something more meaningful, like setViewDirty();
        plogic.Moved = true;

			} // hasSelection()

			// allow the player to continue mining more blocks
			action = PA_Mineblock; //dont more blocks!
			// set ridicolous numbers which will get changed immediately when the player gets a new selection
			mineTimer = 2000;
			mineMax   = 2000;
			minimizer = -2;
		}
		else if (action == PA_Swingtool)
		{
			actionTimer -= 1;
			if (actionTimer == 0)
			{
				cancelDig();
			}
		}
		else // selection
		{
			// here we are going to raycast our way to the closest block we are pointing at
			// if no block is found, we need to set selection.block to null, so that we know no block is selected

			// start at player position
			glm::vec3 ray = playerPos;
			bool foundSelection = false;

			// integrate forward
			for (float forward = 0.0; forward <= action_range; forward += action_bigstep)
			{
				// increase ray by one big step
				ray += rayBigStep;
				Block& found = Spiders::getBlock(ray.x, ray.y, ray.z);

				if (!found.isAir() && !found.isFluid())
				{
					// create fractionals from ray
					glm::vec3 fracs = glm::fract(ray);

					if (found.selectionHitbox3D(fracs.x, fracs.y, fracs.z))
					{
						// for loop to make sure we don't trace ourselves back behind the player
						for (float backward = forward; backward >= 0.0; backward -= action_step)
						{
							// glean backward slowly until we exit completely, but we still retain our closest valid position
							ray -= rayStep;
							Block& bfound = Spiders::getBlock(ray.x, ray.y, ray.z);

							fracs = glm::fract(ray);

							// glean until we hit air, then break
							if (bfound.selectionHitbox3D(fracs.x, fracs.y, fracs.z) == false) break;

						}
						// now that we are out, increase ray by one step ahead to get back inside
						ray += rayStep;
						// create new fractionals
						fracs = glm::fract(ray);

						const int ddx = ray.x, ddy = ray.y, ddz = ray.z;
						// find position in local grid
						Sector* sector = sectors.sectorAt(ddx, ddz);

						// outside of local grid? nothing to select
						if (sector == nullptr) break;

						const auto& selectionBlock = Spiders::getBlock(ddx, ddy, ddz);
						int selectionFace = plogic.determineSelectionFacing(selectionBlock, ray, fracs, action_step);

						// make crc of internal position
						const uint32_t CRC = (ddx * Sector::BLOCKS_XZ + ddz) * Sector::BLOCKS_XZ + ddy;
						{
              std::lock_guard<std::mutex> (plogic.selection_mtx());
              // set selection results
							selection.block = selectionBlock;
							selection.sector = sector;
							selection.pos = ray;
							selection.facing = selectionFace;
							selection.checksum = CRC;
							selection.updated  = true;
						}

						if (action == PA_Mineblock)
						{
							handleMining(frametime, helditem);
						}

						// break out of loop
						foundSelection = true;
						break;

					} // selection hitboxing

				} // air & fluid test

			} // ray integrator

			if (foundSelection == false)
			{
				if (selection.sector)
				{
					// unset our selection
          std::lock_guard<std::mutex> (plogic.selection_mtx());
					selection.sector = nullptr;
				}

				if (action == playeraction_t::PA_Mineblock)
				{
					// reset any mining process, when we lost selection
					// but not cancel it entirely, because mouse is still "held",
					// and we could start mining again soon(TM)
					mineTimer = 2000;
					mineMax   = 2000;
					// invalidate CRC value
					minimizer = -1;
				}
			}

		} // selection hitboxing
	}

	void PlayerActions::handleMining(double frametime, const Item& helditem)
	{
		if (frametime > actionTimer + MINE_SPEED)
		{
			actionTimer = frametime;

			playerselect_t& selection = plogic.selection;

			if (minimizer != selection.checksum)
			{
				minimizer = selection.checksum;

				// if we got here we have selected a new block to mine from
				mineTimer = 16; //helditem.itemdb().getMiningTime(helditem, selection.block);

				if (minimizer == -2)
				{
					// we have mined before
					mineTimer += 6; // extra wait time
				}

				// set the rest, and prepare for battle =)
				mineMax = mineTimer;
			}

			if (mineTimer > 0)
			{
				// propagate start of mining
				/*
				if (network.mymine = false)
				{
					networkSend("PMINE 1");
					network.mymine = true;
				}*/

				// create mining debris, and propagate over inet
				/*
				#Ifdef USE_OBJECTS
					dim itm as inventoryitem = TransformParticle(selectionid)
					itm.count = 2 * (plogic.MineMax - plogic.MineTimer) / plogic.MineMax
					if itm.count <> 0 And itm.id <> 0 then
						CreateObjectMining(s, ddx, ddy, ddz, selface, itm)
						networkSendObject(s, ddx, ddy, ddz, selface, @itm)
					endif
				#EndIf
				*/

				// create particles using fractionals
				// newParticleB(s, bx, by, bz, PARTICLE_M_GENER, 3)

				// play mining sound based on material
				if (mineTimer % MINE_SOUNDMOD == 0)
				{
					if (selection.block.hasSound())
						soundman.playMaterial(selection.block.getSound(),
                                  Soundman::sound_mine,
                                  selection.pos);
				}

				mineTimer -= 1;
			}
			else if (mineTimer == 0)
			{
				action = PA_Remblock;
			}
		}

	}

}
