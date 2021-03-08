#include "blockmodels.hpp"

#include "renderconst.hpp"

namespace cppcraft
{
	void BlockModels::initPlayerMeshes()
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

		float skin_texcoords[6][8] =
		{
			{0.0, 0.0,  1.0, 0.0,  1.0, 1.0,  0.0, 1.0}, // front
			{1.0, 0.0,  1.0, 1.0,  0.0, 1.0,  0.0, 0.0}, // back
			{0.0, 0.0,  0.0, 1.0,  1.0, 1.0,  1.0, 0.0}, // top
			{1.0, 1.0,  0.0, 1.0,  0.0, 0.0,  1.0, 0.0}, // bottom
			{1.0, 0.0,  1.0, 1.0,  0.0, 1.0,  0.0, 0.0}, // right
			{1.0, 0.0,  0.0, 0.0,  0.0, 1.0,  1.0, 1.0}  // left
		};

		signed char cube_normals[6][3] =
		{
			{0, 0, 127}, { 0,  0, -128}, // front back
			{0, 127, 0}, { 0, -128,  0}, // top   bottom
			{127, 0, 0}, {-128,  0,  0}  // right left
		};

		const char TEX_SCALE = 16;

		// centroidal player head
		;{
			PlayerMesh bm(24);
			int index = 0;

			for (int face = 0; face < 6; face++)
			for (int vertex = 0; vertex < 4; vertex++)
			{
				bm[index].x = (cube_vertices[face][vertex * 3    ] - 0.5) * 0.25; // x
				bm[index].y = (cube_vertices[face][vertex * 3 + 1] - 0.5) * 0.25; // y
				bm[index].z = (cube_vertices[face][vertex * 3 + 2] - 0.5) * 0.25; // z

				bm[index].nx = cube_normals[face][0]; // nx
				bm[index].ny = cube_normals[face][1]; // ny
				bm[index].nz = cube_normals[face][2]; // nz

				bm[index].u = skin_texcoords[face][vertex * 2    ] * TEX_SCALE; // u
				bm[index].v = skin_texcoords[face][vertex * 2 + 1] * TEX_SCALE; // v

				// head base tile ids
				switch (face)
				{
				case 0: // front
					bm[index].w = 0; break;
				case 1: // back
					bm[index].w = 2; break;
				case 2: // top
					bm[index].w = 3; break;
				case 3: // bottom
					bm[index].w = 11; break;
				case 4:
				case 5: // right & left
					bm[index].w = 1; break;
				}

				index++;
			}
			// head cube
			skinCubes.push_back(bm);
		}
		// chest center-bottom pivot
		;{
			PlayerMesh bm(24);
			int index = 0; // reset counter

			for (int face = 0; face < 6; face++)
			for (int vertex = 0; vertex < 4; vertex++)
			{
				bm[index].x = (cube_vertices[face][vertex * 3    ] - 0.5) * 0.40; // x
				bm[index].y = (cube_vertices[face][vertex * 3 + 1] - 0.0) * 0.55; // y
				bm[index].z = (cube_vertices[face][vertex * 3 + 2] - 0.5) * 0.18; // z

				bm[index].nx = cube_normals[face][0]; // nx
				bm[index].ny = cube_normals[face][1]; // ny
				bm[index].nz = cube_normals[face][2]; // nz

				bm[index].u = skin_texcoords[face][vertex * 2    ] * TEX_SCALE; // u
				bm[index].v = skin_texcoords[face][vertex * 2 + 1] * TEX_SCALE; // v

				// chest base tile ids
				switch (face)
				{
				case 0: // front
					bm[index].w = 4; break;
				case 1: // back
					bm[index].w = 6; break;
				case 2: // top
					bm[index].w = 7; break;
				case 3: // bottom
					bm[index].w = 6; break;
				case 4:
				case 5: // right & left
					bm[index].w = 5; break;
				}
				index++;
			}
			// chest cube
			skinCubes.push_back(bm);
		}
		// hands center-top pivot
		;{
			PlayerMesh bm(24);
			int index = 0; // reset counter

			for (int face = 0; face < 6; face++)
			for (int vertex = 0; vertex < 4; vertex++)
			{
				bm[index].x = (cube_vertices[face][vertex * 3    ] - 0.5) * 0.14; // x
				bm[index].y = (cube_vertices[face][vertex * 3 + 1] - 0.9) * 0.48; // y
				bm[index].z = (cube_vertices[face][vertex * 3 + 2] - 0.5) * 0.14; // z

				bm[index].nx = cube_normals[face][0]; // nx
				bm[index].ny = cube_normals[face][1]; // ny
				bm[index].nz = cube_normals[face][2]; // nz

				bm[index].u = skin_texcoords[face][vertex * 2    ] * TEX_SCALE; // u
				bm[index].v = skin_texcoords[face][vertex * 2 + 1] * TEX_SCALE; // v

				// hands base tile ids
				switch (face)
				{
				case 0: // front
					bm[index].w = 9; break;
				case 1: // back
					bm[index].w = 9; break;
				case 2: // top
					bm[index].w = 12; break;
				case 3: // bottom
					bm[index].w = 11; break;
				case 4:
				case 5: // right & left
					bm[index].w = 9; break;
				}
				index++;
			}
			// hands
			skinCubes.push_back(bm);
		}
		// legs center-top pivot
		;{
			PlayerMesh bm(24);
			int index = 0;

			for (int face = 0; face < 6; face++)
			for (int vertex = 0; vertex < 4; vertex++)
			{
				bm[index].x = (cube_vertices[face][vertex * 3    ] - 0.50) * 0.18; // x
				bm[index].y = (cube_vertices[face][vertex * 3 + 1] - 1.00) * 0.60; // y
				bm[index].z = (cube_vertices[face][vertex * 3 + 2] - 0.50) * 0.18; // z

				bm[index].nx = cube_normals[face][0]; // nx
				bm[index].ny = cube_normals[face][1]; // ny
				bm[index].nz = cube_normals[face][2]; // nz

				bm[index].u = skin_texcoords[face][vertex * 2    ] * TEX_SCALE; // u
				bm[index].v = skin_texcoords[face][vertex * 2 + 1] * TEX_SCALE; // v

				// legs base tile ids
				switch (face)
				{
				case 0: // front
					bm[index].w = 8; break;
				case 1: // back
					bm[index].w = 8; break;
				case 2: // top
					bm[index].w = 8; break;
				case 3: // bottom
					bm[index].w = 10; break;
				case 4:
				case 5: // right & left
					bm[index].w = 8; break;
				}
				index++;
			}
			// legs
			skinCubes.push_back(bm);
		}

	} // initPlayerMeshes()
}
