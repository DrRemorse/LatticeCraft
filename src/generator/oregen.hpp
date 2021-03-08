#pragma once
#include "terragen.hpp"
#include <block.hpp>
#include <vector>

namespace terragen
{
	using cppcraft::block_t;

	struct OreInfo
	{
		int  block_id;
		int  min_depth;
    int  cluster_min;
    int  cluster_max;
		int  max_clusters;
	};

	class OreGen {
	public:
		static void add(const OreInfo& oi)
		{
			ores.push_back(oi);
		}

    static void begin_deposit(gendata_t* gdata);

    static const OreInfo& get(int i)
		{
			return ores.at(i);
		}
		static std::size_t size()
		{
			return ores.size();
		}

		static void deposit(gendata_t*, const OreInfo&, int x, int y, int z);

    static void init();
    OreGen();
	private:
		static std::vector<OreInfo> ores;
	};
}
