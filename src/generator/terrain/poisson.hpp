#pragma once

#include <array>
#include <vector>
#include <cstdint>
#include <cstdio>
#include <glm/vec2.hpp>

namespace terragen
{
  struct Poisson {
    typedef std::vector<glm::vec2> points_t;
    static points_t generate(int w, int h, float min_dist, uint32_t total);

    static void init();
  };

  struct FastPlacement
  {
    FastPlacement(size_t size, float rad, const int total);

    bool test(int x, int z) const noexcept {
      return get(x, z);
    }

  private:
    bool get(int x, int z) const noexcept {
      return m_array.at((x & (m_size-1)) * m_size + (z & (m_size-1)));
    }
    void set(int x, int z, bool v) {
      m_array.at((z & (m_size-1)) * m_size + (x & (m_size-1))) = v;
    }
    std::vector<bool> m_array;
    const std::size_t m_size;
  };
}
