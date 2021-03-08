#pragma once
#include "../items/item.hpp"
#include <cstdint>

namespace cppcraft
{
  // the inventory interface
  struct Inventory
  {
    using Item = cppcraft::Item;
    // insert @source into inventory, returns the count of items affected
    // eg. if inserting a stack of 64 items its possible only room for 1.
    virtual size_t insert(Item& source) = 0;

    // pull @count items from inventory as an item
    virtual Item pull(size_t count) = 0;
    // try pulling from inventory, receive what you would have pulled
    virtual Item try_pull(size_t count) = 0;
  };
  // inventory with slots
  class InventoryArray {
  public:
    InventoryArray(size_t capacity) : m_storage(capacity) {}

    // get size of inventory
    size_t size() const noexcept { return m_storage.size(); };
    // access element at @idx
    Item& at(int idx) { return m_storage.at(idx); }
    const Item& at(int idx) const { return m_storage.at(idx); }

  private:
    std::vector<Item> m_storage;
  };
}
