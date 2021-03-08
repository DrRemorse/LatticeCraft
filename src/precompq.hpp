#ifndef PRECOMPQ_HPP
#define PRECOMPQ_HPP

#include <cstdint>
#include <list>

namespace cppcraft
{
	class Sector;
	class Precomp;

	class PrecompQ {
	public:
		//! \brief Queues a sector for the mesh generator subsystem
		void add(Sector& sector);

		//! \brief executes one round of mesh generation scheduling
		//! \warn  very time consuming, running N threads in parallell and waits for them to finish
		void run();

		std::size_t size() const noexcept
		{
			return queue.size();
		}

    // for debugging purposes
    bool contains(Sector&) const;

	private:
		// starting a job is actually a little complicated
		void startJob(Sector& sector);

		// queue of sectors waiting for mesh generation
		std::list<Sector*> queue;
	};
	extern PrecompQ precompq;
}

#endif
