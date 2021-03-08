/**
 * Seamless transition function
 *
 * Moves the player back exactly one sector, while adding a new wall of sectors
 * ahead of him, swapping it with the wall behind him
 *
**/

#include "seamless.hpp"

#include "columns.hpp"
#include "camera.hpp"
#include "generator.hpp"
#include "minimap.hpp"
#include "player.hpp"
#include "precompq.hpp"
#include "sectors.hpp"
#include "threading.hpp"
#include "world.hpp"
#include <library/timing/timer.hpp>

using namespace library;
//#define TIMING

namespace cppcraft
{
	// length player has to travel for a transition to take place
	const int Seamless::OFFSET = Sector::BLOCKS_XZ;
  // called each time a transition happens
  static std::vector<delegate<void()>> transition_signal;
  // the edge sector row which gets mesh updated on transition
  static const int EDGE_NO = 2;

	// runs seamless() until no more transitions happen
	// essentially moving the player until he is on a local grid, near center
	bool Seamless::run()
	{
#ifdef TIMING
    Timer timer;
#endif
		// only run seamless if the player actually moved, one way or another
		// -=- COULD DO STRANGE STUFF -=-
		bool seam = false;
		while (true)
		{
			// transition world
			bool running = Seamless::seamlessness();
			seam |= running;
			if (running == false) break;
		}
    // call event handlers when transition happened
    if (seam) {
      for (auto& func : transition_signal) func();
#ifdef TIMING
      printf("Seamless::run() took %f seconds\n", timer.getTime());
#endif
    }
		return seam;
	}

  void Seamless::on_transition(delegate<void()> func) {
    transition_signal.push_back(std::move(func));
  }

	class Seamstress
	{
	public:
		static void resetSectorColumn(Sector& sector)
  	{
  		// we have to load new block content
  		sector.gen_flags = 0;
  		// add to generator queue
  		Generator::add(sector);
  	}
		static void updateSectorColumn(int x, int z)
    {
  		Sector& sector = sectors(x, z);

  		// if the sector was generated, we will regenerate mesh
  		if (sector.generated() && sector.isUpdatingMesh() == false)
          precompq.add(sector);

  	} // updateSectorColumn
	};

	// big huge monster function
	bool Seamless::seamlessness()
	{
		int halfworld = sectors.getXZ() * Sector::BLOCKS_XZ / 2;
		bool returnvalue = false;

		// if player is beyond negative seam offset point on x axis
		if (player.pos.x <= halfworld - Seamless::OFFSET)
		{
			mtx.sectorseam.lock();

			// move player forward one sector (in blocks)
			player.pos.x += Sector::BLOCKS_XZ;
			player.snap_pos.x += Sector::BLOCKS_XZ;
			// offset world x by -1
			world.worldCoords.x -= 1;
			world.increaseDelta(-1, 0);

			// only 25% left on the negative side
			for (int z = 0; z < sectors.getXZ(); z++)
			{
				// remember old sector, at the end of x-axis
				auto oldpointer = sectors.extract(sectors.getXZ()-1, z);

				// move forward on the x-axis
				for (int x = sectors.getXZ() - 1; x >= 1; x--)
				{
					// move sector columns on x
					sectors.replace(x,z, x-1,z);
				}

				// set first column on x-axis to old pointer
        sectors.emplace(0, z, std::move(oldpointer));

				// reset it completely
				Seamstress::resetSectorColumn(sectors(0, z));
				// flag neighboring sector as dirty, if necessary
				Seamstress::updateSectorColumn(EDGE_NO, z);

				// reset edge columns
				columns(0, z, world.getDeltaX(), world.getDeltaZ()).reset();

			} // sectors z
			mtx.sectorseam.unlock();
      // minimap rollover +x
			minimap.roll(-1, 0);
			returnvalue = true;
		}
		else if (player.pos.x >= halfworld + Seamless::OFFSET)
		{
			mtx.sectorseam.lock();

			// move player back one sector (in blocks)
			player.pos.x -= Sector::BLOCKS_XZ;
			player.snap_pos.x -= Sector::BLOCKS_XZ;
			// offset world x by +1
			world.worldCoords.x += 1;
			world.increaseDelta(1, 0);

			// only 25% left on the positive side
			for (int z = 0; z < sectors.getXZ(); z++)
			{
				// remember first sector on x-axis
				auto oldpointer = sectors.extract(0, z);

				for (int x = 0; x < sectors.getXZ()-1; x++)
				{
					sectors.replace(x,z, x+1,z);
				}

				// move oldpointer-sector to end of x-axis
        sectors.emplace(sectors.getXZ()-1, z, std::move(oldpointer));

				// reset sector completely
				Seamstress::resetSectorColumn(sectors(sectors.getXZ()-1, z));
				// update neighbor
				Seamstress::updateSectorColumn(sectors.getXZ()-1-EDGE_NO, z);

				// reset edge columns
				columns(sectors.getXZ()-1, z, world.getDeltaX(), world.getDeltaZ()).reset();

			} // sectors z
		  mtx.sectorseam.unlock();
      // minimap rollover -x
			minimap.roll(1, 0);
			returnvalue = true;

		} // seamless +/- x

		// -== Z axis ==-

		if (player.pos.z <= halfworld - Seamless::OFFSET)
		{
			mtx.sectorseam.lock();

			// offset player +z
			player.pos.z += Sector::BLOCKS_XZ;
			player.snap_pos.z += Sector::BLOCKS_XZ;
			// offset world -z
			world.worldCoords.z -= 1;
			world.increaseDelta(0, -1);

			// only 25% left on the negative side
			for (int x = 0; x < sectors.getXZ(); x++)
			{
				// recursively move the sector
				auto oldpointer = sectors.extract(x, sectors.getXZ()-1);

				for (int z = sectors.getXZ()-1; z >= 1; z--)
				{
					sectors.replace(x,z,  x,z-1);
				}
				// move old pointer to beginning of z axis
        sectors.emplace(x, 0, std::move(oldpointer));

				// reset oldpointer column
				Seamstress::resetSectorColumn(sectors(x, 0));
				// only need to update 1 row for Z
				Seamstress::updateSectorColumn(x, EDGE_NO);

				// reset edge columns
				columns(x, 0, world.getDeltaX(), world.getDeltaZ()).reset();

			} // sectors x
			mtx.sectorseam.unlock();
      // minimap rollover +z
			minimap.roll(0, -1);
			return true;
		}
		else if (player.pos.z >= halfworld + Seamless::OFFSET)
		{
			mtx.sectorseam.lock();

			// move player backward on the Z axis
			player.pos.z -= Sector::BLOCKS_XZ;
			player.snap_pos.z -= Sector::BLOCKS_XZ;
			// move world forward on the Z axis
			world.worldCoords.z += 1;
			world.increaseDelta(0, 1);

			// move sectors forwards +z (and rollback last line)
			for (int x = 0; x < sectors.getXZ(); x++)
			{
				auto oldpointer = sectors.extract(x, 0);

				// recursively move sectors
				for (int z = 0; z < sectors.getXZ()-1; z++)
				{
					sectors.replace(x,z, x,z+1);
				}
				// move sector to end of z axis
        sectors.emplace(x, sectors.getXZ()-1, std::move(oldpointer));

				// reset oldpointer column
				Seamstress::resetSectorColumn(sectors(x, sectors.getXZ()-1));
				// only need to update 1 row for Z
				Seamstress::updateSectorColumn(x, sectors.getXZ()-1-EDGE_NO);

				// reset edge columns
				columns(x, sectors.getXZ()-1, world.getDeltaX(), world.getDeltaZ()).reset();

			} // sectors x
			mtx.sectorseam.unlock();
      // minimap rollover -z
			minimap.roll(0, 1);
			return true;

		} // seamless +/- z

		// final returnvalue
		return returnvalue;

	} // seamlessness

}
