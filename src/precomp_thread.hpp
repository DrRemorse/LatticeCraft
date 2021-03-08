#ifndef PRECOMP_THREAD_HPP
#define PRECOMP_THREAD_HPP

#define AMBIENT_OCCLUSION_GRADIENTS

#include "precompiler.hpp"
#include "precomp_thread_data.hpp"
#include "renderconst.hpp"
#include "vertex_block.hpp"
#include <memory>

namespace cppcraft
{
	class PTD;
	struct bordered_sector_t;

	class PrecompThread
	{
	public:
		PTD ptd;

		// stage 2, generating mesh
		void precompile(Precomp& pc);
		// stage 3, computing AO
		void ambientOcclusion(Precomp& pc);

		// ao gradient program, adding corner shadows to a completed mesh
		void ambientOcclusionGradients(bordered_sector_t& sector, vertex_t* datadump, int vertexCount);
		// mesh optimizers
		void optimizeMesh(Precomp& pc, int shaderline, int txsize);
		void optimizeShadedMesh(Precomp& pc, int shaderline);

		void createIndices(Precomp& pc, int verts);
	};

}

#endif
