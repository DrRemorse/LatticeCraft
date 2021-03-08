#include "textureman.hpp"

#include "library/log.hpp"
#include "library/bitmap/bitmap.hpp"
#include "library/bitmap/colortools.hpp"
#include "library/opengl/opengl.hpp"
#include "library/opengl/texture.hpp"
#include <cmath>

using namespace library;

namespace cppcraft
{
	void Textureman::generateTextures()
	{
		// generate crosshair texture
		const int chsize = 64;
		const int chalfsize = chsize / 2;
		
		Bitmap crossHair(chsize, chsize, 32);
		Bitmap::rgba8_t* color = crossHair.data();
		
		for (int y = 0; y < chsize; y++)
		for (int x = 0; x < chsize; x++)
		{
			// distance from center
			float dx = x - chalfsize;
			float dy = y - chalfsize;
			float dist = sqrt( dx*dx + dy*dy ) / chalfsize;
			if (dist > 1.0) dist = 1.0; // clamp to (0..1)
			// crosshair color ramp
			float dramp = dist * dist * 255.0;
			
			if ( (x >= chalfsize-1 && x <= chalfsize) || (y >= chalfsize-1 && y <= chalfsize) )
			{
				*color = BGRA8(dramp, dramp, dramp, 255 * (1.0 - dist));
			}
			else *color = BGRA8(dramp, dramp, dramp, 0);
			
			color ++; // next pixel
		}
		// upload texture
		textures[T_CROSSHAIR] = Texture(GL_TEXTURE_2D);
		textures[T_CROSSHAIR].create(crossHair, true, GL_REPEAT, GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR);
	}
}