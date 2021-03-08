#pragma once

#include "object.hpp"
#include <list>
#include <vector>

namespace terragen
{
	class ObjectQueue {
	public:
		static void add(const std::vector<SchedObject>& queue)
		{
			for (auto& obj : queue)
				get().objects.push_back(obj);
		}

    static void init();

		static void run()
		{
			get().run_internal();
		}
		static ObjectQueue& get();

    static std::size_t size() noexcept {
      return get().objects.size();
    }
    static std::size_t retry_size() noexcept {
      return get().retry_objects.size();
    }

    // for debugging purposes
    static bool contains(Sector&);

	private:
		void run_internal();

		std::list<SchedObject> objects;
    std::list<SchedObject> retry_objects;
	};
}
