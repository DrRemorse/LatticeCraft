#include "netplayers.hpp"

#include <library/log.hpp>
#include <library/opengl/oglfont.hpp>
#include <library/opengl/opengl.hpp>
#include <library/bitmap/colortools.hpp>
#include "blockmodels.hpp"
#include "camera.hpp"
#include "player.hpp"
#include "render_gui.hpp"
#include "sectors.hpp"
#include "sun.hpp"
#include "shaderman.hpp"
#include "textureman.hpp"
#include "vertex_player.hpp"
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtx/transform.hpp>
#include <library/math/matrix.hpp>
#include <cmath>

using namespace library;
using namespace glm;

namespace cppcraft
{
	NetPlayers netplayers;

	bool NetPlayers::remove(unsigned int uid)
	{
		int index = indexByUID(uid);
		if (index != -1)
		{
			std::swap(players[index], players.back());
			players.pop_back();
		}
		return (index != -1);
	}

	int NetPlayers::indexByUID(unsigned int uid) const
	{
		for (size_t i = 0; i < players.size(); i++)
		{
			if (players[i].getUserID() == uid) return i;
		}
		return -1;
	}
	NetPlayer* NetPlayers::playerByUID(unsigned int uid)
	{
		for (NetPlayer& p : players)
		{
			if (p.getUserID() == uid) return &p;
		}
		return nullptr;
	}

	void NetPlayers::positionSnapshots(int wx, int wz, double dtime)
	{
		for (NetPlayer& p : players)
		{
			if (p.render)
			{
				//logger << Log::INFO << this->name << ": interpolating movement" << Log::ENDL;

				// since the player is inside render distance, and the to and from distances are close enough,
				// we will be interpolating for smooth movement
				int wfx = (p.wc_from.x - wx) * Sector::BLOCKS_XZ;
				int wfy = (p.wc_from.y -  0) * Sector::BLOCKS_Y;
				int wfz = (p.wc_from.z - wz) * Sector::BLOCKS_XZ;

				int wtx = (p.wc_to.x - wx) * Sector::BLOCKS_XZ;
				int wty = (p.wc_to.y -  0) * Sector::BLOCKS_Y;
				int wtz = (p.wc_to.z - wz) * Sector::BLOCKS_XZ;

				// calculate local positions
				glm::vec3 wfvec = glm::vec3(wfx, wfy, wfz) + p.bc_from;
				glm::vec3 wtvec = glm::vec3(wtx, wty, wtz) + p.bc_to;

				// interpolate based on dtime
				const float interpolator = 0.05 * dtime;
				glm::vec3 newpos = wfvec * (1.0f - interpolator) + wtvec * interpolator;

				/// recalculate new from position ///
				// split into fractional and integrals
				glm::vec3 fracs = fract(newpos);
				int bx = newpos.x;
				int by = newpos.y;
				int bz = newpos.z;
				// reconstruct world position
				p.wc_from.x = (bx / Sector::BLOCKS_XZ) + wx;
				p.wc_from.y = (by / Sector::BLOCKS_Y)  +  0;
				p.wc_from.z = (bz / Sector::BLOCKS_XZ) + wz;
				// reconstruct sector position
				p.bc_from.x = (bx & (Sector::BLOCKS_XZ-1)) + fracs.x;
				p.bc_from.y = (by & (Sector::BLOCKS_Y-1))  + fracs.y;
				p.bc_from.z = (bz & (Sector::BLOCKS_XZ-1)) + fracs.z;

				p.gxyz = glm::vec3(
					((p.wc_from.x - wx) * Sector::BLOCKS_XZ) + p.bc_from.x,
					((p.wc_from.y)      * Sector::BLOCKS_Y)  + p.bc_from.y,
					((p.wc_from.z - wz) * Sector::BLOCKS_XZ) + p.bc_from.z
				);
			}
		}
	} // positionSnapshots

	void NetPlayers::handlePlayers()
	{
		for (NetPlayer& p : players)
		{
			p.movementUpdate();
		}
	}

	static const double PI = 4 * atan(1);
	static const double PI2 = PI * 2;

	void NetPlayers::createTestPlayer()
	{
		w_coord wc;
		wc.x = world.getWX() + sectors.getXZ() / 2;
		wc.z = world.getWZ() + sectors.getXZ() / 2 - 1;
		wc.y = 0;

		glm::vec3 pos(-6.0f, 2.45f, 9.0f);

		NetPlayer nplayer(1234, "Test", 1, 255, wc, pos);

		players.push_back(nplayer);
	}

	void NetPlayers::modulateTestPlayer(double frametime)
	{
		NetPlayer* pl = playerByUID(1234);
		if (pl)
		{
			pl->setRotation(cos(frametime * 0.04) / PI2 * 4096, fmod(frametime * 0.03, PI2) / PI2 * 4096);
		}
		//pl->bc_to.x += 0.02;
	}

	/*float curveAngle(float from, float to, float step)
	{
		if (step == 0) return from;
		if (from == to || step == 1) return to;

		Vector2 fromVector = new Vector2((float)Math.Cos(from), (float)Math.Sin(from));
		Vector2 toVector = new Vector2((float)Math.Cos(to), (float)Math.Sin(to));

		Vector2 currentVector = Slerp(fromVector, toVector, step);

		return (float)Math.Atan2(currentVector.Y, currentVector.X);
	}

	public static Vector2 Slerp(Vector2 from, Vector2 to, float step)
	{
		if (step == 0) return from;
		if (from == to || step == 1) return to;

		double theta = Math.Acos(Vector2.Dot(from, to));
		if (theta == 0) return to;

		double sinTheta = Math.Sin(theta);
		return (float)(Math.Sin((1 - step) * theta) / sinTheta) * from + (float)(Math.Sin(step * theta) / sinTheta) * to;
	}*/

	float interpolate_angle(double afrom, double ato, double weight, double maxdelta)
	{
		float delta = ato - afrom;

		if (fabs(delta) > PI)
		{
			if (delta > 0) ato -= PI2;
			else           ato += PI2;
		}

		if (fabs(delta) > maxdelta)
			afrom = afrom * (1.0 - weight) + ato * weight;

		while (afrom >= PI2) afrom -= PI2;
		while (afrom <    0) afrom += PI2;
		return afrom;
	}

	void NetPlayers::renderPlayers(double frameCounter, double dtime)
	{
		#ifdef TEST_MODEL
			/// testing ///
			netplayers.modulateTestPlayer(frameCounter);
		#endif

		// playermodel texture
		textureman[Textureman::T_PLAYERMODELS].bind(0);

		// playermodel shader
		Shader& shd = shaderman[Shaderman::PLAYERMODEL];
		shd.bind();

		// common stuff
		shd.sendVec3 ("v_ldir",   thesun.getRealtimeViewAngle());
		shd.sendFloat("daylight", thesun.getRealtimeDaylight());
		shd.sendFloat("frameCounter", frameCounter);

		//shd.sendFloat("modulation", torchlight.getModulation(frameCounter));

		// head mesh
		if (vao.good() == false)
		{
			// create ccube and copy all centerCube vertices to ccube
			std::vector<player_vertex_t> ccube;
			blockmodels.skinCubes.copyAll(ccube);

			// upload data
			vao.begin(sizeof(player_vertex_t), ccube.size(), ccube.data());
			vao.attrib(0, 3, GL_FLOAT, false, offsetof(player_vertex_t, x));
			vao.attrib(1, 3, GL_BYTE,  true,  offsetof(player_vertex_t, nx));
			vao.attrib(2, 4, GL_BYTE,  false, offsetof(player_vertex_t, u));
			vao.end();
		}

		// render each player
		for (size_t i = 0; i < players.size(); i++)
		{
			NetPlayer& np = players[i];
			if (np.render == false) continue;

			// FIXME: use dot product to determine if player is even in camera vision
			// FIXME: send matview as camera followed by matmodel for use in both position & dotlight

			//logger << Log::INFO << "Rendering player: " << i << " at " << players[i].gxyz << Log::ENDL;

			/// now rendering player model ///
			shd.sendFloat("skinmodel", np.model);

			glm::mat4 matview = camera.getViewMatrix();
			matview *= glm::translate(np.gxyz);

			float headrot = PI - np.rotation.y;
			while (headrot < 0) headrot += PI2;

			/// render head ///
			glm::mat4 matv = matview;
			matv *= glm::rotate(headrot, glm::vec3(0.0f, 1.0f, 0.0f));
			//matv.rotateZYX(0.0, headrot, 0.0);
			matv *= glm::rotate(np.rotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
			//matv.rotateZYX(np.rotation.x, 0.0, 0.0);
			shd.sendMatrix("matview", matv);

			/*mat4 matrot = rotationMatrix(0.0, headrot, 0.0);
			matrot.rotateZYX(np.rotation.x, 0.0, 0.0);
			shd.sendMatrix("matrot", matrot);*/

			vao.render(GL_QUADS, 0, 24);

			const float maxdelta = (np.moving) ? 0.05 : 0.8;
			const float weight = 0.25 * dtime;

			// interpolate body rotation and make it follow head rotation when moving,
			// otherwise make it hold towards some minimum angle (maxdelta)
			float brot = interpolate_angle(np.bodyrot, headrot, weight, maxdelta);
			np.bodyrot = interpolate_angle(brot, np.bodyrot, 0.05 * dtime, 0.0);

			/*matrot = rotationMatrix(0.0, np.bodyrot, 0.0);
			shd.sendMatrix("matrot", matrot);*/

			/// render chest ///
			matv = matview;
			matv *= glm::translate(glm::vec3(0.0f, -0.72f, 0.0f));
			matv *= rotationMatrix(0.0f, np.bodyrot, 0.0f);
			shd.sendMatrix("matview", matv);

			vao.render(GL_QUADS, 24, 24);

			/// render hands ///
			matv = matview;
			matv *= rotationMatrix(0.0f, np.bodyrot, 0.0f);
			matv *= glm::translate(glm::vec3(-0.25f, -0.27f, 0.0f));
			if (np.moving)
				matv *= rotationMatrix(sin(frameCounter * 0.1), 0.0f, 0.0f);
			shd.sendMatrix("matview", matv);

			vao.render(GL_QUADS, 48, 24);

			matv = matview;
			matv *= rotationMatrix(0.0f, np.bodyrot, 0.0f);
			matv *= glm::translate(glm::vec3(0.25f, -0.27f, 0.0f));
			if (np.moving)
				matv *= rotationMatrix(sin(-frameCounter * 0.1), 0.0f, 0.0f);
			shd.sendMatrix("matview", matv);

			vao.render(GL_QUADS, 48, 24);

			/// render legs ///
			matv = matview;
			matv *= rotationMatrix(0.0f, np.bodyrot, 0.0f);
			matv *= glm::translate(glm::vec3(-0.11f, -0.75f, 0.0f));
			if (np.moving)
				matv *= rotationMatrix(-sin(frameCounter * 0.1), 0.0, 0.0f);
			shd.sendMatrix("matview", matv);

			vao.render(GL_QUADS, 72, 24);

			matv = matview;
			matv *= rotationMatrix(0.0f, np.bodyrot, 0.0f);
			matv *= glm::translate(glm::vec3(0.11f, -0.75f, 0.0f));
			if (np.moving)
				matv *= rotationMatrix(sin(frameCounter * 0.1), 0.0f, 0.0f);
			shd.sendMatrix("matview", matv);

			vao.render(GL_QUADS, 72, 24);

		} // render each player
	} // render players

	void NetPlayers::renderNameTags()
	{
    /*
		SimpleFont& font = rendergui.getFont();

		font.bind(0);
		font.setBackColor(glm::vec4(0.2f, 0.2f, 0.2f, 0.3f));

		// render each player
		for (size_t i = 0; i < players.size(); i++)
		{
			NetPlayer& np = players[i];
			if (np.render == false) continue;

			// FIXME: use dot product to determine if player is even in camera vision
			// FIXME: send matview as camera followed by matmodel for use in both position & dotlight

			// rotation & translation
			glm::mat4 matview = camera.getViewMatrix();
			matview *= glm::translate(np.gxyz);
			matview *= glm::translate(glm::vec3(0.0f, 0.25f, 0.0f));
			//matview.rotateZYX(0.0, np.bodyrot, PI);
			matview *= rotationMatrix(0.0, PI2 - player.rot.y, 0.0);

			font.sendMatrix(camera.getProjection() * matview);

			// player color
			glm::vec4 color = colorToVector(np.color);
			color.w = 1.0; // someone always fucks up alpha somewhere
			font.setColor(color);

			// print text
			const vec2 SCALE(0.16 * 0.6, 0.16);
			const vec3 OFFSET(-np.name.size() * SCALE.x * 0.5, 0.0, 0.0);

			font.print(OFFSET, SCALE, np.name, true);
		}
    */
	}

}
