#include "blockmodels.hpp"

#include "renderconst.hpp"

namespace cppcraft
{
	void BlockModels::initSlopedLeafs()
	{
		float cube_vertices[6][12] =
		{
			{0.0, 0.0, 1.0,  1.0, 0.0, 1.0,  1.0, 1.0, 1.0,  0.0, 1.0, 1.0},  // +z front
			{0.0, 0.0, 0.0,  0.0, 1.0, 0.0,  1.0, 1.0, 0.0,  1.0, 0.0, 0.0},  // -z back
			{0.0, 1.0, 0.0,  0.0, 1.0, 1.0,  1.0, 1.0, 1.0,  1.0, 1.0, 0.0},  // +y top
			{0.0, 0.0, 0.0,  1.0, 0.0, 0.0,  1.0, 0.0, 1.0,  0.0, 0.0, 1.0},  // -y bottom
			{1.0, 0.0, 0.0,  1.0, 1.0, 0.0,  1.0, 1.0, 1.0,  1.0, 0.0, 1.0},  // +x right
			{0.0, 0.0, 0.0,  0.0, 0.0, 1.0,  0.0, 1.0, 1.0,  0.0, 1.0, 0.0}   // -x left
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

		for (int model = 0; model < 4; model++)
		{
			for (int face = 0; face < 6; face++)
			{
				BlockMesh bm(4);

				// iterate 4 vertices on quad
				for (int vert = 0; vert < 4; vert++)
				{
					float x = cube_vertices[face][vert * 3 + 0];
					float y = cube_vertices[face][vert * 3 + 1];
					float z = cube_vertices[face][vert * 3 + 2];

					switch (model)
					{
					case 0: // -z
						if (z == 0 && y == 1) y = 0.0;
						break;
					case 1: // +z
						if (z == 1 && y == 1) y = 0.0;
						break;
					case 2: // -x
						if (x == 0 && y == 1) y = 0.0;
						break;
					case 3: // +x
						if (x == 1 && y == 1) y = 0.0;
						break;
					}

					bm[vert].x = x * VERTEX_SCALE; // x
					bm[vert].y = y * VERTEX_SCALE; // y
					bm[vert].z = z * VERTEX_SCALE; // z
					bm[vert].face = 0;

					bm[vert].u = cube_texcoords[face][vert * 2 + 0] * VERTEX_SCALE;
					bm[vert].v = cube_texcoords[face][vert * 2 + 1] * VERTEX_SCALE;
					bm[vert].w = 0;

					// switch normals
					if (face == 2)
					{
						if (model == 0) // -z
						{
							bm[vert].nx =   0;
							bm[vert].ny =  90;
							bm[vert].nz = -90;
						}
						else if (model == 1) // +z
						{
							bm[vert].nx =  0;
							bm[vert].ny = 90;
							bm[vert].nz = 90;
						}
						else if (model == 2) // -x
						{
							bm[vert].nx = -90;
							bm[vert].ny =  90;
							bm[vert].nz =   0;
						}
						else if (model == 3) // +x
						{
							bm[vert].nx = 90;
							bm[vert].ny = 90;
							bm[vert].nz =  0;
						}
					}
					else
					{
						bm[vert].nx = cube_normals[face][0]; // nx
						bm[vert].ny = cube_normals[face][1]; // ny
						bm[vert].nz = cube_normals[face][2]; // nz
					}

				}
				// add face to sloped leaf model list
				slopedLeafs[model].push_back(bm);
			}
		}

	} // initCubes()
}
