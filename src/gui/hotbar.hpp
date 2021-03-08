#pragma once

#include "../game.hpp"
#include <nanogui/nanogui.h>
#include "frontend_inventory.hpp"

namespace gui
{
  struct hotbar_t
  {
    nanogui::ref<nanogui::Window> wnd;
    std::unique_ptr<FrontendInventory> inv = nullptr;

    void create(nanogui::Screen* m_screen)
    {
      using namespace nanogui;
      auto* gui = new FormHelper(m_screen);
      this->wnd = gui->addWindow(Eigen::Vector2i(10, 10), "");

      auto* widget = add_inv(gui, 9, 1);
      this->inv.reset(new FrontendInventory(widget));
      // fake contents
      inv->at(0) = Item(1, db::geti("wooden_pick"), Item::ITEM);
      inv->at(1) = Item(2, db::geti("stone_sword"), Item::ITEM);
      inv->at(3) = Item(3, db::geti("apple"), Item::ITEM, 4);
      inv->at(8) = Item(4, db::geti("torch"), Item::ITEM, 100);

      this->wnd->setVisible(true);
    }
    void set_position()
    {
      // center bottom
      this->wnd->setPosition({(wnd->screen()->size().x() - wnd->size().x()) / 2,
                              wnd->screen()->size().y() - wnd->size().y()});
    }
  };
}
