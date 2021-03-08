#include "../object.hpp"
#include "../random.hpp"
#include "../blocks.hpp"
#include "../../spiders.hpp"
#include <library/math/toolbox.hpp>
#include "helpers.hpp"
#include <cassert>

using namespace library;

namespace terragen
{
	using cppcraft::Spiders;
	using cppcraft::Sector;

	static void
	ingenLeafSphere(int gx, int gy, int gz, int wrad, int hrad, block_t mat, float stencilchance)
	{
		for (int dy = -hrad; dy <= hrad; dy++)
		{
			int rad = (1.0 - std::abs(dy) / hrad) * wrad;

			ocircleXZstencil(gx, gy+dy, gz, rad, mat, stencilchance);
		}
	}

	static void
	ingenJungleBranch(int gx, int gy, int gz, int height, float lowrad, bool coretest = false)
	{
		const float min_rad = 0.25;
		if (lowrad < min_rad) return;
		if (height < 1) return;

    const block_t LEAF_ID = db::getb("leaf_colored");
    const block_t WOOD_ID = db::getb("wood_brown");
		float toprad = min_rad;

		const float extraleaf = 4.0f;
		const float leaf_rad  = 1.7f;
		const float leaf_stencil = 1.5f;

		float startrad = -1.0f;

		float jitter = 0.8;
		float min_jitter = 0.1;

		// branch penalty
		if (coretest == false)
		{
			jitter = 2.5; min_jitter = 0.2;
			height *= 0.25;
      if (height < 1) return;
		}

		const float interpolback = 0.95;
		const float min_branch_jitter = 0.25;

		float jitter_x = randf(gx-1, gy+2, gz+1) * jitter - jitter * 0.5;
		float jitter_z = randf(gx+1, gy-2, gz-1) * jitter - jitter * 0.5;
		float x = 0.0f;
		float z = 0.0f;

		for (float y = 0; y <= height + extraleaf; y += 0.25)
		{
			float currad = ((height-y) / height);
			currad = mix(toprad, lowrad, currad * currad);

			x += jitter_x; jitter_x *= interpolback;
			z += jitter_z; jitter_z *= interpolback;

			// leafs
			if (y >= height * 0.5)
			{
				if (startrad == -1.0f)
					startrad = y; // start leafing now!

				float leafrad = (y - startrad) / ((height + extraleaf) - startrad);
				leafrad = std::sin( (leafrad * 0.75 + 0.25) * PI) * leaf_rad;

				float blobsize = 0.5 + ((height + extraleaf) - startrad) * 0.25;

				ocircleXZstencil(gx+x, gy+y, gz+z,
                         leafrad * blobsize + randf(gx+x, gy+y, gz+z) * 2.0,
								         LEAF_ID, leaf_stencil);
			}

			bool makebranch = false;

			if (y > height * 0.4)
			{
				makebranch = (std::abs(jitter_x) < min_branch_jitter
                   || std::abs(jitter_z) < min_branch_jitter);
			}

			// re-jitter
			if (std::abs(jitter_x) < min_jitter)
				jitter_x = randf(gx+x*x, gy+y, gz-z*z) * jitter - jitter * 0.5;

			if (std::abs(jitter_z) < min_jitter)
				jitter_z = randf(gx-x*x, gy+y, gz+z*x) * jitter - jitter * 0.5;

			if (makebranch && y > 0)
			{
				ingenJungleBranch(gx+x, gy+y, gz+z, (randf(gx*x, gy+y, gz*z) * 0.5 + 0.3) * (height-y), currad);
			}

			if (y <= height)
			{
				// branch trunk
				ocircleXZ(gx+x, gy+y, gz+z, currad, WOOD_ID);
			}
		}
	}

	void jungle_tree(const SchedObject& obj)
	{
		// height of tree
		const int height = obj.data;

		if (coretest(obj.x, obj.y, obj.z, 1, 1, height) == false) return;
		const int lowrad = (height / 40) * 3;

		ingenJungleBranch(obj.x, obj.y, obj.z, height, lowrad, true);
	}
}
