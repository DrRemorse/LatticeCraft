#pragma once

#include <string>
#include <vector>

namespace cppcraft
{
  class Modification
  {
  public:
    Modification(std::string);

    const std::string& name() const noexcept { return m_name; }

    const std::string& modpath() const noexcept { return m_modpath; }

    const auto& json_files() const noexcept {
      return m_json_files;
    }

  private:
    const std::string m_name;
    std::string m_modpath;
    std::vector<std::string> m_json_files;
  };
}
