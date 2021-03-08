#include "poisson.hpp"
#undef NDEBUG
#include <cstdlib>
#include <cstdio>
#include <memory>
#include <glm/gtc/random.hpp>
#include <glm/geometric.hpp>

namespace terragen
{
  template <typename T>
  static inline void
  remove_at(std::vector<T>& v, typename std::vector<T>::size_type n)
  {
      std::swap(v[n], v.back());
      v.pop_back();
  }
  template <typename T>
  inline T delta(const T x1, const T x2, const T size) {
    auto dx = std::abs(x1 - x2);
    if (dx <= size / 2) return dx;
    return size - dx;
  }
  glm::vec2 wrap_around(glm::vec2 p, float rw, float rh) noexcept
  {
    while (p.x >= rw) p.x -= rw; while (p.x < 0.0f) p.x += rw;
    while (p.y >= rh) p.y -= rh; while (p.y < 0.0f) p.y += rh;
    return p;
  }

  struct point_t {
    glm::vec2 pos;
    bool valid = false;
  };

  struct grid_t
  {
    grid_t(int W, int H, float cellsize)
      : w(W), h(H), cell_size(cellsize), points(w * h) {
        for (auto& p : points) p.valid = false;
      }

    bool in_neighborhood(const glm::vec2 p, const float dist, float rw, float rh)
    {
      const int x = (int) (p.x / cell_size);
      const int y = (int) (p.y / cell_size);

      for (int dy = -2; dy <= 2; dy++)
  		for (int dx = -2; dx <= 2; dx++)
  		{
        auto& cell = get(x + dx, y + dy);
        const glm::vec2 wrap(delta(p.x, cell.pos.x, rw),
                             delta(p.y, cell.pos.y, rh));
				if (cell.valid && glm::length(wrap) < dist) return true;
      }
      return false;
    }

    point_t& get(int x, int y) noexcept
    {
      while (x < 0) x += w;
      while (y < 0) y += h;
      return points.at((x % w) + (y % h) * w);
    }
    void set(const glm::vec2 p) {
      const int x = (int) (p.x / cell_size);
      const int y = (int) (p.y / cell_size);
      auto& cell = get(x, y);
      //printf("Setting point(%f, %f) at grid (%d, %d) = %p\n", p.x, p.y, x, y, &cell);
      assert(cell.valid == false);
      cell = point_t{p, true};
    }

    const int w;
    const int h;
    const float cell_size;
    std::vector<point_t> points;
  };

  inline glm::vec2 point_around(const glm::vec2 origin, const float min_dist)
  {
    auto p = glm::diskRand(min_dist);
    return origin + p + glm::normalize(p) * min_dist;
  }

  Poisson::points_t Poisson::generate(
        int w, int h, const float min_dist, const uint32_t total)
  {
    const float cellSize = min_dist / sqrtf(2.0f);
    const uint32_t gridW = std::ceil(w / cellSize);
    const uint32_t gridH = std::ceil(h / cellSize);
    //printf("Grid size: %u, %u  Cell: %f\n", gridW, gridH, cellSize);
    // create grid
    grid_t grid(gridW, gridH, cellSize);
    // create first point
    auto first = glm::linearRand(glm::vec2(0.0f), glm::vec2(w, h));
    grid.set(first);

    points_t m_points;
    m_points.reserve(total);
    m_points.push_back(first);
    uint32_t iterations = 0;

    while (m_points.size() < total)
    {
      // generate new random points around already existing ones
      for (auto& origin : m_points)
      {
        const glm::vec2 p = wrap_around(point_around(origin, min_dist), w, h);
        if (! grid.in_neighborhood(p, min_dist, w, h)) {
          grid.set(p);
          m_points.push_back(p);
          break;
        }
      }
      if (++iterations >= total * 2) {
        printf("PoissonDisc: Max iterations exceeded\n");
        break;
      }
    }
    return m_points;
  }

  FastPlacement::FastPlacement(const size_t size, const float rad, const int total)
    : m_array(size * size), m_size(size)
  {
    auto points = Poisson::generate(size, size, rad, total);

    for (auto& p : points) {
      this->set(p.x, p.y, true);
    }

    /*
    for (int x = 0; x < size; x++)
    for (int z = 0; z < size; z++)
    {
      printf("%d ", get(x, z));
      if (z == size-1) printf("\n");
    }*/
  }

  void Poisson::init()
  {
    /*
    FILE* f = fopen("distribution256.data", "r");
    assert(f);
    fseek(f, 0, SEEK_END);
    int size = ftell(f);
    printf("SIZE: %d  vs ARRAY: %zu\n", size, m_array.size());
    assert(sizeof(m_array) == size);

    rewind(f);
    int res = fread(m_array.data(), 1, size, f);
    assert(res == size);
    fclose(f);
    */

  }
}
