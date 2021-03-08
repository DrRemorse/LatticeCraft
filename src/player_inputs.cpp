#include "player.hpp"

#include <library/log.hpp>
#include <library/opengl/opengl.hpp>
#include "chat.hpp"
#include "game.hpp"
#include "gameconf.hpp"
#include "player_inputs.hpp"
#include "player_logic.hpp"
#include "sun.hpp"
#include "threading.hpp"
#include <cmath>
#include <set>
#include <string>

using namespace library;

namespace cppcraft
{
	keyconf_t keyconf;

  void Input::handle()
  {
    // poll for events
    glfwPollEvents();

    // joystick rotation
    if (keyconf.joy_enabled)
		{
			float dx = tresholdValue(keyconf.jaxis[keyconf.joy_axis_look_xrot]);
			float dy = tresholdValue(keyconf.jaxis[keyconf.joy_axis_look_yrot]);

			this->add_rotation(glm::vec2(dx, dy) * keyconf.joy_speed);
		}

    // interpolate player rotation and signals camera refresh
    player.handleRotation();
  }

  void Input::restore_inputs()
  {
    this->restore_keyboard();
    this->restore_mouse();
    this->mouse_grab(true);
  }

	void PlayerClass::initInputs(GLFWwindow* window)
	{
		logger << Log::INFO << "* Initializing input systems" << Log::ENDL;
    game.input().init(window, true, true);
    game.input().set_rotation(player.rot);
    //game.input().mouse_options();
    game.input().mouse_grab(true);

		/// Keyboard configuration
    keyconf.k_escape   = GLFW_KEY_ESCAPE;
		keyconf.k_forward  = config.get("k_forward",  GLFW_KEY_W); // W
		keyconf.k_backward = config.get("k_backward", GLFW_KEY_S); // S
		keyconf.k_right    = config.get("k_right",    GLFW_KEY_D); // D
		keyconf.k_left     = config.get("k_left",     GLFW_KEY_A); // A

		keyconf.k_jump   = config.get("k_jump", GLFW_KEY_SPACE);   // Space
		keyconf.k_sprint = config.get("k_sprint", (int) GLFW_KEY_LEFT_SHIFT);
		keyconf.k_crouch = config.get("k_crouch", (int) GLFW_KEY_LEFT_CONTROL);
		keyconf.k_throw     = config.get("k_throw",     GLFW_KEY_Q); // Q
    keyconf.k_inventory = config.get("k_inventory", GLFW_KEY_E); // E

		keyconf.k_flying  = config.get("k_flying", GLFW_KEY_F); // F
		keyconf.k_flyup   = config.get("k_flyup",  GLFW_KEY_T); // T
		keyconf.k_flydown = config.get("k_flydown",GLFW_KEY_R); // R


		/// Mouse configuration

		double mspd  = config.get("mouse.speed", 80) / 1000.0;
		double msens = config.get("mouse.sens",  40)  / 10.0;

		keyconf.alternateMiningButton = config.get("mouse.swap_buttons", false);
		keyconf.mouse_btn_place = (keyconf.alternateMiningButton)
                  ? GLFW_MOUSE_BUTTON_RIGHT : GLFW_MOUSE_BUTTON_LEFT;
		keyconf.mouse_btn_mine = (keyconf.alternateMiningButton)
                  ? GLFW_MOUSE_BUTTON_LEFT : GLFW_MOUSE_BUTTON_RIGHT;

		// initialize joystick support
		keyconf.joy_enabled = config.get("joy.enabled", false);
		if (keyconf.joy_enabled)
		{
			keyconf.joy_index   = config.get("joy.index", 0);
			//keyconf.joy_enabled = SDL_IsGameController(keyconf.joy_index);
      keyconf.joy_enabled = false;

			if (keyconf.joy_enabled)
			{
				//std::string jname(SDL_GameControllerNameForIndex(keyconf.joy_index));
        std::string jname = "";
				logger << Log::INFO << "* Joystick: " << jname << Log::ENDL;

				keyconf.joy_deadzone = config.get("joy.deadzone", 0.12);
				keyconf.joy_speed    = config.get("joy.rotspeed", 2.0);

				/// joystick configuration ///
				keyconf.joy_axis_sidestep = config.get("joy.axis_sides", 0);
				keyconf.joy_axis_forward = config.get("joy.axis_forw", 1);

				keyconf.joy_axis_look_xrot = config.get("joy.axis_xrot", 3);
				keyconf.joy_axis_look_yrot = config.get("joy.axis_yrot", 4);

				keyconf.joy_axis_place = config.get("joy.axis_place", 2);

				keyconf.joy_dual_axis_mining = config.get("joy.dual_axis_mining", true);
				if (keyconf.joy_dual_axis_mining)
				{
					// dual axis, so placement has same axis as mining
					keyconf.joy_axis_mine = keyconf.joy_axis_place;
				}
				else
				{
					keyconf.joy_axis_mine = config.get("joy.axis_mine", 2);
				}

				keyconf.joy_btn_jump   = config.get("joy.btn_jump", 0);
				keyconf.joy_btn_sprint = config.get("joy.btn_sprint", 2);
				keyconf.joy_btn_crouch = config.get("joy.btn_crouch", 3);

				keyconf.joy_btn_previtem = config.get("joy.btn_previtem", 4);
				keyconf.joy_btn_nextitem = config.get("joy.btn_nextitem", 5);

				keyconf.joy_btn_flying = config.get("joy.btn_flying", 9);
				keyconf.joy_btn_exit   = config.get("joy.btn_exit", 6);
			}
			else
			{
				logger << Log::INFO << "* Joystick with index " << keyconf.joy_index << " did not exist." << Log::ENDL;
			}
		}
		if (keyconf.joy_enabled == false)
		{
			keyconf.jbuttons = new unsigned char[16]();
			keyconf.jaxis = new float[8]();
		}

	} // PlayerClass::initInputs

	void PlayerClass::handleJoystick()
	{
		/// BUTTONS

		// 0 = A button
		// 1 = B button
		// 2 = X button
		// 3 = Y button

		// 4 = left bumper
		// 5 = right bumper

		// 6 = select/back
		// 7 = start

		// 8 = left rotator button
		// 9 = right rotator button

		// 10 = dpad up
		// 11 = dpad right
		// 12 = dpad down
		// 13 = dpad left

		//keyconf.jbuttons = glfwGetJoystickButtons(keyconf.joy_index, &keyconf.joy_button_count);

		/*for (int i = 0; i < keyconf.joy_button_count; i++)
		{
			if (keyconf.jbuttons[i])
				logger << Log::INFO << "Button pressed: " << i << Log::ENDL;
		}*/

		/// AXES

		// 0 = left rotator (left/right)
		// 1 = left rotator (up/down)

		// 2 = trigger left/right

		// 3 = right rotator (left/right)
		// 4 = right rotator (up/down)

		//keyconf.jaxis = glfwGetJoystickAxes(keyconf.joy_index, &keyconf.joy_axis_count);

		/*for (int i = 0; i < keyconf.joy_axis_count; i++)
		{
			if (std::abs(keyconf.jaxis[i]) > 0.1)
				logger << Log::INFO << "Axis " << i << " value: " << keyconf.jaxis[i] << Log::ENDL;
		}*/
	}

	void PlayerClass::handleInputs()
	{
		// handle joystick if existing
		if (keyconf.joy_enabled) handleJoystick();

		// testing/cheats
		if (busyControls() == false)
		{
			if (game.input().key(GLFW_KEY_F1) == Input::KEY_PRESSED)
			{
				game.input().key_hold(GLFW_KEY_F1);

				thesun.setRadianAngle(3.14159 * 1/8);
			}
			if (game.input().key(GLFW_KEY_F2) == Input::KEY_PRESSED)
			{
				game.input().key_hold(GLFW_KEY_F2);

				thesun.setRadianAngle(3.14159 * 2/8);
			}
			if (game.input().key(GLFW_KEY_F3) == Input::KEY_PRESSED)
			{
				game.input().key_hold(GLFW_KEY_F3);

				thesun.setRadianAngle(3.14159 * 3/8);
			}
			if (game.input().key(GLFW_KEY_F4) == Input::KEY_PRESSED)
			{
				game.input().key_hold(GLFW_KEY_F4);

				thesun.setRadianAngle(-1);
			}

			if (game.input().key(keyconf.k_flying) == Input::KEY_PRESSED)
			{
				game.input().key_hold(keyconf.k_flying);
				// toggle flying
				player.Flying = ! player.Flying;
			}

      // inventory toggle
      if (game.input().key(keyconf.k_inventory) == Input::KEY_PRESSED)
      {
        game.input().key_hold(keyconf.k_inventory);
        game.input().mouse_grab(false);
        game.gui().takeover();
      }

			static bool lock_quickbar_scroll = false;
			const int wheel = game.input().mouse_wheel();
			if (wheel > 0 || keyconf.jbuttons[keyconf.joy_btn_nextitem])
			{
				if (lock_quickbar_scroll == false)
				{
					// next quickbar item
          game.gui().hotbar_select(game.gui().hotbar_index() + 1);
					lock_quickbar_scroll = true;
				}
			}
			else if (wheel < 0 || keyconf.jbuttons[keyconf.joy_btn_previtem])
			{
				if (lock_quickbar_scroll == false)
				{
					// previous quickbar item
          game.gui().hotbar_select(game.gui().hotbar_index() - 1);
					lock_quickbar_scroll = true;
				}
			}
			else
			{
				lock_quickbar_scroll = false;
			}
			// number keys (1-9) to directly select on quickbar
			for (int i = 1; i < 10; i++)
			if (game.input().key(GLFW_KEY_0 + i))
			{
        game.gui().hotbar_select(i - 1);
			}

		} // busyControls

		if (game.input().key(keyconf.k_escape) == Input::KEY_PRESSED || keyconf.jbuttons[keyconf.joy_btn_exit])
		{
			game.input().key_hold(keyconf.k_escape);

			if (chatbox.isOpen())
			{
				chatbox.openChat(false);
			}
			else
			{
        // its over
				game.terminate();
			}
		}

		if (game.input().key(GLFW_KEY_ENTER) == Input::KEY_PRESSED)
		{
			game.input().key_hold(GLFW_KEY_ENTER);
			chatbox.openChat(!chatbox.isOpen());

			if (chatbox.isOpen() == false)
			{
				// say something, as long as its something :)
				//if (!game.input().text().empty())
				//	network.sendChat(game.input().text());
			}
			game.input().text_clear();
		}

		if (game.input().key(GLFW_KEY_BACKSPACE) == Input::KEY_PRESSED)
		{
			game.input().key_hold(GLFW_KEY_BACKSPACE);
			if (chatbox.isOpen()) {
				game.input().text_backspace();
      }
		}

	} // handleInputs()
}
