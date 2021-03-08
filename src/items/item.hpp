#pragma once
#include "db/itemdb.hpp"
#include <string>
#include <cstdint>
// no item is always zero
#define IT_NONE  0

namespace cppcraft
{
	typedef unsigned short item_t;

  /**
   *
   *
   *
  **/
	class Item {
	public:
  	static const int BLOCK = 0;
  	static const int ITEM  = 1;

    // used as empty item slot or free hand
    Item() : m_guid(0), m_id(0), m_count(0) {}

		Item(uint32_t guid, item_t id, item_t type = BLOCK, uint16_t cnt = 1)
			: m_guid(guid), m_id(id | (type << 15)), m_count(cnt) {}

    uint32_t guid() const noexcept {
      return this->m_guid;
    }

		void setID(item_t id) noexcept {
      this->m_id &= 0x8000;
			this->m_id |= id & 0x7FFF;
		}
		item_t getID() const noexcept {
			return this->m_id & 0x7FFF;
		}

    void setType(item_t type) noexcept {
      this->m_id &= 0x7FFF;
			this->m_id |= type << 15;
		}
		item_t getType() const noexcept {
			return this->m_id >> 15;
		}

		void setCount(uint16_t count) noexcept {
			this->m_count = count;
		}
		uint16_t getCount() const noexcept {
			return this->m_count;
		}

		bool isNone() const noexcept {
			return this->getID() == 0 || this->getCount() == 0;
		}
    bool isItem() const noexcept {
      return getType() == ITEM;
    }
    bool isBlock() const noexcept {
      return getType() == BLOCK;
    }
    Block toBlock() const noexcept {
      assert(isBlock());
      return Block(m_id);
    }

    //! lookup in db for this item, depending on type
		const auto& itemdb() const {
      assert(getType() == ITEM);
		  return ::db::ItemDB::cget()[this->getID()];
		}
		auto& itemdb() {
      assert(getType() == ITEM);
			return ::db::ItemDB::get()[this->getID()];
		}

    const auto& blockdb() const {
      assert(getType() == BLOCK);
		  return ::db::BlockDB::cget()[this->getID()];
		}
		auto& blockdb() {
      assert(getType() == BLOCK);
			return ::db::BlockDB::get()[this->getID()];
		}

    uint32_t getTexture() const {
      if (isBlock()) return blockdb().getDiffuseTexture();
      return itemdb().getDiffuseTexture();
    }

    bool isVoxel() const noexcept {
      if (isItem()) return itemdb().isVoxelModel();
      return false;
    }
    int getVoxelModel() const noexcept {
      return itemdb().voxelModel();
    }

	private:
    uint32_t m_guid; // global unique ID
		item_t   m_id;
		uint16_t m_count;
	};
  static_assert(sizeof(Item) == 8, "Raw items should be 64-bits");
}
