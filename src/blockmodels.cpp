#include "blockmodels.hpp"

#include <library/log.hpp>
#include <cstdio>
using namespace library;

namespace cppcraft
{
	BlockModels blockmodels;

  // build all common mesh objects
	void BlockModels::init()
	{
    logger << Log::INFO << "* Initializing block meshes" << Log::ENDL;
		// initialize cube meshes
		initCubes();
		// initialize sloped leafs
		initSlopedLeafs();
		// initialize ladder meshes
		initLadders();
		// initialize stairs
		initStairs();
		// initialize poles (y, x, z)
		initPoles();
		// initialize fence (x, z)
		initFences();
		// initialize doors
		initDoors();
		// lantern model
		initLanterns();
		// initialize cross meshes
		initCrosses();
		// initialize playermodel meshes
		initPlayerMeshes();
	}

	void BlockModels::extrude(selection_vertex_t& select, float dist, int face)
	{
		switch (face) {
		case 0: // +z
			select.z += dist;
			select.x += (select.x < 0.5) ? -dist : dist;
			select.y += (select.y < 0.5) ? -dist : dist;
			break;
		case 1: // -z
			select.z -= dist;
			select.x += (select.x < 0.5) ? -dist : dist;
			select.y += (select.y < 0.5) ? -dist : dist;
			break;
		case 2: // +y
			select.y += dist;
			select.x += (select.x < 0.5) ? -dist : dist;
			select.z += (select.z < 0.5) ? -dist : dist;
			break;
		case 3: // -y
			select.y -= dist;
			select.x += (select.x < 0.5) ? -dist : dist;
			select.z += (select.z < 0.5) ? -dist : dist;
			break;
		case 4: // +x
			select.x += dist;
			select.y += (select.y < 0.5) ? -dist : dist;
			select.z += (select.z < 0.5) ? -dist : dist;
			break;
		case 5: // -x
			select.x -= dist;
			select.y += (select.y < 0.5) ? -dist : dist;
			select.z += (select.z < 0.5) ? -dist : dist;
			break;
		}
	}

}
