#ifndef LIGHTING_HPP
#define LIGHTING_HPP
/**
 * Ray-traced lighting
 *
 *
**/

#include "common.hpp"
#include "block.hpp"

namespace cppcraft
{
	class Sector;

	class Lighting
	{
	public:
		static void init();
		static light_value_t lightValue(const Block& block);

		struct emitter_t
		{
			emitter_t(int X, int Y, int Z, char Ch, char Dir, short Lvl)
				: x(X), y(Y), z(Z), ch(Ch), dir(Dir), lvl(Lvl) {}

			int x, y, z;
			char  ch;
			char  dir;
			short lvl;
		};

		// floods an initialized column of sectors with skylight
		static void atmosphericFlood(Sector& sector);
		static void skyrayDownwards(Sector& sector, int bx, int by, int bz);
		static void torchlight(Sector& sector);
		static void floodInto(Sector*, int x, int y, int z, short ch);
		static void floodOutof(int x, int y, int z, short ch, short lvl);
		static void removeLight(const Block&, int x, int y, int z);
		static void removeSkyLight(int x, int y1, int y2, int z, char lvl);

    struct propagate_t {
      propagate_t(short CH, char D, short L) : ch(CH), dir(D), level(L) {}
      short ch;
      char  dir;
      short level;
    };
    static void propagateChannel(Sector*, int x, int y, int z, propagate_t);

    //
    static void deferredRemove(Sector& sector, int x, int y1, int y2, int z, short lv);
    //
    static void handleDeferred();

    static inline char lightPenetrate(const Block& block)
    {
  	   if (block.isAir()) return 1;
  	   return (block.isTransparent() ? 2 : 15);
    }
	};
}

#endif
