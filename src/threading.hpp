#ifndef THREADING_HPP
#define THREADING_HPP

#include <thread>
#include <mutex>

namespace cppcraft
{
	class WorldManager;

	class ThreadingClass {
	public:
		// world manager thread
		std::thread worldman;

		// mutexes
		std::mutex sectorseam;
		std::mutex playermove;
		std::mutex playerselection;
		std::mutex objects;
		std::mutex inetq;
		std::mutex netplayers;
		std::mutex actors;

		void initThreading(WorldManager& worldman);
		void cleanupThreading();
	};
	extern ThreadingClass mtx;

}

#endif
