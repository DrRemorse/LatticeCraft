#include "blockmodels.hpp"

#include "renderconst.hpp"

namespace cppcraft
{
	void BlockModels::initLadders()
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
			BlockMesh bm(4);

			for (int vert = 0; vert < 4; vert++)
			{
				int side = model;
				if (model >= 2) side = 2 + model;

				const float lextr = 0.95;

				switch (model)
				{
				case 0:
					bm[vert].x = (cube_vertices[side][vert * 3 +0]) * VERTEX_SCALE; // x
					bm[vert].z = (cube_vertices[side][vert * 3 +2] - lextr) * VERTEX_SCALE; // z
					break;
				case 1:
					bm[vert].x = (cube_vertices[side][vert * 3 +0]) * VERTEX_SCALE; // x
					bm[vert].z = (cube_vertices[side][vert * 3 +2] + lextr) * VERTEX_SCALE; // z
					break;
				case 2:
					bm[vert].x = (cube_vertices[side][vert * 3 +0] - lextr) * VERTEX_SCALE; // x
					bm[vert].z = (cube_vertices[side][vert * 3 +2]) * VERTEX_SCALE; // z
					break;
				case 3:
					bm[vert].x = (cube_vertices[side][vert * 3 +0] + lextr) * VERTEX_SCALE; // x
					bm[vert].z = (cube_vertices[side][vert * 3 +2]) * VERTEX_SCALE; // z
					break;
				}
				bm[vert].y = cube_vertices[side][vert * 3 + 1] * VERTEX_SCALE; // y
				bm[vert].face = 0;

				bm[vert].u = cube_texcoords[side][vert * 2 + 0] * VERTEX_SCALE; // u
				bm[vert].v = cube_texcoords[side][vert * 2 + 1] * VERTEX_SCALE; // v
				bm[vert].w = 0;

				bm[vert].nx = cube_normals[side][0];
				bm[vert].ny = cube_normals[side][1];
				bm[vert].nz = cube_normals[side][2];
			}

			blockmodels.ladders.push_back(bm);
		}

		// ladder selection boxes
		for (int model = 0; model < 4; model++)
		{
			int index = 0;
			SelectionMesh sm(24);

			for (int face = 0; face < 6; face++)
			for (int vert = 0; vert < 4; vert++)
			{
				sm[index].x = cube_vertices[face][vert*3 + 0];
				sm[index].y = cube_vertices[face][vert*3 + 1];
				sm[index].z = cube_vertices[face][vert*3 + 2];

				const float EXTRUSION = 0.12;
				const float POLYGON_OFFSET = 0.005;

				switch (model)
				{
				case 0:
					sm[index].z *= EXTRUSION; // -z
					sm[index].z += POLYGON_OFFSET;
					break;
				case 1:
					sm[index].z = (1.0 - EXTRUSION) + sm[index].z * EXTRUSION; // +z
					sm[index].z -= POLYGON_OFFSET;
					break;
				case 2:
					sm[index].x *= EXTRUSION; // -x
					sm[index].x += POLYGON_OFFSET;
					break;
				case 3:
					sm[index].x = (1.0 - EXTRUSION) + sm[index].x * EXTRUSION; // +x
					sm[index].x -= POLYGON_OFFSET;
					break;
				}

				sm[index].u = cube_texcoords[face][vert * 2 + 0]; // u
				sm[index].v = cube_texcoords[face][vert * 2 + 1]; // v

				index++;
			}

			selectionLadder.push_back(sm);
		}
	}


}
