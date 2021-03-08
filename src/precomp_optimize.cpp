#include "vertex_block.hpp"

#include <library/log.hpp>
#include "precompiler.hpp"
#include "precomp_thread.hpp"
#include "renderconst.hpp"

using namespace library;

namespace cppcraft
{
	inline int colorDistance(unsigned int c1, unsigned int c2)
	{
		auto* b1 = (unsigned char*) &c1;
		auto* b2 = (unsigned char*) &c2;

		int d1 = std::abs(b1[0] - b2[0]);
		int d2 = std::abs(b1[1] - b2[1]);
		int d3 = std::abs(b1[2] - b2[2]);

		return (d1 > d2) ? ((d1 > d3) ? d1 : d3) : ((d2 > d3) ? d2 : d3);
	}

	void optimizeMesh(unsigned short& verts, vertex_t* source, int txsize)
	{
		vertex_t* position = source;
		vertex_t* next     = source + 4;
		vertex_t* last     = source + verts;
		//int counter = 0;

		for (; next < last; next += 4)
		{
			bool skip = false;

			for (vertex_t* current = source; current <= position; current += 4)
			{
				// current has same texture as its next
				if (current->w == next->w)
				// same normal / face
				if (current->nx == next->nx && current->ny == next->ny && current->nz == next->nz)
				{
					// optimize +Y-faces
					if (current->ny == 127)
					{
						// next quad has previous quad position (-z)
						if (next->z + RenderConst::VERTEX_SCALE == current->z)
						if (next->x == current->x && next->y == current->y)

						// color match
						if (next[2].light == next[3].light)
						if (next[0].light == next[1].light)
						if (current[2].light == current[3].light)
						if (current[0].light == current[1].light)
						if (next[2].light == current[3].light)
						if (next[1].light == current[0].light)

						// terrain color match
						if (next[2].color == next[3].color)
						if (next[0].color == next[1].color)
						if (next[2].color == current[3].color)
						if (next[1].color == current[0].color)
						{
							// now optimize the quad, by extending the position quad,
							// and effectively removing the next quad
							// PY: (0, 0) --> (0, 1) --> (1, 1) --> (1, 0)
							// --> extend v[0] and v[3]
							current[0].z -= RenderConst::VERTEX_SCALE;
							current[3].z -= RenderConst::VERTEX_SCALE;
							// wrap texture coordinates
							current[0].v -= txsize;
							current[3].v -= txsize;

							verts -= 4;   // decrease total number of vertices
							skip = true; break;
						}
					} // Y-faces
					// optimize +X-faces
					else if (current->nx == 127)
					{
						// next quad has previous quad position (-z)
						if (next->z + RenderConst::VERTEX_SCALE == current->z)
						if (next->y == current->y && next->x == current->x)

						// color match
						if (next[0].light == next[3].light)
						if (next[1].light == next[2].light)
						if (current[0].light == current[3].light)
						if (current[1].light == current[2].light)
						if (next[0].light == current[3].light)
						if (next[1].light == current[2].light)

						// terrain color match
						if (next[0].color == next[3].color)
						if (next[1].color == next[2].color)
						if (next[0].color == current[3].color)
						if (next[1].color == current[2].color)
						{
							// now optimize the quad, by extending the position quad,
							// and effectively removing the next quad
							// {1.0, 0.0, 0.0,  1.0, 1.0, 0.0,  1.0, 1.0, 1.0,  1.0, 0.0, 1.0},  // +x right
							// {1.0, 0.0,  1.0, 1.0,  0.0, 1.0,  0.0, 0.0}, // right
							// --> extend v[0] and v[1]
							current[0].z -= RenderConst::VERTEX_SCALE;
							current[1].z -= RenderConst::VERTEX_SCALE;
							// wrap texture coordinates
							current[0].u -= txsize;
							current[1].u -= txsize;

							verts -= 4;   // decrease total number of vertices
							skip = true; break;
						}
					} // optimize +X faces

					// optimize -X faces
					else if (current->nx == -128)
					{
						// next quad has previous quad position (-z)
						if (next->z + RenderConst::VERTEX_SCALE == current->z)
						if (next->y == current->y && next->x == current->x)

						// color match
						if (next[2].light == next[3].light)
						if (next[0].light == next[1].light)
						if (current[2].light == current[3].light)
						if (current[0].light == current[1].light)
						if (next[2].light == current[3].light)
						if (next[1].light == current[0].light)

						// terrain color match
						if (next[2].color == next[3].color)
						if (next[0].color == next[1].color)
						if (next[2].color == current[3].color)
						if (next[1].color == current[0].color)
						{
							// now optimize the quad, by extending the position quad,
							// and effectively removing the next quad
							current[0].z -= RenderConst::VERTEX_SCALE;
							current[3].z -= RenderConst::VERTEX_SCALE;
							// wrap texture coordinates
							current[0].u -= txsize;
							current[3].u -= txsize;

							verts -= 4;   // decrease total number of vertices
							skip = true; break;
						}
					} // optimize -X faces
					// optimize +Z faces
					else if (current->nz == 127)
					{
						// next quad has previous quad position (-x)
						if (next->x + RenderConst::VERTEX_SCALE == current->x)
						if (next->y == current->y && next->z == current->z)

						// color match
						if (next[2].light == next[3].light)
						if (next[0].light == next[1].light)
						if (current[2].light == current[3].light)
						if (current[0].light == current[1].light)
						if (next[2].light == current[3].light)
						if (next[1].light == current[0].light)

						// terrain color match
						if (next[2].color == next[3].color)
						if (next[0].color == next[1].color)
						if (next[2].color == current[3].color)
						if (next[1].color == current[0].color)
						{
							// now optimize the quad, by extending the position quad,
							// and effectively removing the next quad
							current[0].x -= RenderConst::VERTEX_SCALE;
							current[3].x -= RenderConst::VERTEX_SCALE;
							// wrap texture coordinates
							current[0].u -= txsize;
							current[3].u -= txsize;

							verts -= 4;   // decrease total number of vertices
							skip = true; break;
						}
					} // optimize +Z faces
					// optimize -Z faces
					else if (current->nz == -128)
					{
						// next quad has previous quad position (-x)
						if (next->x + RenderConst::VERTEX_SCALE == current->x)
						if (next->y == current->y && next->z == current->z)

						// color match
						if (next[0].light == next[3].light)
						if (next[1].light == next[2].light)
						if (current[0].light == current[3].light)
						if (current[1].light == current[2].light)
						if (next[0].light == current[3].light)
						if (next[1].light == current[2].light)

						// terrain color match
						if (next[0].color == next[3].color)
						if (next[1].color == next[2].color)
						if (next[0].color == current[3].color)
						if (next[1].color == current[2].color)
						{
							// now optimize the quad, by extending the position quad,
							// and effectively removing the next quad
							current[0].x -= RenderConst::VERTEX_SCALE;
							current[1].x -= RenderConst::VERTEX_SCALE;
							// wrap texture coordinates
							current[0].u -= txsize;
							current[1].u -= txsize;

							verts -= 4;   // decrease total number of vertices
							skip = true; break;
						}
					} // optimize -Z faces

				} // same texture & same normal

			} // next face

			if (skip == false)
			{
				// just go to next position, and at the same time copy the entire
				// next quad into position
				position += 4;
				for (int i = 0; i < 4; i++) position[i] = next[i];
			}
			//else counter ++;
		}

		//if (counter)
		//logger << Log::INFO << "Optimized faces: " << counter << Log::ENDL;

	} // optimizeMesh()

	void optimizeShaderPlane(unsigned short& verts, vertex_t* source)
	{
		// origin quad for each line
		vertex_t* position = source;
		vertex_t* next     = source + 4;
		vertex_t* last     = source + verts;

		for (; next < last; next += 4)
		{
			for (vertex_t* current = source; current <= position; current += 4)
			{
				// determine that quad points upwards (+y)
				if (current->nx != 0 || current->ny != 127 || current->nz != 0)
					continue;

				// now check that the next quad has the same normal
				//if (current[0].nx != next[0].nx || current[0].ny != next[0].ny || current[0].nz != next[0].nz)
				if (next->nx != 0 || next->ny != 127 || next->nz != 0)
					continue;

				// next quad has previous quad position (-z)
				if (next->z + RenderConst::VERTEX_SCALE == current->z)
				if (next->x == current->x && next->y == current->y)
				{
					// next quad has same (consistent) color
					if (next[2].light != next[3].light) continue;
					if (next[0].light != next[1].light) continue;
					if (current[2].light != current[3].light) continue;
					if (current[0].light != current[1].light) continue;
					if (next[2].light != current[3].light) continue;
					if (next[1].light != current[0].light) continue;

					// now optimize the quad, by extending the position quad,
					// and effectively removing the next quad
					// PY: (0, 0) --> (0, 1) --> (1, 1) --> (1, 0)
					// --> extend v[0] and v[3]
					current[0].z -= RenderConst::VERTEX_SCALE;
					current[3].z -= RenderConst::VERTEX_SCALE;

					/*current[0].light = 0 << 24;
					current[1].light = 0 << 24;
					current[2].light = 0 << 24;
					current[3].light = 0 << 24;*/

					verts -= 4;   // decrease total number of vertices
					goto skipAdvancement;
				}
			}
			// go to next position, and at the same time copy the entire
			// next quad into position
			position += 4;
			for (int i = 0; i < 4; i++) position[i] = next[i];
		skipAdvancement:;
		}
	}

	void PrecompThread::optimizeMesh(Precomp& pc, int shaderline, int txsize)
	{
		unsigned short verts = pc.vertices[shaderline];
		if (verts >= 8)
		{
			vertex_t* repeat = &pc.datadump.at(pc.bufferoffset[shaderline]);
			// verts is sent by reference
			cppcraft::optimizeMesh(verts, repeat, txsize);
			// set final number of vertices
			pc.vertices[shaderline] = verts;
		}
	}

	void PrecompThread::optimizeShadedMesh(Precomp& pc, int shaderline)
	{
		static const int WATER_MAX_VERTS = Sector::BLOCKS_XZ * Sector::BLOCKS_XZ * 4;

		int verts = pc.vertices[shaderline];
		vertex_t* water = &pc.datadump.at(pc.bufferoffset[shaderline]);

		if (verts == WATER_MAX_VERTS)
		{
			const auto L = water->light;
      const auto C = water->color;

			for (int i = 1; i < WATER_MAX_VERTS; i++)
			{
				if (water[i].light != L || water[i].color != C)
            goto optimizeManuallyShaded;
			}

			water[0].x = 0;
			water[0].z = 0;

			water[1].x = 0;
			water[1].z = RenderConst::VERTEX_SCALE * Sector::BLOCKS_XZ;

			water[2].x = RenderConst::VERTEX_SCALE * Sector::BLOCKS_XZ;
			water[2].z = RenderConst::VERTEX_SCALE * Sector::BLOCKS_XZ;

			water[3].x = RenderConst::VERTEX_SCALE * Sector::BLOCKS_XZ;
			water[3].z = 0;

			// set final number of vertices
			pc.vertices[shaderline] = 4;
		}
		// try to optimize the water plane manually
		else if (verts >= 8)
		{
		optimizeManuallyShaded:
      (void) verts;
			// verts is sent by reference
			//optimizeShaderPlane(verts, water);
			// set final number of vertices
			//pc.vertices[shaderline] = verts;
		}
	}
}
