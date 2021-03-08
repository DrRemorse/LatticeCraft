#pragma once
#include "database.hpp"
#include "itemdata.hpp"

namespace db
{
  using ItemDB = Database<ItemData>;

	inline int geti(const std::string& name)
	{
		return ItemDB::get()[name];
	}
}
