/**
 * 'World' contains information about world source and the location in
 * this huge-finite world. 'folder' is the directory which the worlds
 * world-data is contained in, including some "permanent" player information.
 *
 * The WORLD coordinates are measured in sectors (of Sector::BLOCKS_XZ blocks)
 * and represents the (0, 0) local sector corner for player view, or alternatively
 * the (0, 0, 0) (left, bottom, back) corner in a 3D setting.
 *
 * Example transition +x:
 * The player is moved backward exactly ONE sector, and worldX increased
 * by +1, when player reaches the seamless OFFSET treshold.
 *
**/

#ifndef WORLD_HPP
#define WORLD_HPP

#include <string>

namespace cppcraft
{
	class World {
	public:
		typedef int wcoord_t;
		static const wcoord_t WORLD_SIZE     = 134217728;
		static const wcoord_t WORLD_CENTER   = WORLD_SIZE / 2;
    static const wcoord_t WORLD_STARTING_X = WORLD_CENTER;
		static const wcoord_t WORLD_STARTING_Z = WORLD_STARTING_X;

		struct world_t
    {
			world_t(wcoord_t wx, wcoord_t wy, wcoord_t wz) : x(wx), y(wy), z(wz) {}
			wcoord_t x, y, z;
		};

		// initializes the world
		void init(const std::string& folder);
		// returns current world folder
		const std::string& worldFolder() const
		{
			return this->folder;
		}

		wcoord_t getWX() const { return worldCoords.x; }
		wcoord_t getWY() const { return worldCoords.y; }
		wcoord_t getWZ() const { return worldCoords.z; }

		wcoord_t getDeltaX() const { return internal.x; }
		wcoord_t getDeltaY() const { return internal.y; }
		wcoord_t getDeltaZ() const { return internal.z; }

		void load();
		void save();

	private:
		std::string folder;
		world_t worldCoords{0, 0, 0};
		world_t internal{0, 0, 0};

		void increaseDelta(int dx, int dz);

		void transitionTo(int wx, int wz);

		// seamless transitions the word, and is the only one
		// allowed to directly modify worldX, worldZ
		friend class Seamless; // special customer
		friend class WorldManager; // teleport
	};
	extern World world;
}

#endif
