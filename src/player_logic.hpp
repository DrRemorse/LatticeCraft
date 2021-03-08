#ifndef PLAYER_LOGIC_HPP
#define PLAYER_LOGIC_HPP

#include "block.hpp"
#include "common.hpp"
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <cstdint>
#include <mutex>

namespace terragen {
  class Terrain;
}

namespace cppcraft
{
	class Sector;

	enum movestate_t
	{
		PMS_Normal,
		PMS_Crouch,
		PMS_Sprint

	};

	struct playerselect_t
	{
    // selected block
		Block block;
		// selected sector
		Sector* sector;
		// grid position
		glm::vec3 pos;
		// selection direction
		int facing;
		// additional info
		int info[2];
    // location as bits
    uint32_t checksum;
		bool updated;
	};

	class PlayerLogic
	{
	public:
		enum playerSubmerged_t
		{
			PS_None,
			PS_Water,
			PS_Lava
		};

		// players current sector
		Sector* sector;
		movestate_t movestate;

		// selection
		playerselect_t selection;

		// current terrain type
    const terragen::Terrain& terrain() const;
    void detect_terrain();

		// players current shadow/color
		light_value_t light = 0;

		// temporary for jump-locking
		bool jumplock = false;
		// temporary for fall testing
		bool freefall = false;
		// water escape mechanism
		bool EscapeAttempt = false;
		// status flags
		bool Falling  = false;
		bool Ladderized = false;
		bool Slowfall = false;
		bool Moved  = true;
		int  Motion = 0;
		playerSubmerged_t Submerged;
		playerSubmerged_t FullySubmerged;

	#ifdef USE_JETPACK
		// jetpacking
		bool canJetpack;
		bool jetpacking;

		short jetpack;
		short lastjet;
		static const short MAX_JETPACK = 32;
		static const short JETPACK_SOUNDWAIT = 24;
	#endif

		// player standing on this:
		Block* block;
		Block* lastblock;

		// returns true if the player has selected a block in the world
		bool hasSelection() const
		{
			return selection.sector != nullptr;
		}

		/// all movement & speed related ///
		void translatePlayer();
		void handlePlayerJumping();

		// walking / etc. sounds
		void playerSounds();

		glm::vec2 getLight() const
		{
			return glm::vec2(light & 0xFF, (light >> 8) & 0xFF) / 255.0f;
		}

		int determineSelectionFacing(const Block&, glm::vec3& ray, glm::vec3& fracs, float stepSize);

    auto& selection_mtx() noexcept { return m_selection_mtx; }

  private:
    std::mutex m_selection_mtx;
    bool m_underground = true;
    int  m_terrain = 0;
	};
	extern PlayerLogic plogic;
}

#endif
