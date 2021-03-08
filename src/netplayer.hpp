#ifndef NETPLAYER_HPP
#define NETPLAYER_HPP

#include "world.hpp"
#include <string>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

namespace cppcraft
{
	struct w_coord
	{
		int x, y, z;
	};
	
	class NetPlayer
	{
	public:
		typedef unsigned int userid_t;
		
		NetPlayer();
		NetPlayer(userid_t uid, const std::string& name, int model, unsigned int color, w_coord& wc, const glm::vec3& pos);
		
		inline userid_t getUserID() const
		{
			return userID;
		}
		inline const std::string& getName() const
		{
			return name;
		}
		
		glm::vec3 getPosition(int wx, int wy, int wz);
		const glm::vec2& getRotation() const
		{
			return rotation;
		}
		
		void setRotation(int rotx, int roty);
		void moveTo(w_coord& wc, const glm::vec3& pos);
		void stopMoving();
		
		void movementUpdate();
		
	private:
		std::string name;
		userid_t userID;
		
		// the position we are interpolating from
		w_coord       wc_from, wc_to;
		// the position we are interpolating towards
		glm::vec3 bc_from, bc_to;
		// player head orientation
		glm::vec2 rotation;
		// extrapolated body rotation
		float bodyrot;
		bool  render, moving;
		
		// synchronized rendering data
		//vec2 grot;
		glm::vec3 gxyz;
		unsigned int  model;
		unsigned int  color;
		
		friend class NetPlayers;
	};
	
}

#endif // NETPLAYER_HPP
