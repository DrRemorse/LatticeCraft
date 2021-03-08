#include "precomp_thread.hpp"

#include "precomp_thread_data.hpp"
#include "blocks_vfaces.hpp"
#include <stdexcept>

namespace cppcraft
{
	void PTD::process_block(const Block& block, int bx, int by, int bz)
	{
		// check which sides of a block is visible, if needed
		uint16_t sides = block.visibleFaces(*sector, bx, by, bz);

		// must have at least one visible face to continue
		if (sides)
		{
			///////////////////////////////
			//  now, emit some vertices  //
			///////////////////////////////
			this->shader = block.db().shader;
			this->repeat_y = block.db().repeat_y;
			// get pointer, increase it by existing vertices
      //CC_ASSERT(shader >= 0 && shader < vertices.size(), "Invalid shader index");
			size_t start = vertices[shader].size();
			// emit vertices
			block.db().emit(*this, bx, by, bz, sides);

			// vertex position in 16bits
			const short vx = bx << RenderConst::VERTEX_SHL;
			const short vy = by << RenderConst::VERTEX_SHL;
			const short vz = bz << RenderConst::VERTEX_SHL;

			// move mesh object to local grid space
			for (auto it = vertices[shader].begin() + start; it != vertices[shader].end(); ++it)
			{
				it->x += vx;
				it->y += vy;
				it->z += vz;
			}
		} // if (sides)

	} // process_block()

  int16_t PTD::getConnectedTexture(int bx, int by, int bz, int face) const
  {
    connected_textures_t ct;
    // assume connected
    switch (face) {
      case 0: // +z
      case 1: // -z
        ct.blocks[0] = sector->get(bx-1, by-1, bz);
        ct.blocks[1] = sector->get(bx  , by-1, bz);
        ct.blocks[2] = sector->get(bx+1, by-1, bz);
        ct.blocks[3] = sector->get(bx-1, by  , bz);
        ct.blocks[4] = sector->get(bx  , by  , bz);
        ct.blocks[5] = sector->get(bx+1, by  , bz);
        ct.blocks[6] = sector->get(bx-1, by+1, bz);
        ct.blocks[7] = sector->get(bx  , by+1, bz);
        ct.blocks[8] = sector->get(bx+1, by+1, bz);
        break;
      case 2: // +y
      case 3: // -y
        ct.blocks[0] = sector->get(bx-1, by, bz-1);
        ct.blocks[1] = sector->get(bx  , by, bz-1);
        ct.blocks[2] = sector->get(bx+1, by, bz-1);
        ct.blocks[3] = sector->get(bx-1, by, bz);
        ct.blocks[4] = sector->get(bx  , by, bz);
        ct.blocks[5] = sector->get(bx+1, by, bz);
        ct.blocks[6] = sector->get(bx-1, by, bz+1);
        ct.blocks[7] = sector->get(bx  , by, bz+1);
        ct.blocks[8] = sector->get(bx+1, by, bz+1);
        break;
      case 4: // +x
      case 5: // -x
        ct.blocks[0] = sector->get(bx, by-1, bz-1);
        ct.blocks[1] = sector->get(bx, by  , bz-1);
        ct.blocks[2] = sector->get(bx, by+1, bz-1);
        ct.blocks[3] = sector->get(bx, by-1, bz);
        ct.blocks[4] = sector->get(bx, by  , bz);
        ct.blocks[5] = sector->get(bx, by+1, bz);
        ct.blocks[6] = sector->get(bx, by-1, bz+1);
        ct.blocks[7] = sector->get(bx, by  , bz+1);
        ct.blocks[8] = sector->get(bx, by+1, bz+1);
        break;
    default:
      throw std::out_of_range("Invalid face in PTD::getConnectedTexture");
      break;
    }
    const auto& self = ct.blocks[4].db();
    return self.connTexFunction(ct, face);
  }

}
