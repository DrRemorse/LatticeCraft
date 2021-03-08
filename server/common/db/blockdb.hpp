#pragma once
#include "blockdata.hpp"
#include "database.hpp"

namespace db
{
  using BlockDB = Database<BlockData>;

	inline int getb(const std::string& name)
	{
		return BlockDB::get()[name];
	}
}
