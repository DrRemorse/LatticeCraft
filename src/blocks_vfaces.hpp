#include "blocks_bordered.hpp"
#include "sectors.hpp"

namespace cppcraft
{
	// returns visible sides of this cube, relying heavily on the provided testdata
	// which contains the 'current' sector which this block belongs to,
	// and all the neighboring sectors
  inline uint16_t
	Block::visibleFaces(bordered_sector_t& bsb, int bx, int by, int bz) const
	{
		uint16_t lbx = 0;

		// left side -x
		lbx |= visibilityComp(bsb(bx-1, by, bz), 32);

		// right side +x
		lbx |= visibilityComp(bsb(bx+1, by, bz), 16);

		// bottom -y
		if (LIKELY(by > 0))
		  lbx |= visibilityComp(bsb(bx, by-1, bz), 8);

		// top +y
		if (LIKELY(by < BLOCKS_Y-1))
			lbx |= visibilityComp(bsb(bx, by+1, bz), 4);
		else
			lbx |= 4;

		// front +z
		lbx |= visibilityComp(bsb(bx, by, bz-1), 2);

		// back -z
		lbx |= visibilityComp(bsb(bx, by, bz+1), 1);
		return lbx;
	}
}
