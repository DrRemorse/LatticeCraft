#pragma once

#include <common.hpp>
#include <block.hpp>
#include "delegate.hpp"
#include <map>
#include <string>

namespace terragen
{
	using cppcraft::BLOCKS_XZ;
	using cppcraft::BLOCKS_Y;
	using cppcraft::Block;
	using cppcraft::Sector;

	struct SchedObject
	{
		SchedObject(const std::string& gname, int X, int Y, int Z, int64_t D)
			: name(gname), x(X), y(Y), z(Z), data(D) {}
    SchedObject(SchedObject&&) = default;
    SchedObject(const SchedObject&) = default;

		int getWX() const {
			return x / BLOCKS_XZ;
		}
		int getWZ() const {
			return z / BLOCKS_XZ;
		}

		const std::string name;
		int x, y, z;
    // whatever you want it to be
    const int64_t data;
	};

  struct GenObject
  {
    typedef delegate<void(const SchedObject&)> callback_t;

    GenObject(callback_t F, int S) : func(F), size(S) {}
    callback_t func;
    uint16_t   size;
  };

	struct ObjectDB
  {
    template <typename... Args>
    void add(const char* name, Args&&... args) {
      objects.emplace(std::piecewise_construct,
                      std::forward_as_tuple(name),
                      std::forward_as_tuple(std::forward<Args>(args)...));
    }

		GenObject& operator[] (const std::string& name)
		{
			return objects.at(name);
		}

	private:
		std::map<std::string, GenObject> objects;
	};
	extern ObjectDB objectDB;
}
