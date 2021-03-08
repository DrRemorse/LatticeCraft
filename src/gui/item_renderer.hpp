#ifndef GUI_ITEM_RENDERER
#define GUI_ITEM_RENDERER

#include <library/opengl/texture.hpp>
#include <library/opengl/vao.hpp>
#include "../items/item.hpp"
#include <cstdint>
#include <map>
#include <vector>
#include <glm/vec2.hpp>
#include <glm/mat4x4.hpp>

namespace gui
{
	using cppcraft::Item;

	class ItemRenderer {
	public:
		struct ivertex_t
		{
			float x, y, z;
			float u, v, w;
			uint32_t color;

			ivertex_t (float X, float Y, float Z, float U, float V, float W, uint32_t C)
        : x(X), y(Y), z(Z), u(U), v(V), w(W), color(C) {}
      ivertex_t(const ivertex_t&) = default;
		};

		static void init();
    void begin();
		int  emit(const Item&, glm::vec2 pos, glm::vec2 size);
		void upload();
		void render(glm::vec2 scale, glm::vec2 offset);

	private:
    struct info_t {
      std::vector<ivertex_t> tiles;
      uint32_t offset;
    };
		library::VAO vao;
    std::map<uint32_t, info_t> db;

    auto& get(uint32_t tex) {
      return db[tex];
    }

		int emitQuad(const Item&, glm::vec2 pos, glm::vec2 size);
		int emitTallQuad(const Item&, glm::vec2 pos, glm::vec2 size);
		int emitBlock(const Item&, glm::vec2 pos, glm::vec2 size);
	};
}

#endif
