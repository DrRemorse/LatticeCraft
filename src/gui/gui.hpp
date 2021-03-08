#pragma once
#include "../items/item.hpp"

namespace nanogui {
  class Screen;
}
namespace cppcraft {
  class Renderer;
}

namespace gui
{
	struct GUI
  {
    using Item = cppcraft::Item;

    void takeover();
    void restore_game();
    void render();

    auto* screen() noexcept { return m_screen; }

    int  hotbar_index() const noexcept { return m_hotbar_index; }
    void hotbar_select(int idx);
    Item& hotbar_item();
    const Item& hotbar_item() const;

    Item& held_item() noexcept {
      return m_held_item;
    }
    void set_held_modified() noexcept { m_held_changed = true; }

    void init(cppcraft::Renderer&);
  private:
    // only used when in GUI mode
    Item m_held_item;
    bool m_held_changed = true;

    int m_hotbar_index = 0;

    bool show_window = false;
    static nanogui::Screen* m_screen;
  };
}
