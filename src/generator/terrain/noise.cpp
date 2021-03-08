#include <glm/gtc/noise.hpp>

using namespace glm;

float fbm(glm::vec2 pos, int octaves=3, float lacunarity =2, float gain =0.5)
{
	float amplitude =1;
	float frequency =1;
	float sum=0;
	
	for(int i = 0; i < octaves; i++)
	{
		sum += glm::simplex(pos * frequency) * amplitude;
		amplitude *= gain;
		frequency *= lacunarity;
	}
	return sum;
}

float ridgedmultifractal(vec2 pos, float octaves, float lacunarity,
						 float h, float offset, float gain)
{
	// compute first octave
	float signal = glm::simplex(pos);
	signal = abs(signal);
	signal = offset  - signal;
	signal *= signal;
	
	float result = signal;
	
	// compute remaining octaves
	float frequency = lacunarity;
	float weight = 1;
	for (int i = 1; i < octaves; ++i)
	{
		weight = signal*gain;
		weight = clamp(weight, 0.0f, 1.0f);
		signal = glm::simplex(pos * frequency);
		signal = abs(signal);
		signal = offset - signal;
		signal *= signal;
		signal *= weight;
		
		// compute spectral weight
		float exponent = pow(pow(lacunarity , i), -h);
		// add the contribution from this octave to the result.
		result += signal * exponent;
		// update the monotonically-decreasing weight
		weight *=  signal;
		
		frequency *= lacunarity;
	}
	return result;
}
