#include "helpers.hpp"

#include <glm/gtc/noise.hpp>
#include <glm/vec3.hpp>

using namespace glm;

namespace terragen
{
	void pillars(const vec3& p, float frequency, float& noise)
	{
		// variable radius
		float pillarRadiusCurve = 1.0 - pow(1.0 - p.y, 0.5);
		float pillarEdge = 0.75 + pillarRadiusCurve * 0.1;
		float pillarNoise = frequency + glm::simplex(p * 3.3f) * 0.1;

		// pillar islands
		if (pillarNoise > pillarEdge)
		{
			float pillarRadius = 1.0 - (1.0 - pillarNoise) / (1.0 - pillarEdge);
			float heightVariance = glm::simplex(p * 0.7f);

			// raise the pillar
			float radHeight = 0.75 + pillarRadius * 0.25;
			float height = 0.85 + heightVariance * 0.15;

			noise = p.y - radHeight * height;
		}
	}

	float barchans(double x, double y)
	{
		int ix = FASTFLOOR(x);
		int iy = FASTFLOOR(y);
		float fx = x - ix + 1;
		float fy = y - iy + 1;

		float ellip = 1.5;
		float dia = 1.4;
		float offset = 0.8 / dia;
		float dx, dy;
		float sum = 0;
		int ox, oy;

		for (oy = iy-1; oy <= iy+1; oy++)
		{
			for (ox = ix-1; ox <= ix+1; ox++)
			{
				unsigned int r = oy * 71717161 + ox;

				dx = fx - noise1u(r)*0.5;
				dy = fy - noise1u(r+1823747)*0.75;

				ellip = 1.0 + noise1s(r+823747)*0.75;
				dia = 1.4 + noise1s(r+23747)*0.3;

				float dd = dx - offset;
				float ee = dy - noise1s(r+3747)*0.1;
				float d1 = dx*dx + dy*dy*ellip;
				float d2 = dd*dd + ee*ee*ellip;

				d1 = 1 - dia * sqrtf(d1); if (d1 < 0) d1 = 0;
				d2 = 1 - dia * sqrtf(d2); if (d2 < 0) d2 = 0;

				d2 = 1 - d2*2.5; if (d2 < 0) d2 = 0;

				float d = d1;;
				if (d2 < d) d = d2;

				d = d*d*(3 - 2*d);

				float sm = sqrt(sum*sum + d*d);
				sum = 0.5*(sum + d) + 0.5 * sm;

				fx -= 1.0;
			}
			fx += 3.0;
			fy -= 1.0;
		}

		return sum;
	}

  float cracks(float x, float y, float width, float down_ratio)
	{
		int ix = FASTFLOOR(x); // first find the x block
		y += noise1u(ix); // randomize the y offset
		int iy = FASTFLOOR(y); // find the y block

		float dx = x - ix - 0.5;
		float dy = y - iy - 0.5;

		dy = 1.0f - std::abs(dy) * 2;
		dy = dy*dy*(3 - 2*dy);

		dx = std::abs(dx)*2 / width + (1-dy); if (dx > 1) dx = 1;
		dx = 1 - dx*dx;

		float d = std::sqrt(dx*dx + dy*dy);
		float max_depth = noise1u(ix + iy*40501); // crack depth

		d = -max_depth * dx * dy;
		if (d > 0) d = 0;

		if (noise1u(ix*40501 + iy) > down_ratio)
			d = -d *0.5;

		return d;
	}

}
