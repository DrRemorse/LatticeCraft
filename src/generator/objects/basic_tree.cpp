#include "../object.hpp"
#include "../random.hpp"
#include "../blocks.hpp"
#include <grid_walker.hpp>
#include <library/math/toolbox.hpp>
#include "helpers.hpp"

using namespace library;

namespace terragen
{
	using cppcraft::Spiders;
	using cppcraft::Sector;

	void basic_tree(const SchedObject& obj)
	{
		const Block   trunk(db::getb("wood_brown"));
		const block_t LEAF_ID =
        (obj.data & 1) ? db::getb("leaf_colored") : db::getb("leaf_green");

		// local coordinates
		const int x = obj.x;
		const int y = obj.y;
		const int z = obj.z;

    cppcraft::GridWalker walker(x, y, z);
    assert(walker.buildable());

    if (walker.move_x(1).get().isAir() == false) return;
    if (walker.move_x(-2).get().isAir() == false) return;
    if (walker.move(1, 0, 1).get().isAir() == false) return;
    if (walker.move_z(-2).get().isAir() == false) return;

    // randomness
    const int rvalue = ihash(x, z);
    // height of tree
		int height = (rvalue & 1) ? obj.data : obj.data - 3;

    cppcraft::GridWalker trunk_walker(x, y, z);
		for (int i = 0; i < height; i++)
		{
			// overwrite with trunk (removing light)
      trunk_walker.set(trunk);
      trunk_walker.move_y(1);
		}

		const int base = height / 3;
    const int leaf_height = height * 1.5f;

		for (int dy = 0; dy < leaf_height; dy++)
		{
      const float curve = 1.0 - dy / (float)leaf_height;
			int radius;
      if (rvalue & 1)
        radius = 3 * curve;
      else
        radius = 2.5 * std::sqrt(curve);

			for (int dx = -radius; dx <= radius; dx++)
      {
        cppcraft::GridWalker walker(x + dx, y + dy + base, z - radius);
        const int axis_length = 2 * radius + 1;
  			for (int i = 0; i < axis_length; i++)
  			{
          if (walker.get().triviallyOverwriteable())
          {
            walker.set(Block(LEAF_ID));
          }
          walker.move_z(1);
  			}
      }
		}
	}

}
