#include "render_gui.hpp"

#include <library/opengl/opengl.hpp>
#include <library/opengl/window.hpp>
#include "camera.hpp"
#include "chat.hpp"
#include "game.hpp"
#include "minimap.hpp"
#include "generator/objectq.hpp"
#include "player_logic.hpp"
#include "renderman.hpp"
#include "sectors.hpp"
#include "shaderman.hpp"
#include "threading.hpp"
#include <glm/vec3.hpp>
#include <library/math/matrix.hpp>
#include <nanogui/nanogui.h>

#include "drawq.hpp"
#include "player.hpp"
#include "spiders.hpp"
#include "generator.hpp"
#include "precompq.hpp"
#include "generator/terrain/terrains.hpp"
#include "sun.hpp"

using namespace library;

namespace cppcraft
{
	GUIRenderer rendergui;
  static nanogui::ref<nanogui::Window> stats;
  static nanogui::IntBox<float>* fpsbox = nullptr;
  static nanogui::IntBox<size_t>* drawbox = nullptr;
  static nanogui::IntBox<size_t>* genbox = nullptr;
  static nanogui::IntBox<size_t>* prqbox = nullptr;
  static nanogui::IntBox<size_t>* objbox = nullptr;
  static nanogui::IntBox<size_t>* objretrybox = nullptr;
  // sector //
  static nanogui::IntBox<int>* sectlts = nullptr;
  static nanogui::IntBox<int>* sectstage = nullptr;
  static nanogui::IntBox<int>* sectobjs = nullptr;
  static nanogui::IntBox<int>* sectatmos = nullptr;

  static nanogui::TextBox* trnbox = nullptr;
  static nanogui::IntBox<int>* skybox = nullptr;
  static nanogui::IntBox<int>* gndbox = nullptr;

	void GUIRenderer::init(Renderer& renderer)
	{
		width  = 1.0;
		height = 1.0 / renderer.aspect();

		// orthographic projection
		ortho = ortho2dMatrix(width, height, 0.0f, 2.0f);

		// initialize minimap
		minimap.init();

		// our esteemed graphical interfaces
		game.gui().init(renderer);

		// initialize chatbox & chat-transformation station
		chatbox.init(width, height);

    using namespace nanogui;
    auto* helper = new FormHelper(game.gui().screen());
    stats = helper->addWindow(Eigen::Vector2i(10, 10), "cppcraft v0.2");
    stats->setLayout(new GroupLayout());

    auto* main = new Widget(stats);
    main->setLayout(new BoxLayout(Orientation::Horizontal,
                     Alignment::Middle, 0, 20));

    new Label(main, "fps");
    fpsbox = new nanogui::IntBox<float>(main);
    fpsbox->setEditable(false);
    fpsbox->setFontSize(16);
    fpsbox->setFormat("[-]?[0-9]*\\.?[0-9]+");

    new Label(main, "DrawQ");
    drawbox = new nanogui::IntBox<size_t>(main);
    drawbox->setEditable(false);
    drawbox->setFixedSize(Vector2i(100, 20));

    new Label(main, "Generator");
    genbox = new nanogui::IntBox<size_t>(main);
    genbox->setEditable(false);
    genbox->setFixedSize(Vector2i(100, 20));

    new Label(main, "PrecompQ");
    prqbox = new nanogui::IntBox<size_t>(main);
    prqbox->setEditable(false);
    prqbox->setFixedSize(Vector2i(100, 20));

    new Label(main, "ObjectQ");
    objbox = new nanogui::IntBox<size_t>(main);
    objbox->setEditable(false);
    objbox->setFixedSize(Vector2i(100, 20));

    new Label(main, "RetryQ");
    objretrybox = new nanogui::IntBox<size_t>(main);
    objretrybox->setEditable(false);
    objretrybox->setFixedSize(Vector2i(100, 20));

    // sector stats
    auto* sector = new Widget(stats);
    sector->setLayout(new BoxLayout(Orientation::Horizontal,
                      Alignment::Middle, 0, 20));
    new Label(sector, "Sector");

    new Label(sector, "Terrain");
    trnbox = new nanogui::TextBox(sector);
    trnbox->setEditable(false);
    trnbox->setFixedSize(Vector2i(160, 20));

    new Label(sector, "Sky level");
    skybox = new nanogui::IntBox<int>(sector);
    skybox->setEditable(false);
    skybox->setFixedSize(Vector2i(50, 20));

    new Label(sector, "Ground level");
    gndbox = new nanogui::IntBox<int>(sector);
    gndbox->setEditable(false);
    gndbox->setFixedSize(Vector2i(50, 20));

    new Label(sector, "Lights");
    sectlts = new nanogui::IntBox<int>(sector);
    sectlts->setEditable(false);
    sectlts->setFixedSize(Vector2i(50, 20));

    new Label(sector, "Meshgen");
    sectstage = new nanogui::IntBox<int>(sector);
    sectstage->setEditable(false);
    sectstage->setFixedSize(Vector2i(25, 20));

    new Label(sector, "Objects");
    sectobjs = new nanogui::IntBox<int>(sector);
    sectobjs->setEditable(false);
    sectobjs->setFixedSize(Vector2i(25, 20));

    new Label(sector, "Atmospherics");
    sectatmos = new nanogui::IntBox<int>(sector);
    sectatmos->setEditable(false);
    sectatmos->setFixedSize(Vector2i(25, 20));

    stats->setPosition({0, 0});
    game.gui().screen()->performLayout();
	}

	std::string str_tail(std::string const& source, size_t const length)
	{
		if (length >= source.size()) { return source; }
		return source.substr(source.size() - length);
	}
	std::string str_front(std::string const& source, size_t const length)
	{
		if (length > source.size()) { return source; }
		return source.substr(0, length);
	}

	void GUIRenderer::render(Renderer& renderer)
	{
		// clear depth buffer
		glDepthMask(GL_TRUE);
		glClear(GL_DEPTH_BUFFER_BIT);

		glDisable(GL_DEPTH_TEST);
		glDepthMask(GL_FALSE);

		/// player hand ///
		renderPlayerhand(renderer.getCounter());

		glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glDisable(GL_CULL_FACE);

		/// compass & minimap ///
		renderMinimap(ortho);

		/// crosshair ///
		renderCrosshair(ortho);

		/// debug text ///
    const float fps = renderer.fps();
    fpsbox->setValue(((int) (fps * 128.0)) / 128.0);
    drawbox->setValue(drawq.size());

    genbox->setValue(Generator::size());
    prqbox->setValue(precompq.size());
    objbox->setValue(terragen::ObjectQueue::size());
    objretrybox->setValue(terragen::ObjectQueue::retry_size());

    trnbox->setValue(plogic.terrain().name);
    auto* sector = sectors.sectorAt(player.pos.x, player.pos.z);
		if (sector)
    {
      sectlts->setValue(sector->getLightCount());
      sectstage->setValue(sector->isUpdatingMesh());
      sectobjs->setValue(sector->objects);
      sectatmos->setValue(sector->atmospherics);
      // only show flatland values when generated
      if (sector->generated())
      {
        int x = int(player.pos.x);
        int z = int(player.pos.z);
        auto& flat = sector->flat()(x & (BLOCKS_XZ-1), z & (BLOCKS_XZ-1));
        skybox->setValue(flat.skyLevel);
        gndbox->setValue(flat.groundLevel);
      }
    }

    /// render graphical interfaces ///
    game.gui().render();

    /*
		// determine selection
		std::lock_guard<std::mutex> lock(mtx.playerselection);

		// exit if we have no selection
		if (plogic.hasSelection())
		{
		Block& selb   = plogic.selection.block;
		int selection = plogic.selection.facing;

		std::stringstream ss;
		ss << "(inner) ID: " << selb.getID() << " f: " << selection << "  light: " << (int) selb.getSkyLight()
		   << "  tex: " << (int) selb.getTexture(selection);

		int ddx = plogic.selection.pos.x;
		int ddy = plogic.selection.pos.y;
		int ddz = plogic.selection.pos.z;
		switch (selection)
		{
			case 0: ddz += 1; break; // +z
			case 1: ddz -= 1; break; // -z
			case 2: ddy += 1; break; // +y
			case 3: ddy -= 1; break; // -y
			case 4: ddx += 1; break; // +x
			case 5: ddx -= 1; break; // -x
		}

		selb = Spiders::getBlock(ddx, ddy, ddz);
		std::string torch = std::to_string( (int) selb.getChannel(1) ) + "," +
							std::to_string( (int) selb.getChannel(2) ) + "," +
							std::to_string( (int) selb.getChannel(3) );
		ss << "  (outer) ID: " << selb.getID() << " sky: " << (int) selb.getSkyLight() << "  torch: " << torch;

		font.print(glm::vec3(0.01, 0.035, 0.0), textScale, ss.str(), false);
		}
    */

		glDisable(GL_BLEND);
		//////////////////
	}

}
