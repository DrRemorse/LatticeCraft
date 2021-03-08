#pragma once

#include "player_inputs.hpp"
#include "script/script.hpp"
#include "modification.hpp"
#include "gui/gui.hpp"
#include <vector>

namespace cppcraft
{
  struct Game
  {
    auto& input() {
      return m_input;
    }
    const auto& input() const {
      return m_input;
    }

    Script& script() noexcept {
      return m_script;
    }

    gui::GUI& gui() noexcept {
      return m_gui;
    }
    const gui::GUI& gui() const noexcept {
      return m_gui;
    }

    auto& mods() {
      return m_mods;
    }
    inline void add_mod(std::string name);

    void terminate() {
      this->m_terminate = true;
    }
    bool is_terminating() const noexcept {
      return this->m_terminate;
    }

  private:
    Script   m_script;
    Input    m_input;
    gui::GUI m_gui;
    std::vector<Modification> m_mods;
    bool   m_terminate = false;
  };
  extern Game game;

  inline void Game::add_mod(std::string name)
  {
    m_mods.emplace_back(std::move(name));
  }
}
