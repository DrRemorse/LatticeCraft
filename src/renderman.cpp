#include "renderman.hpp"

#include <library/log.hpp>
#include <library/timing/timer.hpp>
#include "camera.hpp"
#include "compilers.hpp"
#include "drawq.hpp"
#include "game.hpp"
#include "gameconf.hpp"
#include "player.hpp"
#include "render_fs.hpp"
#include "render_gui.hpp"
#include "render_scene.hpp"
#include "shaderman.hpp"
#include "sun.hpp"
#include "textureman.hpp"
#include "tiles.hpp"
#include "threading.hpp"
#include <cmath>
#include <deque>

using namespace library;

namespace cppcraft
{
	Renderer::Renderer(const std::string& title)
	{
    const bool FULLSCREEN = config.get("render.fullscreen", false);
    const bool vsync      = config.get("render.vsync", false);
    const int refreshrate = config.get("render.refresh", 0);
    const bool MAXIMIZE = config.get("window.maximize", false);
    const int SW = config.get("window.width", 1280);
    const int SH = config.get("window.height", 720);
    const int WX = config.get("window.x", 128);
    const int WY = config.get("window.y", 128);

    glfwInit();
    glfwWindowHint(GLFW_RESIZABLE, 1);
    if (MAXIMIZE) glfwWindowHint(GLFW_MAXIMIZED, 1);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);

		// open SDL window
    m_window = glfwCreateWindow(
            SW, SH, title.c_str(),
            (FULLSCREEN) ? glfwGetPrimaryMonitor() : nullptr, nullptr);
    assert(m_window);
    glfwMakeContextCurrent(m_window);
    glfwSwapInterval(1);

    const auto ogl = library::OpenGL(false);
    assert(ogl.supportsVBO);
    assert(ogl.supportsVAO);
    assert(ogl.supportsFramebuffers);
    assert(ogl.supportsShaders);
    assert(ogl.supportsTextureArrays);

    // get actual size
    glfwGetFramebufferSize(m_window, &this->m_width, &this->m_height);
    // enable resize event (TODO: fix hack)
    static Renderer* meself = this;
    glfwSetFramebufferSizeCallback(m_window,
    [] (GLFWwindow*, int w, int h) {
      meself->resize_handler(w, h);
    });

		// enable custom point sprites
		glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
		glTexEnvi(GL_POINT_SPRITE, GL_COORD_REPLACE, GL_TRUE);

    // sample to coverage (only when multisampling)
    //glSampleCoverage(0.5f, GL_FALSE);
    //glEnable(GL_SAMPLE_ALPHA_TO_ONE);

		// enable seamless cubemaps (globally)
		glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
		//
		glDisable(GL_PRIMITIVE_RESTART);
		glPrimitiveRestartIndex(65535);

#ifdef OPENGL_DO_CHECKS
		if (OpenGL::checkError()) {
			logger << Log::ERR << "Renderer::init(): OpenGL error. Line: " << __LINE__ << Log::ENDL;
			throw std::runtime_error("Renderer::init(): General openGL error");
		}
#endif
	}

	void Renderer::prepare()
	{
		logger << Log::INFO << "* Preparing renderer" << Log::ENDL;

		// initialize compilers
		Compilers::init();

		// initialize sun, and lens textures at half-size
		thesun.init(SunClass::SUN_DEF_ANGLE);

		// initialize screenspace blur, lensflare & crepuscular beams
		screenspace.init(*this);

		// initialize texture manager
		textureman.init(*this);
    // initialize camera
		camera.init(*this);
		// initialize shader manager
		shaderman.init(*this);

		// initialize scene renderer
		m_scene.reset(new SceneRenderer(*this));

		// initialize gui renderer
		rendergui.init(*this);

    // call on_resize handler once
    resize_handler(this->m_width, this->m_height);
	}

	void Renderer::render(double time_d_factor)
	{
		this->dtime = time_d_factor;

		// increment framecounter
		this->frametick += dtime;
		// interpolate sun position
		thesun.integrate(0.02 * dtime);

#ifdef TIMING
		Timer timer;
		timer.startNewRound();
#endif

		// render scene
		m_scene->render();

#ifdef TIMING
		logger << Log::INFO << "Time spent rendering: " << timer.getDeltaTime() * 1000.0 << Log::ENDL;
#endif

		// count elements in draw queue
		this->scene_elements = drawq.size();

		// post processing
		screenspace.render(*this, m_scene->isUnderwater());
		// gui
		rendergui.render(*this);

#ifdef OPENGL_DO_CHECKS
		if (OpenGL::checkError()) {
			throw std::runtime_error("Renderer::render(): OpenGL state error after rendering frame");
		}
#endif

		// flip burgers
		glfwSwapBuffers(this->m_window);

		// disable stuff
		camera.rotated = false;
	}


	/**
	 * Rendering loop w/delta frame timing
	 * Runs forever until game ends
	**/
	void Renderer::renderloop()
	{
		const double render_granularity = 0.01; // 10ms granularity
		double t1 = 0.0;

		int framesCounter = 0;
		const int framesMax = 15;
		double framesTime = 0.0;

    glfwSetTime(0.0);
		this->FPS = 0.0;

		while (game.is_terminating() == false && !glfwWindowShouldClose(m_window))
		{
			/// variable delta frame timing ///
			double t0 = t1;
			t1 = glfwGetTime();

			dtime = (t1 - t0) / render_granularity;

			/// FPS COUNTER ///
			if (framesCounter == framesMax)
			{
				this->FPS = framesMax / (t1 - framesTime);
				// round to one decimal
				this->FPS = std::round(this->FPS * 10.0) / 10.0;
				// reset
				framesTime = t1;
				framesCounter = 1;
			}
			else framesCounter++;

			// rendering function
			render(dtime);

      // compiling columns
			Compilers::run(m_scene->snap_wx(), m_scene->snap_wz(),
                     m_scene->snap_delta_x(), m_scene->snap_delta_z());

      // handle inputs (main thread)
      game.input().handle();

		} // rendering loop

    // call renderer termination functions
    for (auto& term_func : terminate_signal) {
      term_func();
    }

    // make sure game is terminating
    game.terminate();
	}

  void Renderer::resize_handler(const int w, const int h)
  {
    this->m_width  = w;
    this->m_height = h;
    for (auto& func : resize_signal) func(*this);
  }

}
