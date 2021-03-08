#include "blockmodels.hpp"

#include "renderconst.hpp"

namespace cppcraft
{
	void BlockModels::initPoles()
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
			BlockMesh bm(6 * 4);
			SelectionMesh sm(bm.size());
			int index = 0;

			for (int face = 0; face < 6; face++)
			for (int vert = 0; vert < 4; vert++)
			{
				// texcoords (u, v, w)
				if (face == 2 || face == 3)
				{
					bm[index].u = cube_texcoords[face][vert * 2 + 0] * 0.25 * VERTEX_SCALE;
					bm[index].v = (1.0 - cube_texcoords[face][vert * 2 + 0] * 0.25) * VERTEX_SCALE;
				}
				else
				{
					if (cube_texcoords[face][vert * 2 +0] == 0)
					{
						bm[index].u = 3.0 / 8 * VERTEX_SCALE;
					}
					else
					{
						bm[index].u = 5.0 / 8 * VERTEX_SCALE;
					}
					bm[index].v = cube_texcoords[face][vert * 2 +1] * VERTEX_SCALE; // v
				}
				bm[index].w = 0;

				// x
				if (cube_vertices[face][vert * 3 + 0] == 0)
					bm[index].x = 3.0 / 8 * VERTEX_SCALE;
				else
					bm[index].x = 5.0 / 8 * VERTEX_SCALE;

				// z
				if (cube_vertices[face][vert * 3 + 2] == 0)
					bm[index].z = 3.0 / 8 * VERTEX_SCALE;
				else
					bm[index].z = 5.0 / 8 * VERTEX_SCALE;

				// y
				bm[index].y = cube_vertices[face][vert * 3 + 1] * VERTEX_SCALE;
				bm[index].face = 0;

				bm[index].nx = cube_normals[face][0];
				bm[index].ny = cube_normals[face][1];
				bm[index].nz = cube_normals[face][2];

				/// pole selection box ///
				sm[index].x = bm[index].x / (float)VERTEX_SCALE;
				sm[index].y = bm[index].y / (float)VERTEX_SCALE;
				sm[index].z = bm[index].z / (float)VERTEX_SCALE;
				// extrude face to avoid z-fighting
				extrude(sm[index], 0.02, face);

				sm[index].u = cube_texcoords[face][vert*2 + 0];
				sm[index].v = cube_texcoords[face][vert*2 + 1];
				/// pole selection box ///

				index++;
			}

			poles.push_back(bm);
			selectionPole.push_back(sm);
		}

	}


}
