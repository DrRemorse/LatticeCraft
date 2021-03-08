#include "world.hpp"

#include <library/config.hpp>
#include "sectors.hpp"
#include "gameconf.hpp"
#include "player.hpp"
#include <fstream>

using namespace library;

namespace cppcraft
{
	// one and only instance of WorldClass
	World world;

	void World::init(const std::string& worldFolder)
	{
		/// initialize sectors, blocks & flatlands ///
		// default world folder
		const std::string DEFAULT_WORLD_FOLDER = "Worlds/test";

    const int view_dist = config.get("world.viewdist", 256) / BLOCKS_XZ;
		const int sectors_xz = view_dist * 2 + 4;
		sectors.rebuild(sectors_xz);

		/// load world position from folder ///

		// initialize world coordinates centering player in the world
		worldCoords.x = WORLD_STARTING_X - sectors_xz / 2; //+ 84; //
		worldCoords.y = 0;
		worldCoords.z = WORLD_STARTING_Z - sectors_xz / 2; //- 104; //

		// finally, world folder, where world-data is located
		if (worldFolder.size() == 0)
		{
			std::string folder = config.get("world", "");

			if (folder.size() == 0)
				this->folder = DEFAULT_WORLD_FOLDER;
			else
				this->folder = folder;
		}
		else
		{
			this->folder = worldFolder;
		}

		// initialize internal coordinates
		world.internal.x = 0;
		world.internal.y = 0;
		world.internal.z = 0;
	}

	void World::load()
	{
		std::ifstream ff (worldFolder() + "/world.data", std::ios::in | std::ios::binary);
		if (!ff) return;

		ff.seekg(0);
		ff.read( (char*) &worldCoords, sizeof(worldCoords) );
		ff.read( (char*) &player, sizeof(player) );
	}
	void World::save()
	{
		std::ofstream ff (worldFolder() + "/world.data", std::ios::trunc | std::ios::binary);
		if (!ff) return;

		ff.seekp(0);
		ff.write( (char*) &worldCoords, sizeof(worldCoords) );
		ff.write( (char*) &player, sizeof(player) );
	}

	void World::increaseDelta(int dx, int dz)
	{
		internal.x += dx;
		while (internal.x < 0) internal.x += sectors.getXZ();
		internal.z += dz;
		while (internal.z < 0) internal.z += sectors.getXZ();
	}

	void World::transitionTo(int wx, int wz)
	{
		internal.x = 0;
		internal.z = 0;
		this->worldCoords.x = wx;
		this->worldCoords.z = wz;
	}
}
