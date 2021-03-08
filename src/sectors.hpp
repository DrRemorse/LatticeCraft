#ifndef SECTORS_HPP
#define SECTORS_HPP

#include "sector.hpp"
#include "delegate.hpp"

namespace cppcraft
{
	class Sectors {
	public:
    Sectors(int xz);
		~Sectors() = default;

		static const int MAX_SECTORS_XZ_GRIDSIZE = 128;

		// rebuilds sector grid with @xz radi, called from World::init()
		void rebuild(int xz);
		// returns the sector axes length (double the view distance)
		int getXZ() const { return this->sectors_XZ; }

		// returns a reference to a Sector located at (x, z)
		Sector& operator() (int sx, int sz)
		{
			return *this->getSector(sx, sz);
		}
		Flatland& flatland(int sx, int sz)
		{
			return this->getSector(sx, sz)->flat();
		}

		// returns sector at position (x, z), or null
		Sector* sectorAt(float x, float z);
		// returns flatland at (x, z), or GOD HELP US ALL
		Flatland::flatland_t* flatland_at(int x, int z);

		// the rectilinear distance from origin, used in sorting by viewdistance
		// for mesh generation and terrain generation priority list
		int rectilinearDistance(Sector& sector) const
		{
			// simple rectilinear distance (aka manhattan distance)
			return std::abs(sector.getX() - sectors_XZ / 2) + std::abs(sector.getZ() - sectors_XZ / 2);
		}

		//! \brief execute lambda on a NxN square centered around @sector
		//! the result @bool is only true when all results are true
    bool onNxN(const Sector&, int size, delegate<bool(Sector&)> lambda);

		// updates all sectors, for eg. when sun-position changed
		void updateAll();
		// regenerate all sectors, for eg. teleport
		void regenerateAll();

	private:
		// returns a pointer to the sector at (x, z)
		inline Sector* getSector(int x, int z)
		{
			return this->sectors.at(x * sectors_XZ + z).get();
		}
		// returns a reference to a pointer to a sector, which is ONLY used by Seamless
		inline auto& getSectorRef(int x, int z)
		{
			return this->sectors[x * sectors_XZ + z];
		}

    // replace sector (x, z) with (x2, z2)
		inline void replace(int x, int z, int x2, int z2)
		{
      auto& dest = getSectorRef(x2, z2);
      // move to new position
      dest->x = x;
			dest->z = z;
			getSectorRef(x, z) = std::move(dest);
		}
    inline auto extract(int x, int z)
    {
      return std::move(getSectorRef(x, z));
    }
    inline void emplace(int x, int z, std::unique_ptr<Sector> sector)
		{
      sector->x = x;
			sector->z = z;
			getSectorRef(x, z) = std::move(sector);
		}

		// 3d and 2d data containers
		std::vector<std::unique_ptr<Sector>> sectors;
		// sectors XZ-axes size
		int sectors_XZ = 0;

		friend class Seamless;
	};
	extern Sectors sectors;

	// sort sectors by distance from center
	inline bool GenerationOrder(Sector* s1, Sector* s2)
	{
		const int center = sectors.getXZ() / 2;
		const int dx1 = s1->getX() - center;
		const int dz1 = s1->getZ() - center;

		const int dx2 = s2->getX() - center;
		const int dz2 = s2->getZ() - center;

		// euclidian:
		return (dx1*dx1 + dz1*dz1) < (dx2*dx2 + dz2*dz2);
		// manhattan:
		//return std::abs(dx1) + std::abs(dz1) < std::abs(dx2) + std::abs(dz2);
	}
}

#endif
