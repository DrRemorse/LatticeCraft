#include "textureman.hpp"

#include <library/log.hpp>
#include <library/config.hpp>
#include <library/bitmap/bitmap.hpp>
#include <library/opengl/opengl.hpp>
#include "gameconf.hpp"
#include "render_fs.hpp"
#include "sun.hpp"
#include "tiles.hpp"
#include "voxelmodels.hpp"
#include <stdexcept>

using namespace library;

namespace cppcraft
{
	Textureman textureman;

	void Textureman::init(Renderer& renderer)
	{
		// create all textures
		logger << Log::INFO << "* Loading & processing textures" << Log::ENDL;

    tiledb.tiles.create_textures();
    tiledb.bigtiles.create_textures();
    tiledb.items.create_textures();
    tiledb.particles.create_textures();

    Bitmap bmp;
		/// PLAYER MODELS tileset ///
		bmp = Bitmap(config.get("textures.players", "bitmap/default/playerskins.png"), Bitmap::PNG);
		bmp.parse2D(tiledb.skinSize, tiledb.skinSize);

		textures[T_PLAYERMODELS] = Texture(GL_TEXTURE_2D_ARRAY);
		textures[T_PLAYERMODELS].create(bmp, true, GL_CLAMP_TO_EDGE, GL_NEAREST, GL_LINEAR_MIPMAP_LINEAR);
		textures[T_PLAYERMODELS].setAnisotropy(gameconf.anisotropy);

		if (OpenGL::checkError()) throw std::runtime_error("Player skins texture2d array error");

		/// PlayerSelection textures ///
		bmp = Bitmap(config.get("textures.selection", "bitmap/default/selection.png"), Bitmap::PNG);
		textures[T_SELECTION] = Texture(GL_TEXTURE_2D);
		textures[T_SELECTION].create(bmp, true, GL_CLAMP_TO_EDGE, GL_NEAREST, GL_LINEAR_MIPMAP_LINEAR);

		if (OpenGL::checkError()) throw std::runtime_error("Selection texture2d error");

		bmp = Bitmap(config.get("textures.mining", "bitmap/default/mining.png"), Bitmap::PNG);
		bmp.parse2D(tiledb.tiles.tilesize(), tiledb.tiles.tilesize());

		textures[T_MINING] = Texture(GL_TEXTURE_2D_ARRAY);
		textures[T_MINING].create(bmp, true, GL_CLAMP_TO_EDGE, GL_NEAREST, GL_LINEAR_MIPMAP_LINEAR);

		if (OpenGL::checkError()) throw std::runtime_error("Selection texture2d error");

		/// Sky renderer ///

		// sun texture
		bmp = Bitmap(config.get("textures.sun", "bitmap/default/sun.png"), Bitmap::PNG);
		textures[T_SUN] = Texture(GL_TEXTURE_2D);
		textures[T_SUN].create(bmp, true, GL_CLAMP_TO_EDGE, GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR);

		// moon texture
		bmp = Bitmap(config.get("textures.moon", "bitmap/default/moon.png"), Bitmap::PNG);
		textures[T_MOON] = Texture(GL_TEXTURE_2D, GL_COMPRESSED_RGBA);
		textures[T_MOON].create(bmp, true, GL_CLAMP_TO_EDGE, GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR);

		// stars texture
		bmp = Bitmap(config.get("textures.stars", "bitmap/default/stars.png"), Bitmap::PNG);
		textures[T_STARS] = Texture(GL_TEXTURE_2D, GL_COMPRESSED_RGBA);
		textures[T_STARS].create(bmp, true, GL_REPEAT, GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR);

		// sky cubemap
		bmp = Bitmap(config.get("textures.skybox", "bitmap/default/skybox.png"), Bitmap::PNG);
		textures[T_SKYBOX] = Texture(GL_TEXTURE_CUBE_MAP, GL_COMPRESSED_RGBA);
		textures[T_SKYBOX].create(bmp, false, GL_CLAMP_TO_EDGE, GL_LINEAR, GL_LINEAR);

		// clouds texture
		bmp = Bitmap(config.get("textures.clouds", "bitmap/default/clouds.png"), Bitmap::PNG);
		textures[T_CLOUDS] = Texture(GL_TEXTURE_2D);
		textures[T_CLOUDS].create(bmp, true, GL_REPEAT, GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR);

		// magma texture
		bmp = Bitmap(config.get("textures.magma", "bitmap/default/magma.png"), Bitmap::PNG);
		textures[T_MAGMA] = Texture(GL_TEXTURE_2D);
		textures[T_MAGMA].create(bmp, true, GL_REPEAT, GL_NEAREST, GL_LINEAR_MIPMAP_LINEAR);

		// random noise normal texture
		bmp = Bitmap(config.get("textures.noise", "bitmap/default/noise.png"), Bitmap::PNG);
		textures[T_NOISE] = Texture(GL_TEXTURE_2D);
		textures[T_NOISE].create(bmp, true, GL_REPEAT, GL_LINEAR, GL_LINEAR);

		/// FS Renderer ///

		// dirty lens texture
		bmp = Bitmap(config.get("textures.lens", "bitmap/default/lensdirt_lowc.png"), Bitmap::PNG);
		textures[T_LENSDIRT] = Texture(GL_TEXTURE_2D, GL_COMPRESSED_RGBA);
		textures[T_LENSDIRT].create(bmp, false, GL_CLAMP_TO_EDGE, GL_LINEAR, GL_LINEAR);

		/// GUI Renderer ///

		// compass
		bmp = Bitmap(config.get("textures.compass", "bitmap/default/gui/compass.png"), Bitmap::PNG);
		textures[T_COMPASS] = Texture(GL_TEXTURE_2D);
		textures[T_COMPASS].create(bmp, true, GL_CLAMP_TO_EDGE, GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR);

		// GUI composite texture
		bmp = Bitmap(config.get("textures.gui", "bitmap/default/gui/gui.1.png"), Bitmap::PNG);
		textures[T_GUI] = Texture(GL_TEXTURE_2D);
		textures[T_GUI].create(bmp, true, GL_CLAMP_TO_EDGE, GL_NEAREST, GL_NEAREST);

		// generated textures
		generateTextures();

    // create fullscreen textures
    renderer.on_resize({this, &Textureman::rebuild_buffers});
	}

  void Textureman::rebuild_buffers(Renderer& renderer)
  {
    // Lens fullscreen buffers
		for (int i = 0; i < 3; i++)
		{
			textures[(int)T_LENSFLARE + i] = Texture(GL_TEXTURE_2D, GL_RGBA16F);
			textures[(int)T_LENSFLARE + i].create(0, screenspace.flareTxW, screenspace.flareTxH);
			textures[(int)T_LENSFLARE + i].setInterpolation(true);
		}

    /// Fullscreen Buffers ///
		int  skyWidth  = renderer.width();
		int  skyHeight = renderer.height();
		bool skyLinear = false;
		// as long as multisampling is disabled, we can use lowq sky when enabled
		if (gameconf.multisampling == 0 && gameconf.highq_sky == false)
		{
			skyWidth /= 2; skyHeight /= 2;
			skyLinear = true;
		}

		// fullscreen skybuffer
		textures[T_SKYBUFFER] = Texture(GL_TEXTURE_2D, GL_RGBA8);
		textures[T_SKYBUFFER].create(0, skyWidth, skyHeight);
		textures[T_SKYBUFFER].setInterpolation(skyLinear);

		if (gameconf.multisampling)
		{
			// supersampled scene colorbuffer
			textures[T_SCENEBUFFER] = Texture(GL_TEXTURE_2D_MULTISAMPLE, GL_RGBA8);
			textures[T_SCENEBUFFER].createMultisample(renderer.width(), renderer.height(), gameconf.multisampling);
			// scene depth buffer
			textures[T_DEPTHBUFFER] = Texture(GL_TEXTURE_2D_MULTISAMPLE, GL_DEPTH_COMPONENT24);
			textures[T_DEPTHBUFFER].createDepthMultisampled(renderer.width(), renderer.height(), gameconf.multisampling);
		}
		else
		{
			int SSW = renderer.width()  * gameconf.supersampling;
			int SSH = renderer.height() * gameconf.supersampling;

			// supersampled scene colorbuffer
			textures[T_SCENEBUFFER] = Texture(GL_TEXTURE_2D, GL_RGBA8);
			textures[T_SCENEBUFFER].create(0, SSW, SSH);
			// scene depth buffer
			textures[T_DEPTHBUFFER] = Texture(GL_TEXTURE_2D, GL_DEPTH_COMPONENT24);
			textures[T_DEPTHBUFFER].createDepth(SSW, SSH);
		}

		bool underHQ = gameconf.highq_water || gameconf.multisampling > 0;
		int underW = renderer.width() / (underHQ ? 1 : 2);
		int underH = renderer.height() / (underHQ ? 1 : 2);

		// fullscreen underwater texture
		textures[T_UNDERWATERMAP] = Texture(GL_TEXTURE_2D, GL_RGBA8);
		textures[T_UNDERWATERMAP].create(0, underW, underH);
		textures[T_UNDERWATERMAP].setInterpolation(true);
		// underwater depth texture
		textures[T_UNDERWDEPTH] = Texture(GL_TEXTURE_2D, GL_DEPTH_COMPONENT24);
		textures[T_UNDERWDEPTH].createDepth(underW, underH);

		if (gameconf.reflections)
		{
			int reflWidth  = renderer.width()  / (gameconf.hq_reflections ? 1 : 2);
			int reflHeight = renderer.height() / (gameconf.hq_reflections ? 1 : 2);

			// world reflection buffer
			textures[T_REFLECTION] = Texture(GL_TEXTURE_2D, GL_RGBA8);
			textures[T_REFLECTION].create(0, reflWidth, reflHeight);
			textures[T_REFLECTION].setInterpolation(true);
		}

		// fullscreen colorbuffer
		textures[T_RENDERBUFFER] = Texture(GL_TEXTURE_2D, GL_RGBA16F);
		textures[T_RENDERBUFFER].create(0, renderer.width(), renderer.height());

		// resolved from multisampling texture
		textures[T_FINALBUFFER] = Texture(GL_TEXTURE_2D, GL_RGBA16F);
		textures[T_FINALBUFFER].create(0, renderer.width(), renderer.height());
		// resolved multisampled depth
		if (gameconf.multisampling > 1 || gameconf.supersampling > 1)
		{
			textures[T_FINALDEPTH] = Texture(GL_TEXTURE_2D, GL_DEPTH_COMPONENT24);
			textures[T_FINALDEPTH].createDepth(renderer.width(), renderer.height());
		}

		if (OpenGL::checkError()) {
      throw std::runtime_error("Fullscreen textures error");
    }
  }
}
