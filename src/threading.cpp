#include "threading.hpp"

#include "worldmanager.hpp"

namespace cppcraft
{
	ThreadingClass mtx;

	void ThreadingClass::initThreading(WorldManager& worldman)
	{
		// start world manager thread
		mtx.worldman = std::thread(&WorldManager::submain, &worldman);
	}

	void ThreadingClass::cleanupThreading()
	{
		// wait for worldman thread to end
		mtx.worldman.join();
	}

}
