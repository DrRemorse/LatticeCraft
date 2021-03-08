#include "precomp_thread.hpp"

#include <library/log.hpp>
#include "precompiler.hpp"
#include "vertex_block.hpp"
#include <cstring>

using namespace library;

namespace cppcraft
{
	void PrecompThread::createIndices(Precomp& precomp, int totalVertices)
	{
		(void) precomp;
		(void) totalVertices;
		// create some index bullshit
		/*
		delete[] precomp.indidump;
		precomp.indidump = new indice_t[(totalVertices / 4) * 6];
		
		indice_t* indices = precomp.indidump;
		
		for (int i = 0; i < RenderConst::MAX_UNIQUE_SHADERS; i++)
		{
			if (precomp.vertices[i] == 0)
			{
				precomp.indices[i] = 0;
				continue;
			}
			
			vertex_t* source = precomp.datadump + precomp.bufferoffset[i];
			int vertexID = 0;
			
			if (i != RenderConst::TX_2SIDED && i != RenderConst::TX_CROSS)
			{
				for (int vert = 0; vert < precomp.vertices[i]; vert += 4)
				{
					vertex_t* vertex = source + vert;
					int done = 0;
					
					// find existing vertices
					for (vertex_t* current = vertex-4; current >= source; current -= 4)
					{
						// same texture tile id
						if (current->w == vertex->w)
						// same normal
						if (current->nx == vertex->nx && current->ny == vertex->ny && current->nz == vertex->nz)
						{
							for (int v1 = 0; v1 < 4; v1++)
							{
								if (vertex[v1].w != 32767)
								for (int c1 = 0; c1 < 4; c1++)
								{
									if (vertex[c1].w != 32767)
									// same UVs
									if (current[c1].u == vertex[v1].u && current[c1].v == vertex[v1].v)
									// same position
									if (current[c1].x == vertex[v1].x && 
										current[c1].y == vertex[v1].y && 
										current[c1].z == vertex[v1].z)
									{
										// prevent vertex from being used again
										vertex[v1].w = 32767;
										// set new index to the existing one
										vertex[v1].face = current[c1].face;
										//current[c1].c = 255;
										// count up to 4, which is when its done
										if (++done == 4) goto doneFinding;
										break;
									}
								} // check all vertices against each other
							}
						}
					} // enumerate previous vertices
					
					// set regular index
					for (int j = 0; j < 4; j++)
					{
						if (vertex[j].w != 32767)
						{
							// assuming that vertices not used will be removed
							vertex[j].face = vertexID;
							// only increase vertex ID for original vertices
							vertexID++;
						}
						
					}
					
					doneFinding:
					indices[0] = vertex[0].face;
					indices[1] = vertex[1].face;
					indices[2] = vertex[2].face;
					
					indices[3] = vertex[2].face;
					indices[4] = vertex[3].face;
					indices[5] = vertex[0].face;
					indices += 6;
					
				} // enumerate all vertices
				
			}
			else
			{
				for (int vert = 0; vert < precomp.vertices[i]; vert += 4)
				{
					indices[0] = vertexID++;
					indices[1] = vertexID++;
					indices[2] = vertexID++;
					
					indices[3] = indices[2];
					indices[4] = vertexID++;
					indices[5] = indices[0];
					indices += 6;
					
				} // enumerate all vertices
			}
			
			// MUST be set, even if 0
			//int indexCount = (precomp.vertices[i] / 4) * 6;
			int indexCount = (precomp.vertices[i] / 2) * 3;
			//logger << Log::INFO << "A: " << vertexID << " vs B: " << indexCount << Log::ENDL;
			precomp.indices[i] = indexCount;
			
			// remove unused vertices
			if (precomp.vertices[i] != vertexID)
			{
				// the first 4 vertices are never removed
				vertex_t* vertex = source + 4;
				vertex_t* last = source + precomp.vertices[i];
				
				// fast-forward to first deviation
				while (vertex->w != 32767)
					vertex++;
				// copy vertices downwards
				for (vertex_t* current = vertex; current < last; current++)
				{
					vertex[0] = current[0];
					if (current[0].w != 32767) vertex++;
				}
				
				// set new vertex count
				precomp.vertices[i] = vertexID;
			}
		}
		
		// condense everything into smaller vertex array
		vertex_t* source = precomp.datadump;
		
		// find total vertices
		int total = precomp.vertices[0];
		for (int i = 1; i < RenderConst::MAX_UNIQUE_SHADERS; i++)
		{
			total += precomp.vertices[i];
		}
		// create new vertex array
		precomp.datadump = new vertex_t[total];
		
		total = 0;
		for (int i = 0; i < RenderConst::MAX_UNIQUE_SHADERS; i++)
		{
			if (precomp.vertices[i])
			{
				// copy (hopefully) smaller vertex data into new array
				memcpy(precomp.datadump + total, source + precomp.bufferoffset[i], precomp.vertices[i] * sizeof(vertex_t));
				// set new buffer offsets
				precomp.bufferoffset[i] = total;
				total += precomp.vertices[i];
			}
		}
		// remove old vertex array
		delete[] source;
		*/
	} // createIndices()
	
}
