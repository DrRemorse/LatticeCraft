#ifndef WORLDMANAGER_HPP
#define WORLDMANAGER_HPP

#include "world.hpp"
#include <glm/vec3.hpp>
#include <string>

struct GLFWwindow;

namespace cppcraft
{
	class Chunks;
	class PrecompQ;

	class WorldManager {
	public:
		enum gamestate_t
		{
			GS_INIT,
			GS_INTRO,
			GS_TRANSITION,
			GS_RUNNING,
			GS_PAUSED,
			GS_DEATH,

			GS_EXIT
		};

		WorldManager(gamestate_t, GLFWwindow*, const std::string& folder);
		void exit();
		void initPlayer();
		// running thread function
		void submain();
		void main();

		void teleport(const World::world_t& coords, const glm::vec3& position);
		void teleportHandler();

		gamestate_t getState() const noexcept
		{
			return this->gamestate;
		}

    static constexpr double TIMING_TICKTIMER = 0.0167;
    static constexpr double MAX_TIMING_WAIT  = 0.012;
  	static constexpr double TIMING_SLEEP_TIME = TIMING_TICKTIMER / 2.0;
	private:
		gamestate_t gamestate;
	};

}

#endif
