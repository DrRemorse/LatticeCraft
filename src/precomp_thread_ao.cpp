#include <library/log.hpp>
#include "blocks_bordered.hpp"
#include "gameconf.hpp"
#include "precomp_thread.hpp"
#include "precompiler.hpp"
#include "renderconst.hpp"
#include "spiders.hpp"
#include "tiles.hpp"

//#define TIMING
#ifdef TIMING
#include <mutex>
#include <library/timing/timer.hpp>
static std::mutex timingMutex;
#endif

using namespace library;

namespace cppcraft
{
  // the channel used for AO in a vertex
  #define vertexAO(vt)  (vt)->ao

	void PrecompThread::ambientOcclusion(Precomp& precomp)
	{
#ifdef TIMING
		timingMutex.lock();
		Timer timer;
#endif

#ifdef AMBIENT_OCCLUSION_GRADIENTS
    // ambient occlusion processing stage
    auto* datadump = precomp.datadump.data();
    auto* data_end = datadump + precomp.datadump.size();
    // flip quads that need it to avoid triangle interpolation issues
		for (auto* vt = datadump; vt < data_end; vt += 4)
		if (std::abs(vertexAO(vt+0) - vertexAO(vt+2)) >
        std::abs(vertexAO(vt+1) - vertexAO(vt+3)))
		{
			vertex_t vt0 = vt[0];
			// flip quad
			vt[0] = vt[1];
			vt[1] = vt[2];
			vt[2] = vt[3];
			vt[3] = vt0;
		}
#endif

#ifdef TIMING
		logger << Log::INFO << "AO time: " << timer.getTime() << Log::ENDL;
#endif

		/*
		// optimize repeating textures mesh
		optimizeMesh(precomp, RenderConst::TX_REPEAT, RenderConst::VERTEX_SCALE / Tiles::TILES_PER_BIG_TILE);
		// optimize normal solids
		optimizeMesh(precomp, RenderConst::TX_SOLID, RenderConst::VERTEX_SCALE);
		// optimize 2-sided textures
		optimizeMesh(precomp, RenderConst::TX_2SIDED, RenderConst::VERTEX_SCALE);
		// optimize transparent textures
		optimizeMesh(precomp, RenderConst::TX_TRANS, RenderConst::VERTEX_SCALE);
		*/
		// optimize water & lava meshes
		//optimizeShadedMesh(precomp, RenderConst::TX_WATER);
		//optimizeShadedMesh(precomp, RenderConst::TX_LAVA);

#ifdef TIMING
		logger << Log::INFO << "Optimize time: " << timer.startNewRound() << Log::ENDL;
#endif

		// recount vertices
    /*
		size_t cnt = precomp.vertices[0];
		for (int i = 1; i < RenderConst::MAX_UNIQUE_SHADERS; i++)
		{
			cnt += precomp.vertices[i];
		}

		createIndices(precomp, cnt);
		*/

#ifdef TIMING
		logger << Log::INFO << "Indexing time: " << timer.getTime() << Log::ENDL;
		timingMutex.unlock();
#endif
	}

}
