/**
 * Generator
 *
 * Chunk loader, loading compressed / raw chunks
 * from world file system
 *
**/

#ifndef GENERATOR_HPP
#define GENERATOR_HPP

#include "sector.hpp"
#include <deque>

namespace library
{
	class Timer;
}

namespace cppcraft
{
	class Generator {
	public:
		// initialize the generator
		static void init();
    // schedule some generator jobs, from the queue
		static void run();

		// add generator jobs to queue
		static void add(Sector& sector) {
      if (!sector.generating()) {
          sector.add_genflag(Sector::GENERATING);
  		    queue.push_back(&sector);
      }
    }
		// returns the size of the internal queue
		static std::size_t size() {
      return queue.size();
    }

		// unused atm
		bool generate(Sector& sector);

	private:
		static void loadSector(Sector&, std::ifstream&, unsigned int);
    static std::deque<Sector*> queue;
	};
}

#endif
