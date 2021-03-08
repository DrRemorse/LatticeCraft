#include "voxelmodels.hpp"

#include <library/bitmap/bitmap.hpp>
#include <library/math/vector.hpp>
#include <library/opengl/opengl.hpp>
#include <library/voxels/voxelizer.hpp>
#include <library/voxels/voxelmodel.hpp>
#include <cstring>

using namespace library;

namespace cppcraft
{
  static std::vector<library::VoxelModel> models;
  // z extrude scale 1:Z
  static const float VOXELMODELS_ZSCALE = 0.05;

	int VoxelModels::create(const Bitmap& bmp, const int tileID)
	{
    const int size = bmp.getWidth();
		// create temporary bitmap
		Bitmap tmp(size, size, 32);

		// copy tile to temporary buffer
		std::memcpy(tmp.data(), bmp.data() + tileID * size * size, size * size * 4);

		glm::vec3 offset(0.0, 0.0, 0.0);
		glm::vec3 scale(1.0 / size, 1.0 / size, VOXELMODELS_ZSCALE);
		XModel xmod;

		// create model from bitmap
		xmod.extrude(tmp, offset, scale);

		// create renderable mesh
    models.emplace_back(xmod.vertices(), xmod.data());
    return models.size()-1;
	}

	int VoxelModels::createTall(const Bitmap& bmp, int tile_bot, int tile_top)
	{
    const int size = bmp.getWidth();
		// create temporary bitmap
		Bitmap tmp(size, size * 2, 4);

    const int size2 = size * size;
		// copy tiles to temporary buffer
		std::memcpy(tmp.data() + 0,     bmp.data() + tile_bot * size2, size2 * 4);
		std::memcpy(tmp.data() + size2, bmp.data() + tile_top * size2, size2 * 4);

		glm::vec3 offset(0.0, 0.0, 0.0);
		glm::vec3 scale (1.0 / size, 1.0 / size, VOXELMODELS_ZSCALE);
		XModel xmod;

		// create model from bitmap
		xmod.extrude(tmp, offset, scale);

    // create renderable mesh
		models.emplace_back(xmod.vertices(), xmod.data());
    return models.size()-1;
	}

	void VoxelModels::render(int index)
	{
    auto& model = models.at(index);
		assert(model.isGood());
		model.render();
	}
}
