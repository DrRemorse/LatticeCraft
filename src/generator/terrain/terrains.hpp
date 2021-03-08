#pragma once

#include "terrain.hpp"
#include <vector>
#include <map>

namespace terragen
{
	class Terrains {
	public:
		static void init();

		Terrain& operator[] (int id)
		{
			return tvec.at(id);
		}
		int operator[] (const std::string& name)
		{
			return names[name];
		}

		template <typename... Args>
		Terrain& add(const std::string& name, Args&&... args)
		{
			size_t index = tvec.size();
			tvec.emplace_back(index, args...);
			names[name] = index;
			return tvec.back();
		}

		size_t size() const noexcept
		{
			return tvec.size();
		}

    const auto& get() const { return tvec; }

	private:
		std::vector<Terrain> tvec;
		std::map<std::string, int> names;
	};
	extern Terrains terrains;
  extern Terrains cave_terrains;
}
