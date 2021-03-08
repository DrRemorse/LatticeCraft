#include "frontend_inventory.hpp"
#include "../game.hpp"

using namespace cppcraft;

namespace gui
{
  void FrontendInventory::click(int idx, int btn, int mod)
  {
    // left click
    printf("click(): Idx %d  Button %d  Modifier %d\n", idx, btn, mod);
    if (btn == 0)
    {
      printf("Swapping held with inventory item at idx %d\n", idx);
      auto& item = this->at(idx);
      // if the held item is empty, swap places
      std::swap(item, game.gui().held_item());
      // update meshes
      this->invalidate();
      game.gui().set_held_modified();
    }
  }
}
