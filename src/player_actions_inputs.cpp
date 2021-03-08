#include "player_actions.hpp"

#include "game.hpp"

namespace cppcraft
{
	void PlayerActions::handleInputs()
	{
		static bool lock_hand_axes = false;
		int result = 99;
		bool trigger_left = false;
		bool trigger_right = false;

		if (keyconf.joy_enabled)
		{
			if (keyconf.joy_dual_axis_mining)
			{
				trigger_left = keyconf.jaxis[keyconf.joy_axis_place] > 0.5;
				trigger_right = keyconf.jaxis[keyconf.joy_axis_place] < -0.5;
			}
			else
			{
				trigger_left = keyconf.jaxis[keyconf.joy_axis_place] > 0.5;
				trigger_right = keyconf.jaxis[keyconf.joy_axis_mine] > 0.5;
			}
		}

		// left mouse button
		if (game.input().mouse_button(keyconf.mouse_btn_place) || trigger_left)
		{
			if (lock_hand_axes == false)
			{
				lock_hand_axes = true;
				result = 0; // place/add block
			}
		}
		else if (game.input().mouse_button(keyconf.mouse_btn_mine) || trigger_right)
		{
			if (lock_hand_axes == false)
			{
				lock_hand_axes = true;
				result = 1; // mine/remove block
			}
		}
		else
		{
			lock_hand_axes = false;
			result = 3;
		}

		/// perform some action determined by inputs above ///
		switch (result)
		{
		case 0:
			// add block
			handWave();
			//this->startAction(PA_Addblock);
			action = PA_Addblock;
			break;
		case 1:
			// enable mining
			handWave();
			mineTimer = 2000;
			mineMax   = 2000;
			minimizer = -1; // CRC
			action = PA_Mineblock;
			break;

		case 99: // do nothing
			break;

		default:
			// cancel stuff
			cancelDig();
		}
	}

}
