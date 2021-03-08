#pragma once

#include <string>
#include <unordered_map>
#include <vector>

namespace db
{
  template <class Datatype>
	class Database {
	public:
		int operator[] (const std::string& name)
		{
			return names.at(name);
		}
		const Datatype& operator[] (int id) const
		{
			return storage[id];
		}
		Datatype& operator[] (int id)
		{
			return storage[id];
		}

		Datatype& create(const std::string& name = "")
		{
      // get ID
			const std::size_t ID = storage.size();
			// add to registry
			storage.emplace_back(ID);
			// associate name with ID
			if (!name.empty()) assign(name, storage.back());
			return storage.back();
		}
    void assign(const std::string& name, const Datatype& bd)
    {
      names.emplace(std::piecewise_construct,
          std::forward_as_tuple(name), std::forward_as_tuple(bd.getID()));
    }
		std::size_t size() const noexcept
		{
			return names.size();
		}

		// get the instance
		static Database<Datatype>& get()
		{
			static Database<Datatype> db;
			return db;
		}
		static const Database<Datatype>& cget()
		{
			return get();
		}

	private:
    // the ID of a type is its index into this vector
		std::vector<Datatype> storage; // NOTE: its VERY important its a vector
		// name to id conversion
		std::unordered_map<std::string, int> names;
	};
}
