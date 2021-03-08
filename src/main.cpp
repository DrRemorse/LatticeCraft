/**
 * Everyzing ztarts hier
 *
 * LOC:
 * find . -name '*.?pp' | xargs wc -l | tail -1
 *
**/

#include <library/config.hpp>
#include <library/log.hpp>
#include "game.hpp"
#include "gameconf.hpp"
#include "generator.hpp"
#include "renderman.hpp"
#include "threading.hpp"
#include "worldmanager.hpp"
#include <string>

const std::string configFile = "config.ini";
const std::string logFile    = "cppcraft.log";

using namespace library;
using namespace cppcraft;
#ifdef ENABLE_GPERF
extern "C" int ProfilerStart(const char* fname);
#endif

int main(int argc, char* argv[])
{
	// start logging to file
	//logger.open(logFile);
	logger << Log::INFO << "Starting up..." << Log::ENDL;
  // std is a default mod
  game.add_mod("std");

	std::string wfolder = "";
	if (argc > 1)
	{
		wfolder = argv[1];
		logger << Log::INFO << "Using world: " << wfolder << Log::ENDL;
	}

	// read config file
	if (config.load(configFile) == false)
		logger << Log::WARN << "[!] Could not find config file: " << configFile << Log::ENDL;

	// read game configuration
	gameconf.init();

	// initialize renderer
	std::unique_ptr<Renderer> renderer = nullptr;
	renderer.reset(new Renderer("test window"));

	// initialize game/world manager
	std::unique_ptr<WorldManager> worldman = nullptr;
	worldman.reset(new WorldManager(WorldManager::GS_RUNNING, renderer->window(), wfolder));

	// prepare renderer
	renderer->prepare();

	// load player location
	worldman->initPlayer();

#ifdef ENABLE_GPERF
  // start profiling
  ProfilerStart("/tmp/prof.out");
#endif

	// start world manager thread
	mtx.initThreading(*worldman);

	logger << Log::INFO << "* Starting renderer..." << Log::ENDL;
	// get stuck in rendering-loop
	renderer->renderloop();
	//////////////////////////////

	logger << Log::INFO << "Ending..." << Log::ENDL;
	// cleanup
	mtx.cleanupThreading();
	return EXIT_SUCCESS;
}
