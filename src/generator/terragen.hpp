#pragma once
#include <sectorblock.hpp>
#include "../flatlands.hpp"
#include "biomegen/biome.hpp"
#include "oregen.hpp"
#include "object.hpp"
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <array>
#include <vector>

namespace cppcraft
{
	extern Block air_block;
}

namespace terragen
{
	using cppcraft::BLOCKS_XZ;
	using cppcraft::BLOCKS_Y;
  using cppcraft::CAVE_GRID2D;
  using cppcraft::sectorblock_t;
	using cppcraft::Flatland;
	using cppcraft::Block;

	struct gendata_t
	{
		auto& getWeights(int x, int z) {
			return weights.at(x * (BLOCKS_XZ+1) + z);
		}
		void setWeights(int x, int z, const Biome::tweight_t& bi) {
			getWeights(x, z) = bi;
		}

		glm::vec2 getBaseCoords2D(int x, int z) const
		{
			return glm::vec2(genx + x, genz + z);
		}
		glm::vec3 getBaseCoords3D(int x, int y, int z) const
		{
			return glm::vec3(genx + x, y / (float) (BLOCKS_Y-1), genz + z);
		}

		inline Block& getb(int x, int y, int z)
		{
			return (*sblock)(x, y, z);
		}
    inline void setLight(int y) { sblock->setLight(y); }

    // schedule object for creation
    template <typename... Args>
    void add_object(Args&&... args) {
        objects.emplace_back(std::forward<Args>(args)...);
    }
    const auto& get_objects() const {
      return objects;
    }

    gendata_t(int WX, int WZ)
			: wx(WX), wz(WZ)
		{
			// position we use to generate with:
			genx = (wx - cppcraft::World::WORLD_CENTER) * BLOCKS_XZ;
			genz = (wz - cppcraft::World::WORLD_CENTER) * BLOCKS_XZ;

			// allocate new block data to avoid a copy at the end
			sblock.reset(new sectorblock_t);
      sblock->clearLights();
			// create new flatland data, since it isnt allocated by default :(
			flatl.assign({
        Flatland::data_array_t(BLOCKS_XZ*BLOCKS_XZ),
        Flatland::cave_array_t(CAVE_GRID2D * CAVE_GRID2D)
      });
		}
		gendata_t(const gendata_t&) = delete;
		gendata_t& operator= (const gendata_t&) = delete;

		auto unassignBlocks() {
			return std::move(sblock);
		}

		/// === working set === ///
		// where the sector we are generating terrain for is located
		const int wx, wz;
		// same, but in blocks relative to the center of the world
		int genx, genz;
		/// === working set === ///

		/// === results === ///
		// ALL final results produced from terragen is in sblock and flatl
    Flatland flatl;                // 2d data, colors etc.
  private:
    std::vector<SchedObject> objects;
		std::unique_ptr<sectorblock_t> sblock = nullptr;
		/// === results === ///

    // biome weights are 17x17 because of bilinear interpolation
		std::array<Biome::tweight_t, (BLOCKS_XZ+1) * (BLOCKS_XZ+1)> weights;
	};

	class Generator
	{
	public:
		static void init();
    static void init_objects();
		static void run(gendata_t* data);
	};
}
