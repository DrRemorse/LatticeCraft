#include "netplayer.hpp"

#include <library/log.hpp>
#include "sectors.hpp"
#include "world.hpp"
#include <cmath>

using namespace library;

namespace cppcraft
{
	static const float PI = 4 * atan(1);
	static const float PI2 = PI * 2;
	
	NetPlayer::NetPlayer(NetPlayer::userid_t uid, const std::string& name, int model, unsigned int color, w_coord& wc, const glm::vec3& pos)
	{
		this->userID = uid;
		this->name   = name;
		this->model  = model;
		this->color  = color;
		this->render = false;
		this->wc_from = this->wc_to = wc;
		this->bc_from = this->bc_to = pos;
		this->moving = false;
	}
	
	void NetPlayer::movementUpdate()
	{
		/// check if player is in local grid ///
		if (wc_to.x < world.getWX() || wc_to.z < world.getWZ() ||
			wc_to.x >= world.getWX() + sectors.getXZ() || wc_to.z >= world.getWZ() + sectors.getXZ())
		{
			// since the player is obviously outside render distance, get out
			//logger << Log::INFO << this->name << " was outside rendering distance" << Log::ENDL;
			wc_from = wc_to;
			bc_from = bc_to;
			render = false;
			return;
		}
		
		/// check if player FROM is too far from TO ///
		static const int S_MAX_DIST = 2;
		
		// make sure to-position isn't too far from from-position, if it is we need to just set the position and get out
		if (abs(wc_from.x-wc_to.x) > S_MAX_DIST || abs(wc_from.z-wc_to.z) > S_MAX_DIST)
		{
			// the position was too far to interpolate on, just set new from position
			//logger << Log::INFO << this->name << " was outside interpolation distance" << Log::ENDL;
			wc_from = wc_to;
			bc_from = bc_to;
		}
		
		render = true;
	}
	
	void NetPlayer::moveTo(w_coord& wc, const glm::vec3& pos)
	{
		this->wc_to = wc;
		this->bc_to = pos;
		moving = true;
	}
	void NetPlayer::stopMoving()
	{
		moving = false;
	}
	void NetPlayer::setRotation(int rx, int ry)
	{
		this->rotation = glm::vec2(rx, ry) / 4096.0f * PI2;
	}
	
	glm::vec3 NetPlayer::getPosition(int wx, int wy, int wz)
	{
		glm::vec3 position;
		position.x = (this->wc_from.x - wx) * Sector::BLOCKS_XZ + bc_from.x;
		position.y = (this->wc_from.y - wy) * Sector::BLOCKS_Y  + bc_from.y;
		position.z = (this->wc_from.z - wz) * Sector::BLOCKS_XZ + bc_from.z;
		return position;
	}
}

