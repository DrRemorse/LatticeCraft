#include <glm/vec2.hpp>

// fractal brownian motion
extern float fbm(glm::vec2 pos, int octaves = 3, float lacunarity = 2.0f, float gain = 0.5f);
// ridged multifractal with no seed, unfortunately
extern float ridgedmultifractal(glm::vec2 pos, float octaves, float lacunarity,
								float h, float offset, float gain);
