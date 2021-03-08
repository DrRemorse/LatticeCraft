#pragma once

#include <delegate.hpp>
#include <cstdint>
#include "../block.hpp"
#include <glm/vec3.hpp>

namespace cppcraft {
  class Item;
}

namespace db
{
  using cppcraft::Item;

	class ItemData {
	public:
    int getID() const noexcept { return id; }

    typedef delegate<std::string(const Item&)> name_func_t;
    std::string getName(const Item& item) const { return m_name(item); }
    void setName(name_func_t func) { m_name = std::move(func); }

    // for items with a constant tile ID
    short getTileID() const noexcept { return m_tile_id; }
    void setTileID(short tile) noexcept { m_tile_id = tile; }

    uint32_t getDiffuseTexture() const noexcept;
    uint32_t getTonemapTexture() const noexcept;

    bool isVoxelModel() const noexcept { return m_voxel_id != -1; }
    void setVoxelModel(int id) { this->m_voxel_id = id; }
    int voxelModel() const noexcept {
      assert(isVoxelModel());
      return this->m_voxel_id;
    }

    // rotation necessary to show this item properly in player hand
    void setHandRotation(glm::vec3 rot) { m_hand_rotation = rot; }
    auto getHandRotation() const noexcept { return m_hand_rotation; }
    // translation necessary to show this item properly in player hand
    void setHandTranslation(glm::vec3 pos) { m_hand_pos = pos; }
    auto getHandTranslation() const noexcept { return m_hand_pos; }

    enum ActivationMode {
      ACT_NONE  = 0,  // wave with item
      ACT_BUILD = 1   // build something from item
    };
    void setActivation(ActivationMode mode, const std::string& result) {
      this->m_act_mode = mode;
      this->m_act_result = result;
    }
    auto getActivationMode() const noexcept { return m_act_mode; }
    const std::string& getActivationResult() const noexcept { return m_act_result; }

    ItemData(int ID) : id(ID) {}
  private:
    const int id;
    name_func_t m_name = nullptr;
    short m_tile_id = -1;
    short m_voxel_id = -1;
    glm::vec3 m_hand_rotation;
    glm::vec3 m_hand_pos;

    ActivationMode m_act_mode = ACT_NONE;
    std::string    m_act_result;
	};
}
