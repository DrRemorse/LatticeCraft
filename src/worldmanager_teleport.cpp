#include "worldmanager.hpp"

#include <library/opengl/opengl.hpp>
#include "chunks.hpp"
#include "columns.hpp"
#include "game.hpp"
#include "player.hpp"
#include "compiler_scheduler.hpp"
#include "sectors.hpp"
#include "threading.hpp"

using namespace library;

namespace cppcraft
{
	bool teleport_teleport = false;
	World::world_t teleport_wcoords{0, 0, 0};
	glm::vec3 teleport_xyz;

	void WorldManager::teleport(const World::world_t& coords, const glm::vec3& position)
	{
		// center local grid on world coordinate location
		teleport_wcoords.x = coords.x - sectors.getXZ() / 2;
		teleport_wcoords.y = 0;
		teleport_wcoords.z = coords.z - sectors.getXZ() / 2;
		// center player on grid, add his local coordinates and offset by world coords.y
		teleport_xyz.x = position.x + Sector::BLOCKS_XZ * sectors.getXZ() / 2;
		teleport_xyz.y = position.y + Sector::BLOCKS_Y  * coords.y;
		teleport_xyz.z = position.z + Sector::BLOCKS_XZ * sectors.getXZ() / 2;
		// bzzzzzzzzzzzz
		teleport_teleport = true;
	}

	void WorldManager::teleportHandler()
	{
		if (game.input().key(GLFW_KEY_P) == Input::KEY_PRESSED)
		{
			game.input().key_hold(GLFW_KEY_P);

			teleport_wcoords.x = World::WORLD_CENTER + 64;
			teleport_wcoords.z = World::WORLD_CENTER + 64;

			teleport_xyz = player.pos;
			teleport_teleport = true;
		}

		if (teleport_teleport == false) return;
		teleport_teleport = false;

		// flush chunk queue
		chunks.flushChunks();
		// clear precomp scheduler
		CompilerScheduler::reset();

		mtx.sectorseam.lock();
		{
			// transition to new location
			world.transitionTo(teleport_wcoords.x, teleport_wcoords.z);

			// invalidate ALL sectors
			sectors.regenerateAll();

			// move player to:
			// center grid, center sector, center block
			player.pos = teleport_xyz;

			// disable all terrain meshes
			for (int x = 0; x < sectors.getXZ(); x++)
			for (int z = 0; z < sectors.getXZ(); z++)
			for (int y = 0; y < columns.getHeight(); y++)
			{
				columns(x, z, world.getDeltaX(), world.getDeltaZ()).reset();
			}
		}
		mtx.sectorseam.unlock();
	}

}
