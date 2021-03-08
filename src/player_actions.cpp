#include "player_actions.hpp"

namespace cppcraft
{
	PlayerActions paction;
	// timing intervals
	const double PlayerActions::MINE_SPEED = 0.04;
	double _actiontime;
	
	void PlayerActions::init()
	{
		action = PA_Nothing;
		mineTimer = 0;
		actionTimer  = 0;
		cooldownTime = 0;
	}
	
	void PlayerActions::handle(double frametime)
	{
		_actiontime = frametime;
		
		this->handleInputs();
		this->handleActions(frametime);
		
		if (action == playeraction_t::PA_Cooldown)
		{
			if (frametime > actionTimer + MINE_SPEED)
			{
				actionTimer = frametime;
				
				if (mineTimer)
				{
					mineTimer -= 1;
				}
				else
				{
					action = PA_Nothing;
					cancelDig(); // cancel after action != cooldown
					cooldownTime = frametime;
				}
			}
		}
	}
	
	PlayerActions::playeraction_t PlayerActions::getAction() const
	{
		return this->action;
	}
	
	float PlayerActions::getMiningLevel() const
	{
		return 1.0 - ((float) mineTimer / mineMax);
	}
	
	void PlayerActions::cancelDig()
	{
		if (action != playeraction_t::PA_Cooldown)
		{
			action = playeraction_t::PA_Nothing;
			/*
			if (network.mymine == true)
			{
				networkSend("PMINE 0");
				network.mymine = false;
			}*/
		}
	}
	
	void PlayerActions::handWave()
	{
		action = playeraction_t::PA_Cooldown;
		mineTimer = MINE_COOLDOWN;
		cooldownTime = _actiontime;
		/*
		if (network.mymine == false)
		{
			networkSend("PMINE 1");
			network.mymine = true;
		}*/
	}
	
	void PlayerActions::cooldown()
	{
		action = PA_Cooldown;
		mineTimer = MINE_COOLDOWN;
		cooldownTime = _actiontime;
	}
	
	void PlayerActions::swingTool(const Item& item)
	{
		action = playeraction_t::PA_Swingtool;
		actionTimer = 8;
	}
	
	
}