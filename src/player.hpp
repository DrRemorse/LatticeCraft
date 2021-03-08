#ifndef PLAYER_HPP
#define PLAYER_HPP

#define DBG_FLYING

#include "block.hpp"
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

struct GLFWwindow;

namespace cppcraft
{
	class Sector;
	class InventoryItem;

	class PlayerClass {
	public:
		// position
		glm::vec3 pos;
		glm::vec3 snap_pos;
		int    snapStage;

		// rotation in radians
		glm::vec2 rot;

		// acceleration
		glm::vec3 accel;
		// no-gravity toggle flag
		bool Flying;
		// true if player moved voluntarily, update camera etc.
		bool JustMoved;
		// true if player changed position
		bool changedPosition;
		// true if player changed camera orientation (head rotation)
		bool changedRotation;

		void initPlayer();
		void initInputs(GLFWwindow*);
		// first we run this, to get inputs
		void handleInputs();
		// handle any Joystick
		void handleJoystick();
		// then, for each tick passed we run this
		void handlePlayerTicks(double time);
		// then finally, we finish by running this once
		void handleActions(double frametime);

		bool fullySubmerged() const;

		// handles player rotation by following the input rotation vector
		void handleRotation();
		// returns the player look vector
		glm::vec3 getLookVector() const;
		// returns the cube face that would face the player
		block_t getBlockFacing() const;
		// returns true if we are using in-game controls, or busy with something else
		bool busyControls() const;
	};
	extern PlayerClass player;

}

#endif
