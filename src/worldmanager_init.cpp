#include "worldmanager.hpp"

#include "blockmodels.hpp"
#include "chunks.hpp"
#include "lighting.hpp"
#include "generator.hpp"
#include "particles.hpp"
#include "player.hpp"
#include "sectors.hpp"
#include "soundman.hpp"
#include "threadpool.hpp"
#include "world.hpp"

namespace cppcraft
{
  Sectors sectors(0);

	WorldManager::WorldManager(
      gamestate_t gs,
      GLFWwindow* window,
      const std::string& worldFolder)
	{
		this->gamestate = gs;
		// initalize world
		world.init(worldFolder);
    // initialize block meshes
		blockmodels.init();
		// initialize player
		player.initPlayer();
		// initialize chunk systems
		chunks.initChunks();
		// initialize threads
		AsyncPool::init();
		// initialize lighting
		Lighting::init();

		// initialize blocks and such
		Generator::init();

		// initialize particles
		particleSystem.init();

		// initialize sound
		soundman.init();

		// initialize keyboard / joystick input
		player.initInputs(window);
	}
	void WorldManager::exit()
	{
		// flush if queue still exists
		chunks.flushChunks();

		// save our stuff!
		world.save();

    // stop threadpool
    AsyncPool::stop();
	}

	void WorldManager::initPlayer()
	{
		// center grid, center sector, center block
		player.pos.x = ((float)sectors.getXZ() / 2.0 - 0.5) * Sector::BLOCKS_XZ + 0.5;
		player.pos.z = player.pos.x;
		player.pos.y = Sector::BLOCKS_Y * 0.75;
		// load world data (if any)
		world.load();
	}
}
