#include "blockmodels.hpp"

#include "renderconst.hpp"

namespace cppcraft
{
	void BlockModels::initFences()
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

		// huge monstrous loop, creating 2 fences, one following X
		// and the other following Z axis.
		for (int model = 0; model < 2; model++)
		{
			BlockMesh bm(4 * 6 * 4);
			int index = 0;

			for (int i = 0; i < 6 * 4; i++)
			for (int vert = 0; vert < 4; vert++)
			{
				// 4 cubes, 6 sides
				int cube = i / 6;
				int side = i % 6;

				// position (xyz)
				if (model == 0)
				{
					switch (cube)
					{
					case 0:
					case 1: // pikes
						if (cube == 0)
						{
							if (cube_vertices[side][vert * 3 + 0] == 0.0) // x
								bm[index].x = VERTEX_SCALE * 0.5 / 8;
							else
								bm[index].x = VERTEX_SCALE * 2.5 / 8;
						}
						else
						{
							if (cube_vertices[side][vert * 3 + 0] == 0.0) // x
								bm[index].x = VERTEX_SCALE * 5.5 / 8;
							else
								bm[index].x = VERTEX_SCALE * 7.5 / 8;
						}
						if (cube_vertices[side][vert * 3 + 2] == 0.0) // z
							bm[index].z = VERTEX_SCALE * 3.0 / 8;
						else
							bm[index].z = VERTEX_SCALE * 5.0 / 8;

						bm[index].y = cube_vertices[side][vert * 3 + 1] * VERTEX_SCALE; // y
						break;
					case 2:
					case 3: // bands
						bm[index].x = cube_vertices[side][vert * 3 + 0] * VERTEX_SCALE; // x

						if (cube == 2)
						{
							if (cube_vertices[side][vert * 3 + 1] == 0.0) // y
								bm[index].y = VERTEX_SCALE * 1.0 / 8;
							else
								bm[index].y = VERTEX_SCALE * 3.0 / 8;
						}
						else
						{
							if (cube_vertices[side][vert * 3 + 1] == 0) // y
								bm[index].y = VERTEX_SCALE * 5.0 / 8;
							else
								bm[index].y = VERTEX_SCALE * 7.0 / 8;
						}
						if (cube_vertices[side][vert * 3 + 2] == 0.0) // z
							bm[index].z = VERTEX_SCALE * 3.5 / 8;
						else
							bm[index].z = VERTEX_SCALE * 4.5 / 8;
						break;
					}
				}
				else // model == 1
				{
					switch (cube)
					{
					case 0:
					case 1: // pikes
						if (cube_vertices[side][vert * 3 + 0] == 0) // x
							bm[index].x = VERTEX_SCALE * 3.2 / 8;
						else
							bm[index].x = VERTEX_SCALE * 4.8 / 8;

						if (cube == 0)
						{
							if (cube_vertices[side][vert * 3 + 2] == 0.0) // z
								bm[index].z = VERTEX_SCALE * 0.5 / 8;
							else
								bm[index].z = VERTEX_SCALE * 2.5 / 8;
						}
						else
						{
							if (cube_vertices[side][vert * 3 + 2] == 0.0) // z
								bm[index].z = VERTEX_SCALE * 5.5 / 8;
							else
								bm[index].z = VERTEX_SCALE * 7.5 / 8;
						}
						bm[index].y = cube_vertices[side][vert * 3 + 1] * VERTEX_SCALE; // y
						break;
					case 2:
					case 3: // bans
						if (cube_vertices[side][vert * 3 + 0] == 0.0) // x
							bm[index].x = VERTEX_SCALE * 3.5 / 8;
						else
							bm[index].x = VERTEX_SCALE * 4.5 / 8;

						bm[index].z = cube_vertices[side][vert * 3 + 2] * VERTEX_SCALE; // z

						if (cube == 2)
						{
							if (cube_vertices[side][vert * 3 + 1] == 0.0) // y
								bm[index].y = VERTEX_SCALE * 1.0 / 8;
							else
								bm[index].y = VERTEX_SCALE * 3.0 / 8;
						}
						else
						{
							if (cube_vertices[side][vert * 3 + 1] == 0.0) // y
								bm[index].y = VERTEX_SCALE * 5.0 / 8;
							else
								bm[index].y = VERTEX_SCALE * 7.0 / 8;
						}
						break;
					}

				}

				// texcoords (uvw)
				switch (cube)
				{
				case 0:
				case 1:
					if (side == 2 || side == 3)
					{
						bm[index].u = cube_texcoords[side][vert * 2 + 0] * 0.25 * VERTEX_SCALE; // u
						bm[index].v = (1.0 - cube_texcoords[side][vert * 2 + 1] * 0.25) * VERTEX_SCALE; // v
					}
					else
					{
						if (cube_texcoords[side][vert * 2 + 0] == 0.0) // u
							bm[index].u = 0.375 * VERTEX_SCALE;
						else
							bm[index].u = 0.625 * VERTEX_SCALE;

						bm[index].v = cube_texcoords[side][vert * 2 + 1] * VERTEX_SCALE; // v
					}
					break;
				case 2:
				case 3:
					if (side == 2 || side == 3)
					{
						if (model == 0)
						{
							bm[index].u = cube_texcoords[side][vert * 2 + 0] * VERTEX_SCALE; // u
							bm[index].v = (0.5 + 2/16 * cube_texcoords[side][vert * 2 + 1]) * VERTEX_SCALE; // v
						}
						else
						{
							bm[index].u = (0.5 + 2/16 * cube_texcoords[side][vert * 2 + 0]) * VERTEX_SCALE; // u
							bm[index].v = cube_texcoords[side][vert * 2 + 1] * VERTEX_SCALE; // v
						}
					}
					else
					{
						bm[index].u = cube_texcoords[side][vert * 2 + 0] * VERTEX_SCALE; // u
						if (cube_texcoords[side][vert * 2 + 1] == 0.0) // v
							bm[index].v = 0.375 * VERTEX_SCALE;
						else
							bm[index].v = 0.625 * VERTEX_SCALE;
					}
					break;
				}
				bm[index].w = 0;

				bm[index].nx = cube_normals[side][0]; // nx
				bm[index].ny = cube_normals[side][1]; // ny
				bm[index].nz = cube_normals[side][2]; // nz

				index++;

			} // vert

			fences.push_back(bm);

		} // model
	}
}
