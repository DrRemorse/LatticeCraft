#include "objectq.hpp"

#include "../sectors.hpp"
#include "../seamless.hpp"
#include "../spiders.hpp"
#include "../world.hpp"
#include <library/timing/timer.hpp>
using namespace library;

namespace terragen
{
	using cppcraft::BLOCKS_XZ;
	using cppcraft::BLOCKS_Y;
	using cppcraft::Sector;
	using cppcraft::sectors;
  static bool transitioned = true;

  void ObjectQueue::init()
  {
    cppcraft::Seamless::on_transition(
      [] {
        transitioned = true;
      });
  }

	// returns true if the sector is surrounded by sectors
	// that are already properly generated, or on an edge
	inline bool validateSector(Sector& sect, int size)
	{
		return sectors.onNxN(sect, size,
		[] (Sector& sect)
		{
			return sect.generated();
		});
	}

	ObjectQueue& ObjectQueue::get()
	{
		static ObjectQueue queue;
		return queue;
	}

	void ObjectQueue::run_internal()
	{
		if (objects.empty()) return;

		int worldX = cppcraft::world.getWX() * BLOCKS_XZ;
		int worldZ = cppcraft::world.getWZ() * BLOCKS_XZ;

    if (transitioned) {
      transitioned = false;
      objects.splice(objects.begin(), retry_objects);
    }

		for (auto it = objects.begin(); it != objects.end(); ++it)
		{
			auto& obj = *it;
      const auto& db_obj = objectDB[obj.name];

			const int sectX = (obj.x - worldX) / BLOCKS_XZ;
			const int sectZ = (obj.z - worldZ) / BLOCKS_XZ;
      const int size = db_obj.size;
      assert(size > 0);

			// we don't want to generate any objects for the edge sectors
			if (sectX >= size && sectX < sectors.getXZ()-size
			 && sectZ >= size && sectZ < sectors.getXZ()-size)
			{
				Sector& sector = sectors(sectX, sectZ);
				// this object is inside the safe zone,
				// check if it is surrounded by generated sectors
				if (validateSector(sector, size))
				{
          Timer timer;
          const int64_t placed_before = cppcraft::Spiders::total_blocks_placed();
          // convert object coordinates to local grid
          obj.x -= worldX;
          obj.z -= worldZ;
					// generate object
					db_obj.func(obj);
          // verify that object didnt spend too much time
          double time_spent = timer.getTime();
          if (time_spent > 0.01) {
            int64_t diff = cppcraft::Spiders::total_blocks_placed() - placed_before;
            printf("*** Object %s took %f seconds to generate (%ld blocks)\n",
                   obj.name.c_str(), time_spent, diff);
          }
					// ..... and remove from queue
					it = objects.erase(it);
					// reduce the object count on sector (FIXME)
					//assert (sector.objects);
					if (sector.objects) {
						sector.objects--;
            if (sector.objects == 0) {
              // now that we have completed all objects on this sector,
              // its possible that all its neighbors can be added to precompq
              sectors.onNxN(sector, 1, // 3x3
                  [] (Sector& sect) -> bool {
                    if (sect.isReadyForAtmos() && sect.isUpdatingMesh() == false)
                        sect.updateAllMeshes();
                    return true;
                  });
            } // no longer has objects
          } // has objects
				}
        return;
			}
			else if (sectX < 0 || sectX >= sectors.getXZ()
				    || sectZ < 0 || sectZ >= sectors.getXZ())
			{
				// this object is outside the grid completely, remove it:
				//printf("Removing object oob at (%d, %d)\n", sectX, sectZ);
				it = objects.erase(it);
			}
      else {
        // remove from queue and put on hold
        auto next = it; next++;
        retry_objects.splice(retry_objects.begin(), objects, it, next);
        it = next;
      }
		} // for(objects)

	} // ObjectQueue::run()

  bool ObjectQueue::contains(Sector& sector)
  {
    const int worldX = cppcraft::world.getWX() * BLOCKS_XZ;
		const int worldZ = cppcraft::world.getWZ() * BLOCKS_XZ;

    for (const auto& obj : get().objects)
    {
      const int sectX = (obj.x - worldX) / BLOCKS_XZ;
			const int sectZ = (obj.z - worldZ) / BLOCKS_XZ;
      if (sector.getX() == sectX && sector.getZ() == sectZ) return true;
    }
    for (const auto& obj : get().retry_objects)
    {
      const int sectX = (obj.x - worldX) / BLOCKS_XZ;
			const int sectZ = (obj.z - worldZ) / BLOCKS_XZ;
      if (sector.getX() == sectX && sector.getZ() == sectZ) return true;
    }
    return false;
  }

}
