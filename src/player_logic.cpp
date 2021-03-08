#include "player_logic.hpp"

#include <library/math/toolbox.hpp>
#include "generator/terrain/terrains.hpp"
#include "player.hpp"
#include "sector.hpp"
#include "soundman.hpp"

using namespace library;

namespace cppcraft
{
	// PlayerLogicClass
	PlayerLogic plogic;

  const terragen::Terrain& PlayerLogic::terrain() const
  {
    if (this->m_underground == false)
        return terragen::terrains[this->m_terrain];
    else
        return terragen::cave_terrains[this->m_terrain];
  }
  void PlayerLogic::detect_terrain()
  {
    if (this->sector != nullptr)
    {
      if (this->sector->generated())
      {
  			const auto& flat = this->sector->flat();
        const int x = int(player.pos.x) & (BLOCKS_XZ-1);
        const int z = int(player.pos.z) & (BLOCKS_XZ-1);
  			// if the player is below something, and its REALLY dark,
  			// then we change to super-dark terrain (T_CAVES)
        const int skylight = this->light & 0xFF;
  			if (player.pos.y < flat(x, z).groundLevel && skylight < 40)
        {
          const int y = int(player.pos.y);
          // underground
          this->m_underground = true;
          if (y >= 0)
  				    this->m_terrain = flat.cave(x, z).underworld[y / 4];
          else
              this->m_terrain = 0;
          return;
        }
  			else
        {
          this->m_underground = false;
  				this->m_terrain = flat(x, z).terrain;
          return;
        }
      }
		}
    // select the empty cave terrain by default
    this->m_underground = true;
    this->m_terrain = 0;
  }

	// sound timing intervals
	static const int TIME_FIRST_STEP      = 4;
	static const int TIME_BETWEEN_CROUCH  = 50;
	static const int TIME_BETWEEN_STEPS   = 32;
	static const int TIME_BETWEEN_SPRINTS = 20;
	int timesteps = TIME_FIRST_STEP;
	int stepsound = 0;

	void PlayerLogic::playerSounds()
	{
		if (Moved && Submerged == PS_None) // the player moved
		{
			if (timesteps == 0)
			{
				// play material sound, except for fluids, crosses and air
				if (block->hasSound())
				{
					int value = rand() & 1;
					stepsound = 0 + value;

					soundman.playMaterial(block->getSound(), stepsound,
              {player.pos.x, player.pos.y - 1.51f, player.pos.z});
				}

				if (movestate == PMS_Crouch)
				{
					timesteps = TIME_BETWEEN_CROUCH;
				}
				else if (movestate == PMS_Normal)
				{
					timesteps = TIME_BETWEEN_STEPS;
				}
				else if (movestate == PMS_Sprint)
				{
					timesteps = TIME_BETWEEN_SPRINTS;
				}

			}
			else timesteps--;
		}
		else timesteps = TIME_FIRST_STEP;
	}

}
