#include "blockmodels.hpp"

#include "renderconst.hpp"

namespace cppcraft
{
	void BlockModels::initCubes()
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
			{0.0, 0.0,  1.0, 0.0,  1.0, 1.0,  0.0, 1.0}, // front
			{0.0, 0.0,  0.0, 1.0,  1.0, 1.0,  1.0, 0.0}, // back
			{0.0, 0.0,  0.0, 1.0,  1.0, 1.0,  1.0, 0.0}, // top
			{1.0, 1.0,  0.0, 1.0,  0.0, 0.0,  1.0, 0.0}, // bottom
			{0.0, 0.0,  0.0, 1.0,  1.0, 1.0,  1.0, 0.0}, // right
			{0.0, 0.0,  1.0, 0.0,  1.0, 1.0,  0.0, 1.0}  // left
		};

		signed char cube_normals[6][3] =
		{
			{0, 0, 127}, { 0,  0, -128}, // front back
			{0, 127, 0}, { 0, -128,  0}, // top   bottom
			{127, 0, 0}, {-128,  0,  0}  // right left
		};

		/*signed char cube_tangents[6][3] =
		{
			{127, 0,  0}, {-128,  0,    0}, // front back
			{127, 0,  0}, {-128,  0,    0}, // top   bottom
			{0, 0, -128}, { 0  ,  0,  127}  // right left
		};*/

    const short VERTEX_SCALE = RenderConst::VERTEX_SCALE;

		// create sides of different types of cubes
		// cube model type
		for (int model = 0; model < MI_MODEL_COUNT; model++)
		{
			// face order: +z -z +y -y +x -x
			for (int face = 0; face < 6; face++)
			{
				// create mesh object with 4 vertices
				BlockMesh bm(4);
				SelectionMesh sm(4);

				for (int vertex = 0; vertex < 4; vertex++)
				{
					bm[vertex].x = cube_vertices[face][vertex * 3    ] * VERTEX_SCALE; // x
					bm[vertex].y = cube_vertices[face][vertex * 3 + 1] * VERTEX_SCALE; // y
					bm[vertex].z = cube_vertices[face][vertex * 3 + 2] * VERTEX_SCALE; // z

					if (model == MI_HALFBLOCK)     bm[vertex].y /= 2;
					else if (model == MI_LOWBLOCK) bm[vertex].y /= 8;
					else if (model == MI_INSET || model == MI_LEAF)
					{
            const short INSET_LEVEL = VERTEX_SCALE / 16;
            const short LEAF_LEVEL  = VERTEX_SCALE / 16;
            const auto inset = (model == MI_INSET) ? INSET_LEVEL : LEAF_LEVEL;
						if (face < 2)
						{
							if (bm[vertex].z == 0) bm[vertex].z = inset;
							if (bm[vertex].z == VERTEX_SCALE) bm[vertex].z = VERTEX_SCALE - inset;
						}
						if (face > 3)
						{
							if (bm[vertex].x == 0) bm[vertex].x = inset;
							if (bm[vertex].x == VERTEX_SCALE) bm[vertex].x = VERTEX_SCALE - inset;
						}
            if ((face == 2 || face == 3) && model == MI_LEAF)
            {
              if (bm[vertex].y == 0) bm[vertex].y = inset;
							if (bm[vertex].y == VERTEX_SCALE) bm[vertex].y = VERTEX_SCALE - inset;
            }
            if (model == MI_LEAF) {
              bm[vertex].data1 = 32;
            }
					}

					if (model != MI_LOWBLOCK && model != MI_HALFBLOCK)
					{
						// 1 to 6 (front, back, top, bottom, right, left)
						bm[vertex].face = (  (1 + face) |
							int(cube_vertices[face][vertex * 3    ] *  8) | // x
							int(cube_vertices[face][vertex * 3 + 1] * 16) | // y
							int(cube_vertices[face][vertex * 3 + 2] * 32)   // z
						);
					}
					else
					{
						if (face == 2)
						bm[vertex].face = (  (1 + face) |
							int(cube_vertices[face][vertex * 3    ] *  8) | // x
							int(16) | // y
							int(cube_vertices[face][vertex * 3 + 2] * 32) | // z
							1 * 64 // extra height
						);
						else
						bm[vertex].face = 0;
					}

					bm[vertex].nx = cube_normals[face][0]; // nx
					bm[vertex].ny = cube_normals[face][1]; // ny
					bm[vertex].nz = cube_normals[face][2]; // nz

					//bm[vertex].tx = cube_tangents[face][0]; // tx
					//bm[vertex].ty = cube_tangents[face][1]; // ty
					//bm[vertex].tz = cube_tangents[face][2]; // tz

					bm[vertex].u = cube_texcoords[face][vertex * 2    ] * VERTEX_SCALE; // u
					bm[vertex].v = cube_texcoords[face][vertex * 2 + 1] * VERTEX_SCALE; // v
					bm[vertex].w = 0; // w = tile_id

					if (face != 2 && face != 3)
					{
						if (model == MI_HALFBLOCK)
						{
							bm[vertex].v *= 0.5;
						}
						else if (model == MI_LOWBLOCK)
						{
							bm[vertex].v *= 0.125;
						}
					}

					/// create block selection quads ///
					// re-using blockmesh data
					sm[vertex].x = bm[vertex].x / (float)VERTEX_SCALE;
					sm[vertex].y = bm[vertex].y / (float)VERTEX_SCALE;
					sm[vertex].z = bm[vertex].z / (float)VERTEX_SCALE;
					// extrude to avoid z-fighting
					extrude(sm[vertex], 0.005, face);

					sm[vertex].u = cube_texcoords[face][vertex * 2  ];
					sm[vertex].v = cube_texcoords[face][vertex * 2+1];
					/// create block selection quads ///

				} // vertex

				// add mesh object to cubes model list
				cubes[model].push_back(bm);
				selectionCube[model].push_back(sm);
			} // face
		} // cube model

		// centroidal cube (24 verts)
		// in this order: +z -z +y -y +x -x
		;{
			BlockMesh bm(24);
			int index = 0;

			for (int face = 0; face < 6; face++)
			{
				for (int vertex = 0; vertex < 4; vertex++)
				{
					bm[index].x = (cube_vertices[face][vertex * 3    ] - 0.5) * VERTEX_SCALE; // x
					bm[index].y = (cube_vertices[face][vertex * 3 + 1] - 0.5) * VERTEX_SCALE; // y
					bm[index].z = (cube_vertices[face][vertex * 3 + 2] - 0.5) * VERTEX_SCALE; // z
					bm[index].face = 0;

					bm[index].nx = cube_normals[face][0]; // nx
					bm[index].ny = cube_normals[face][1]; // ny
					bm[index].nz = cube_normals[face][2]; // nz

					bm[index].u = cube_texcoords[face][vertex * 2    ] * VERTEX_SCALE; // u
					bm[index].v = cube_texcoords[face][vertex * 2 + 1] * VERTEX_SCALE; // v
					bm[index].w = 0; // w = tile_id

					index++;
				} // vertex

			} // face
			centerCube.push_back(bm);

		} // centroidal cube

	} // initCubes()
}
