#include "biome.hpp"

#include <common.hpp>
#include <library/math/toolbox.hpp>
#include "../terrain/terrains.hpp"
#include <glm/vec2.hpp>
#include <glm/gtc/noise.hpp>
#include <Simplex.h>
#include <algorithm>
#include <cmath>

using namespace library;

namespace terragen
{
  Biome::result_t Biome::solve(glm::vec3 in_coords,
                               const float MAX_DISTANCE,
                               const Terrains& terralist)
  {
    static const float HEIGHT_STRENGTH = 4.0f;
    // suitability vector
    std::vector<terrain_value_t> values;

    // generate suitability for each terrain
    const auto& terrains = terralist.get();
    values.reserve(terrains.size());

    assert(!terrains.empty());
    for (size_t i = 0; i < terrains.size(); i++)
    {
      const auto& b = terrains[i].biome;
      float dx = b.temperature   - in_coords.x;
      float dy = b.precipitation - in_coords.y;
      float dz = b.height        - in_coords.z;
      values.emplace_back(i, sqrtf(dx*dx + dy*dy + dz*dz * HEIGHT_STRENGTH));
    }
    // sort by distance
    std::sort(values.begin(), values.end(),
      [] (auto left, auto right) {
          return left.second < right.second;
      });
    // -= pick N closest values =-
    const float closest = values[0].second;
    size_t total = 1;
    for (; total < values.size(); total++) {
      if (values[total].second - closest > MAX_DISTANCE) break;
    }
    values.resize(total);
    // first point is always factor 1
    values[0].second = 1.0f;
    float norma = 1.0f;
    for (size_t i = 1; i < total; i++) {
      float weight = (values[i].second - closest) / MAX_DISTANCE;
      weight = hermite(weight);
      values[i].second = 1.0f / (1.0f + weight * 35.0f);
      norma += values[i].second;
    }
    norma = 1.0f / norma;
    for (size_t i = 0; i < total; i++) {
      values[i].second *= norma;
      //printf("Final weight for %d: %f\n", values[i].first, values[i].second);
    }
    // downsize to acceptable size
    return values;
  }
  Biome::terrain_value_t Biome::first(
      glm::vec3 in_coords,
      const Terrains& terralist)
  {
    // suitability vector
    std::array<terrain_value_t, 32> values;

    // generate suitability for each terrain
    const auto& terrains = terralist.get();

    assert(!terrains.empty());
    for (size_t i = 0; i < terrains.size(); i++)
    {
      const auto& b = terrains[i].biome;
      float dx = b.temperature   - in_coords.x;
      float dy = b.precipitation - in_coords.y;
      values[i] = {i, sqrtf(dx*dx + dy*dy) + b.height};
    }
    // sort by distance
    std::sort(values.begin(), values.begin() + terrains.size(),
      [] (auto left, auto right) {
          return left.second < right.second;
      });
    return values.front();
  }

	glm::vec3 Biome::overworldGen(const glm::vec2 pos)
	{
		const float climateBias = 1.0f; // <1.0 more warm, >1.0 more cold

		float b1 = 0.5f + 0.45f*glm::simplex(pos*0.3f) + 0.05f*glm::simplex(pos*3.0f);
		float b2 = 0.5f + 0.45f*glm::simplex(pos*1.0f) + 0.05f*glm::simplex(pos*7.0f) + 0.025*glm::simplex(pos*14.0f);
    float h = cppcraft::WATERLEVEL_FLT + 0.05f + 0.2f*glm::simplex(pos*2.12f);

		b1 = powf(b1, climateBias);
		b1 /= 0.95f; // b1 only reaches 0.95
    b1 = glm::clamp(b1, 0.0f, 1.0f);  // temperature
    b2 = glm::clamp(b2, 0.0f, 1.0f); // precipitation

    return {b1, b2, h};
	}
  glm::vec3 Biome::underworldGen(const glm::vec3 p)
	{
    const glm::vec3 npos(p.x * 1.62f, p.y * 6.6f, p.z * 1.63f);
		float b1 = 0.5f + 0.5f * glm::simplex(npos);
    return {b1, p.y, 0.0f};
	}
}
