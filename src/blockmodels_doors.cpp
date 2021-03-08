#include "blockmodels.hpp"

#include "renderconst.hpp"

namespace cppcraft
{
	void BlockModels::initDoors()
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

		// huge monstrous loop, creating 8 doors, one set for closed doors
		// and the other set with opened doors
		for (int model = 0; model < 8; model++)
		{
			BlockMesh bm(6 * 4);
			int index = 0;

			for (int face = 0; face < 6; face++)
			for (int vert = 0; vert < 4; vert++)
			{
				// vertex coordinates
				bm[index].x = cube_vertices[face][vert * 3 + 0] * VERTEX_SCALE; // x
				bm[index].y = cube_vertices[face][vert * 3 + 1] * VERTEX_SCALE; // y
				bm[index].z = cube_vertices[face][vert * 3 + 2] * VERTEX_SCALE; // z
				bm[index].face = 0;

				// uvw coordinates
				bm[index].u = cube_texcoords[face][vert * 2 + 0] * VERTEX_SCALE; // u
				bm[index].v = cube_texcoords[face][vert * 2 + 1] * VERTEX_SCALE; // v
				bm[index].w = 0; // tile id

				if (face == 2 || face == 3)
				{
					bm[index].u = 0;
					bm[index].v = 0;
				}

				switch (model)
				{
				case 0: // +z
					if (bm[index].z == 0)
					{
						bm[index].z = 12.0 / 16 * VERTEX_SCALE;

						// short sides
						if (face == 4) bm[index].u =  2.5 / 16 * VERTEX_SCALE;
						if (face == 5) bm[index].u = 13.5 / 16 * VERTEX_SCALE;
					}
					break;
				case 1: // -z
					if (bm[index].z == VERTEX_SCALE)
					{
						bm[index].z = 4.0 / 16 * VERTEX_SCALE;

						// short sides
						if (face == 4) bm[index].u = 13.5 / 16 * VERTEX_SCALE;
						if (face == 5) bm[index].u =  2.5 / 16 * VERTEX_SCALE;
					}
					break;
				case 2: // +x
					if (bm[index].x == 0)
					{
						bm[index].x = 12.0 / 16 * VERTEX_SCALE;

						// short sides
						if (face == 0) bm[index].u = 13.5 / 16 * VERTEX_SCALE;
						if (face == 1) bm[index].u =  2.5 / 16 * VERTEX_SCALE;
					}
					break;
				case 3: // -x
					if (bm[index].x == VERTEX_SCALE)
					{
						bm[index].x = 4.0 / 16 * VERTEX_SCALE;

						// short sides
						if (face == 0) bm[index].u =  2.5 / 16 * VERTEX_SCALE;
						if (face == 1) bm[index].u = 13.5 / 16 * VERTEX_SCALE;
					}
					break;
				case 4: // +z --> -x
					if (bm[index].x == VERTEX_SCALE)
					{
						bm[index].x = 4.0 / 16 * VERTEX_SCALE;
					}
					else
					{
						// short sides
						if (face == 0) bm[index].u = 13.5 / 16 * VERTEX_SCALE;
						if (face == 1) bm[index].u =  2.5 / 16 * VERTEX_SCALE;
					}
					break;
				case 5: // -z --> +x
					if (bm[index].x == 0)
					{
						bm[index].x = 12.0 / 16 * VERTEX_SCALE;
					}
					else
					{
						// short sides
						if (face == 0) bm[index].u =  2.5 / 16 * VERTEX_SCALE;
						if (face == 1) bm[index].u = 13.5 / 16 * VERTEX_SCALE;
					}
					break;
				case 6: // +x --> +z
					if (bm[index].z == 0)
					{
						bm[index].z = 12.0 / 16 * VERTEX_SCALE;
					}
					break;
				case 7: // -x --> -z
					if (bm[index].z == VERTEX_SCALE)
					{
						bm[index].z = 4.0 / 16 * VERTEX_SCALE;


					}
					break;
				}

				// 0 to 5 (front, back, top, bottom, right, left)
				//    face
				//.face = (1 + i) | int(vertex(i, vert * 3 + 0) *  8) | // x
				//					int(vertex(i, vert * 3 + 1) * 16) | // y
				//					int(vertex(i, vert * 3 + 2) * 32);  // z

				bm[index].nx = cube_normals[face][0]; // nx
				bm[index].ny = cube_normals[face][1]; // ny
				bm[index].nz = cube_normals[face][2]; // nz

				index++;

			} // vert

			doors.push_back(bm);

		} // model
	}
}
