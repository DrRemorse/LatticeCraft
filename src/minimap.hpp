#ifndef MINIMAP_HPP
#define MINIMAP_HPP

#include <glm/mat4x4.hpp>
#include <glm/vec2.hpp>
#include "sector.hpp"

namespace library
{
	class Bitmap;
	class Texture;
}

namespace cppcraft
{
	class Sector;

	class Minimap
	{
	public:
		Minimap();
		void init();
		void update(double px, double pz);
		void render(glm::mat4& mvp);

    // update minimap now
		void addSector(Sector& s);
		void roll(int x, int z) noexcept;
    void setUpdated() noexcept;

    // delay-update minimap for this sector
    void sched(Sector& sector) {
      sector.add_genflag(Sector::MINIMAP);
    }

	private:
		library::Bitmap*  bitmap;
		library::Texture* texture;

    glm::vec2 render_offset;

    bool needs_update;
    glm::ivec2 offset;
	};
	extern Minimap minimap;
}
#endif
