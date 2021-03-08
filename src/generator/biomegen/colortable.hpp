#ifndef BIOMEGEN_COLORTABLE_HPP
#define BIOMEGEN_COLORTABLE_HPP

#include <cstdint>

namespace terragen
{
	union RGB
	{
	public:
		RGB() {}
		RGB(uint32_t v) : rgba(v) {}

		RGB(int R, int G, int B)
			: r(R), g(G), b(B), a(0) {}

		uint8_t operator[] (int i) const noexcept
		{
			return array[i];
		}
		uint8_t& operator[] (int i) noexcept
		{
			return array[i];
		}

		auto toColor() const noexcept
		{
			return rgba;
		}

		struct
		{
			uint8_t r, g, b, a;
    };
    uint8_t  array[4];
		uint32_t rgba;
	};

}

#endif
