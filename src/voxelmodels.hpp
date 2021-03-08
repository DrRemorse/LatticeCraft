#ifndef VOXELMODELS_HPP
#define VOXELMODELS_HPP

#include <vector>

namespace library {
	class Bitmap;
}

namespace cppcraft
{
	class VoxelModels
	{
	public:
    static int create(const library::Bitmap& bmp, int tile);
		static int createTall(const library::Bitmap& bmp, int tileBot, int tileTop);

    static void render(int idx);
	};
}

#endif
