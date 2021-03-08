#include "worldmanager.hpp"

#include <library/log.hpp>
#include <library/timing/timer.hpp>
#include <library/sleep.hpp>
#include "chunks.hpp"
#include "game.hpp"
#include "generator.hpp"
#include "generator/objectq.hpp"
#include "generator/simulation/simulator.hpp"
#include "lighting.hpp"
#include "particles.hpp"
#include "player.hpp"
#include "precompq.hpp"
#include "seamless.hpp"
#include "soundman.hpp"
#include "sun.hpp"
#include "world.hpp"

using namespace library;
//#define TIMING

namespace cppcraft
{
	void WorldManager::submain()
	{
		main();
		exit();
	}
	void WorldManager::main()
	{
		// integral delta timing
		Timer timer;
		double localTime = timer.getTime();
		double _ticktimer = localTime;

		// world manager main loop
		while (game.is_terminating() == false)
		{
			// fixed timestep
			localTime = timer.getTime();

			try
			{
				// handle player inputs, just once
				player.handleInputs();
			}
			catch (std::string exc)
			{
				logger << Log::ERR << "Error from player.handleInputs(): " << exc << Log::ENDL;
				break;
			}

			// integrator
			double _localtime = localTime;

			while (_localtime >= _ticktimer + TIMING_TICKTIMER)
			{
				//----------------------------------//
				//       PLAYER RELATED STUFF       //
				//----------------------------------//
				player.handlePlayerTicks(_ticktimer);

        ///----------------------------------///
				/// ---------- WORLD SIM ----------- ///
				///----------------------------------///
        terragen::Simulator::run(_ticktimer);

				// handle actors & particles & objects
				particleSystem.update(_ticktimer);

				// handle sound, music & ambience
				soundman.sound_processing();

				_ticktimer += TIMING_TICKTIMER;

				// fixed timestep
				_localtime = timer.getTime();
			}

			try
			{
				/// if the player moved, or is currently doing stuff we will be doing it here ///

				// this function changes the player.positionChanged value each round
				// also transports relevant player values to rendering thread
				player.handleActions(localTime);
			}
			catch (std::string exc)
			{
				logger << Log::ERR << "Error from player.handleActions(): " << exc << Log::ENDL;
				break;
			}

#ifdef TIMING
      double timings[8] = {0.0};
#endif
			while (true)
			{
				///----------------------------------///
				///        SEAMLESS TRANSITION       ///
				///----------------------------------///
#ifdef TIMING
        Timer seamless_timer;
#endif
				bool transition = Seamless::run();
#ifdef TIMING
        timings[0] = seamless_timer.getTime();
#endif
        if (transition) break;

				double timeOut = localTime + MAX_TIMING_WAIT;

				///----------------------------------///
				/// ---------- OBJECT GEN ---------- ///
				///----------------------------------///
#ifdef TIMING
        Timer objectq_timer;
#endif
				terragen::ObjectQueue::run();
#ifdef TIMING
        timings[1] = objectq_timer.getTime();
#endif

				// check for timeout
				if (timer.getTime() > timeOut) break;

				///----------------------------------///
				/// --------- PRECOMPILER ---------- ///
				///----------------------------------///
#ifdef TIMING
        Timer meshgen_timer;
#endif
				// start scheduling sectors for meshgen
				precompq.run();
#ifdef TIMING
        timings[2] = meshgen_timer.getTime();
#endif

        // check for timeout
				if (timer.getTime() > timeOut) break;

				///----------------------------------///
				/// ---------- GENERATOR ----------- ///
				///----------------------------------///
#ifdef TIMING
        Timer generator_timer;
#endif
				Generator::run();
#ifdef TIMING
        timings[3] = generator_timer.getTime();
#endif

        // check for timeout
				if (timer.getTime() > timeOut) break;

				// update shadows if sun has travelled far
				// but not when connected to a network
				//if (network.isConnected() == false)
				//{
				//	thesun.travelCheck();
				//}

				teleportHandler();

#ifdef TIMING
        Timer lighting_timer;
#endif
        // make scheduled corrections to lighting
        Lighting::handleDeferred();
#ifdef TIMING
        timings[4] = lighting_timer.getTime();
#endif
				break;
			} // world tick

#ifdef TIMING
      double sum = timings[0] + timings[1] + timings[2] + timings[3] + timings[4];
      if (sum > 0.001) {
      printf("Timings: SEAM %f  OBJQ %f  MESH %f  GEN %f  LIGHT %f\n",
            timings[0], timings[1], timings[2], timings[3], timings[4]);
      }
#endif
			// send & receive stuff
			//network.handleNetworking();

			// flush chunk write queue
			chunks.flushChunks();

			// this shit won't work...
			/*if (timer.getDeltaTime() < localTime + TIMING_SLEEP_TIME)
			{
				sleepMillis(6);
			}*/
		}

	} // world thread

}
