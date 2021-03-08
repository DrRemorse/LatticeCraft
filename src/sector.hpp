#ifndef SECTOR_HPP
#define SECTOR_HPP

#include <common.hpp>
#include <sectorblock.hpp>
#include "flatland.hpp"
#include <memory>
#include <unordered_map>

namespace cppcraft
{
	class Sector
	{
	public:
		// sector size constants
		static const int BLOCKS_XZ = cppcraft::BLOCKS_XZ;
		static const int BLOCKS_Y  = cppcraft::BLOCKS_Y;

		static const int GENERATED  = 0x1;
		static const int GENERATING = 0x2;
    static const int MINIMAP    = 0x8;

		struct sectordata_t
		{
			sectordata_t() {}

			void assign(int bx, int by, int bz, void* data)
			{
				uint16_t index = Sector::to_block_index(bx, by, bz);
				this->data[index] = data;
			}
			void* get(int bx, int by, int bz)
			{
				int index = Sector::to_block_index(bx, by, bz);
				if (data.find(index) != data.end())
					return data[index];
				return nullptr;
			}
		private:
			std::unordered_map<uint16_t, void*> data;
		};

		// creates a sector with location (x, z)
		Sector(int xx, int zz) : x(xx), z(zz)
    {
      m_blocks =  std::make_unique<sectorblock_t> ();
    }

		// returns the local coordinates for this sector X and Z
		int getX() const noexcept {
			return this->x;
		}
		int getZ() const noexcept {
			return this->z;
		}
		// returns the world absolute coordinates for this sector X and Z
		int getWX() const;
		int getWZ() const;

		// returns true if the sector has been assigned blocks
		bool hasBlocks() const noexcept
		{
			return m_blocks != nullptr;
		}
    bool hasLight(int y) const noexcept { return m_blocks->getLight(y); }
    int getHighestLightPoint() const { return m_blocks->highest_light_y; }
    int getLightCount() const noexcept {
      return m_blocks->light_count;
    }

		// fill sector with air
		void clear();

		bool isUpdatingMesh() const noexcept {
			return this->meshgen;
		}
		// returns true if the sector is surrounded by sectors
		// that are already properly generated, or on an edge
		bool isReadyForAtmos() const;

		// update relevant parts of this sectors mesh
		void updateAllMeshes();

		// returns reference to a Block at (x, y, z)
		const Block& operator() (int x, int y, int z) const
		{
			return m_blocks->operator()(x, y, z);
		}
		Block& operator() (int x, int y, int z)
		{
			return m_blocks->operator()(x, y, z);
		}
		// returns a reference to the special section, if one exists
		// otherwise, GOD HELP US ALL
		sectordata_t& data()
		{
      assert(datasect != nullptr);
			return *datasect;
		}
		// returns a reference to the flatland container, if one exists
		// otherwise, GOD HELP US ALL
		const Flatland& flat() const {
			return m_flat;
		}
		Flatland& flat() {
			return m_flat;
		}

		// distance to another sector (in block units)
		float distanceTo(const Sector& sector, int bx, int bz) const;

		sectorblock_t& getBlocks()
		{
      assert(m_blocks != nullptr);
			return *m_blocks;
		}
		void assignBlocks(std::unique_ptr<sectorblock_t> blocks)
		{
			this->m_blocks = std::move(blocks); // in with the new
		}

		std::string to_string() const
		{
			char buffer[32];
			int len = snprintf(buffer, 32, "(x=%d, z=%d)", this->getX(), this->getZ());
			return std::string(buffer, len);
		}

		static uint16_t to_block_index(int bx, int by, int bz)
		{
			return (bx * BLOCKS_XZ + bz) * BLOCKS_XZ + by;
		}

    // make this sector use Generator (delayed)
    void regenerate();

		bool generated() const noexcept
		{
			return has_flag(GENERATED);
		}
		bool generating() const noexcept
		{
			return has_flag(GENERATING);
		}
    void add_genflag(const int flag) noexcept {
      gen_flags |= flag;
    }
    void rem_genflag(const int flag) noexcept {
      gen_flags &= ~flag;
    }
    bool has_flag(const int flag) const noexcept {
      return gen_flags & flag;
    }

	private:
		// blocks
		std::unique_ptr<sectorblock_t> m_blocks = nullptr;
		// data section
		std::unique_ptr<sectordata_t> datasect = nullptr;
		// 2d data (just a container!)
		Flatland m_flat;

    friend class Sectors;
		friend class Seamstress;
		friend class Compressor;
	public:
		// various flags for generator, minimap etc.
		uint8_t gen_flags = 0;
		// non-zero when objects are scheduled directly on this sector
		uint8_t objects = 0;
    // 8 bits to signify which parts of sector needs update
		// when an update is needed,
		bool meshgen = false;

		// we flooded this with light, or it needs flooding if the player looks at it?
		bool atmospherics = false;

  private:
		// grid position
		int x, z;
	};
}

#endif
