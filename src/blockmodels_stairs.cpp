#include "blockmodels.hpp"

#include "renderconst.hpp"

namespace cppcraft
{
	void BlockModels::initStairs()
	{
		const float vs_top = 0.5;

		/*float v_stair_bottom[6][12] =
		{
			{0.0, 0.0   , 1.0,  1.0, 0.0   , 1.0,  1.0, vs_top, 1.0,  0.0, vs_top, 1.0}, // front
			{0.0, 0.0   , 0.0,  0.0, vs_top, 0.0,  1.0, vs_top, 0.0,  1.0, 0.0   , 0.0}, // back
			{0.0, vs_top, 0.0,  0.0, vs_top, 1.0,  1.0, vs_top, 1.0,  1.0, vs_top, 0.0}, // top
			{0.0, 0.0   , 0.0,  1.0, 0.0   , 0.0,  1.0, 0.0   , 1.0,  0.0, 0.0   , 1.0}, // bottom
			{1.0, 0.0   , 0.0,  1.0, vs_top, 0.0,  1.0, vs_top, 1.0,  1.0, 0.0   , 1.0}, // right
			{0.0, 0.0   , 0.0,  0.0, 0.0   , 1.0,  0.0, vs_top, 1.0,  0.0, vs_top, 0.0}  // left
		};*/

		float v_stair_top[6][12] =
		{
			{0.0, vs_top, 1.0,  1.0, vs_top, 1.0,  1.0, 1.0   , 1.0,  0.0, 1.0   , 1.0}, // front
			{0.0, vs_top, 0.0,  0.0, 1.0   , 0.0,  1.0, 1.0   , 0.0,  1.0, vs_top, 0.0}, // back
			{0.0, 1.0   , 0.0,  0.0, 1.0   , 1.0,  1.0, 1.0   , 1.0,  1.0, 1.0   , 0.0}, // top
			{0.0, vs_top, 0.0,  1.0, vs_top, 0.0,  1.0, vs_top, 1.0,  0.0, vs_top, 1.0}, // bottom
			{1.0, vs_top, 0.0,  1.0, 1.0   , 0.0,  1.0, 1.0   , 1.0,  1.0, vs_top, 1.0}, // right
			{0.0, vs_top, 0.0,  0.0, vs_top, 1.0,  0.0, 1.0   , 1.0,  0.0, 1.0   , 0.0}  // left
		};

		float cube_texcoords[6][8] =
		{
			{0.0, 0.0,  1.0, 0.0,  1.0, 1.0,  0.0, 1.0},
			{1.0, 0.0,  1.0, 1.0,  0.0, 1.0,  0.0, 0.0},
			{0.0, 1.0,  0.0, 0.0,  1.0, 0.0,  1.0, 1.0},
			{1.0, 1.0,  0.0, 1.0,  0.0, 0.0,  1.0, 0.0},
			{1.0, 0.0,  1.0, 1.0,  0.0, 1.0,  0.0, 0.0},
			{0.0, 0.0,  1.0, 0.0,  1.0, 1.0,  0.0, 1.0}
		};

		signed char cube_normals[6][3] =
		{
			{0, 0, 127}, { 0,  0, -128}, // front back
			{0, 127, 0}, { 0, -128,  0}, // top   bottom
			{127, 0, 0}, {-128,  0,  0}  // right left
		};

		const short VERTEX_SCALE = RenderConst::VERTEX_SCALE;

		// huge monstrous loop, creating sides of different types of cubes
		// cube model type
		for (int model = 0; model < 8; model++)
		{
			int index = 0;
			BlockMesh bm(20);

			for (int face = 0; face < 5; face++)
			{
				int side = face;
				if (side > 2) side += 1; // ignore bottom face, which is never visible

				for (int vert = 0; vert < 4; vert++)
				{
					// 4 side models that are regular stair top parts
					// and 4 corner model stair top parts

					// position
					float x = v_stair_top[side][vert * 3 + 0];
					float y = v_stair_top[side][vert * 3 + 1];
					float z = v_stair_top[side][vert * 3 + 2];

					switch (model) {
					// sides
					case 0:
						if (z == 1.0) z = 0.5;
            break;
					case 1:
						if (z == 0.0) z = 0.5;
            break;
					case 2:
						if (x == 1.0) x = 0.5;
            break;
					case 3:
						if (x == 0.0) x = 0.5;
            break;
					// corners
					case 4:
						if (z == 1.0) z = 0.5;
						if (x == 1.0) x = 0.5;
						break;
					case 5:
						if (z == 0.0) z = 0.5;
						if (x == 1.0) x = 0.5;
						break;
					case 6:
						if (z == 1.0) z = 0.5;
						if (x == 0.0) x = 0.5;
						break;
					case 7:
						if (z == 0.0) z = 0.5;
						if (x == 0.0) x = 0.5;
						break;
					}

					bm[index].x = x * VERTEX_SCALE; // x
					bm[index].y = y * VERTEX_SCALE; // y
					bm[index].z = z * VERTEX_SCALE; // z
					bm[index].face = 0;

					// normals
					bm[index].nx = cube_normals[side][0]; // nx
					bm[index].ny = cube_normals[side][1]; // ny
					bm[index].nz = cube_normals[side][2]; // nz

					// tangents
					//bm[index].tx = cube_tangents[side][0]; // tx
					//bm[index].ty = cube_tangents[side][1]; // ty
					//bm[index].tz = cube_tangents[side][2]; // tz

					// texcoords
					bm[index].u = cube_texcoords[side][vert * 2 + 0] * VERTEX_SCALE;
					bm[index].w = 0;

					if (side != 2) // not top-face
					{
						bm[index].v = (0.5 + 0.5 * cube_texcoords[side][vert * 2 + 1]) * VERTEX_SCALE;
					}
					else // top face
					{
						if (model < 4)
						{
							// 0,1,2,3 = normal stair top
							if (model < 2)
							{
								bm[index].u = cube_texcoords[side][vert * 2 + 0] * VERTEX_SCALE;
								bm[index].v = cube_texcoords[side][vert * 2 + 1] * 0.5 * VERTEX_SCALE;
							}
							else
							{
								bm[index].u = cube_texcoords[side][vert * 2 + 0] * 0.5 * VERTEX_SCALE;
								bm[index].v = cube_texcoords[side][vert * 2 + 1] * VERTEX_SCALE;
							}
						}
						else // 4,5,6,7 = corner stair top
						{
							bm[index].u = cube_texcoords[side][vert * 2 + 0] * 0.5 * VERTEX_SCALE;
							bm[index].v = cube_texcoords[side][vert * 2 + 1] * 0.5 * VERTEX_SCALE;
						}
					}
					index++;

				} // vert
			} // face

			// add to mesh object
			stairs.push_back(bm);

		} // models

	} // initStairs()

}
