#include "vertex_block.hpp"
#include "vertex_selection.hpp"
#include "vertex_player.hpp"
#include <cstddef>
#include <vector>

namespace cppcraft
{
  typedef std::vector<vertex_t> BlockMesh;
  typedef std::vector<selection_vertex_t> SelectionMesh;
  typedef std::vector<player_vertex_t> PlayerMesh;

  template <class T>
  class MeshContainer {
	public:
		void push_back(const std::vector<T>& bm) {
      meshes.push_back(bm);
    }
		auto copyTo(size_t id, std::vector<T>& dest) const
    {
      dest.insert(dest.end(), meshes.at(id).begin(), meshes[id].end());
      return dest.end() - meshes[id].size();
    }
		auto copyAll(std::vector<T>& dest) const
    {
      size_t total = 0;
      for (const auto& mesh : meshes) {
          dest.insert(dest.end(), mesh.begin(), mesh.end());
          total += mesh.size();
      }
      return dest.end() - total;
    }

		int size(int meshid) const
		{
			return meshes.at(meshid).size();
		}
		int total() const noexcept
		{
			int vertices = 0;
			for (const auto& mesh : meshes)
				  vertices += mesh.size();
			return vertices;
		}
  private:
		std::vector<std::vector<T>> meshes;
	};

	class BlockModels {
	public:
		enum modid_t
		{	// cube mesh types
			MI_BLOCK = 0,
			MI_LOWBLOCK,
			MI_HALFBLOCK,
			MI_INSET,
      MI_LEAF,
			MI_MODEL_COUNT
		};

		// 6 faces * 4 verts/face = 24 vertices
		// general 6-sided block models
		// each "model" has 6 sides, in this SPECIAL element
		MeshContainer<vertex_t> cubes[MI_MODEL_COUNT];
		// sloped leafs 4 sides x 20 vertices
		MeshContainer<vertex_t> slopedLeafs[4];
		// 1. simple cross (2 crossed faces)
		// 2. simple cross with bottom face (floating flowers)
		// 3. bottom face only (simple floating flower)
		MeshContainer<vertex_t> crosses;
		// door 2 states x 4 sides x 6 faces x 4 vertices
		MeshContainer<vertex_t> doors;
		// horizontal and vertical (x, z) poles (6-sided cubes)
		// 3 poles total: Y, Z, X
		MeshContainer<vertex_t> poles;
		// ladder 4 sides x 1 face
		MeshContainer<vertex_t> ladders;
		// fence 2 directions x 4 parts x 6 faces x 4 vertices
		MeshContainer<vertex_t> fences;
		// stair meshes
		MeshContainer<vertex_t> stairs;
		// lantern meshes 6 sides x 4 vertices
		MeshContainer<vertex_t> lanterns;

		// cubes with various center-offsets, used in direct rendering
		MeshContainer<vertex_t> centerCube;
		MeshContainer<vertex_t> ctopCube; // cube offset centered top
		MeshContainer<vertex_t> cbotCube; // cube offset centered bottom

		MeshContainer<vertex_t> guiSprite; // single face used to display a sprite
		MeshContainer<vertex_t> guiDoors; // simplification of doors, rendering thread

		MeshContainer<selection_vertex_t> selectionCube[MI_MODEL_COUNT];
		MeshContainer<selection_vertex_t> selecionCross; // selection box for crosses, lanterns etc.
		MeshContainer<selection_vertex_t> selectionLadder;
		MeshContainer<selection_vertex_t> selectionPole;
		MeshContainer<selection_vertex_t> selectionStair;
		MeshContainer<selection_vertex_t> selectionDoor;

		enum skinparts_t
		{
			SP_CENTROID,
			SP_TOPCENTER,
			SP_COUNT
		};
		MeshContainer<player_vertex_t> skinCubes;

		// initialization function
		void init();

	private:
		void initCubes();
		void initSlopedLeafs();
		void initLadders();
		void initPoles();
		void initStairs();
		void initFences();
		void initDoors();
		void initLanterns();
		void initCrosses();
		void initPlayerMeshes();

		void extrude(selection_vertex_t& select, float dist, int face);
	};
	extern BlockModels blockmodels;
}
