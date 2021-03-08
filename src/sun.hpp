#ifndef THESUN_HPP
#define THESUN_HPP

#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>

namespace library
{
	class mat4;
}

namespace cppcraft
{
	class SunClass
	{
	public:
		static const float SUN_DEF_ANGLE;
		
		void init(float radangle);
		void render();
		
		// permanently sets new static sun angle
		void setRadianAngle(float radians);
		// getter for static sun angle
		float getRadianAngle() const
		{
			return this->radianAngle;
		}
		
		// getters for motionless sun
		const glm::vec3& getAngle() const
		{
			return this->angle;
		}
		const glm::vec3& getHalfAngle() const
		{
			return this->half1;
		}
		const glm::vec3& getHalf2Angle() const
		{
			return this->half2;
		}
		
		// daylight multiplier: 1.0 = full daylight, 0.0 = total darkness
		float daylight(float y);
		// world ambient light color
		glm::vec3 ambienceColor();
		
		// integrator for realtime sun values
		void integrate(float step);
		// travel distance check for updating shadows
		void travelCheck();
		// gettesr for realtime sun values
		const glm::vec3& getRealtimeAngle() const
		{
			return realAngle;
		}
		float getRealtimeRadianAngle() const
		{
			return realRadian;
		}
		float getRealtimeDaylight() const
		{
			return realAmbience;
		}
		
		// each frame set realtime view-sunvector
		void setRealtimeSunView(const glm::mat4& matrot);
		// getter for realtime view-sunvector
		const glm::vec3& getRealtimeViewAngle() const
		{
			return realViewAngle;
		}
		
		// timestep
		void setStep(int step);
		float getStepValue();
		
		glm::mat4 getSunMatrix() const;
		
	private:
		float step;
		float radianAngle;   // target angle
		glm::vec3 angle; // target sun vector from player
		glm::vec3 half1;
		glm::vec3 half2;
		
		float realRadian;
		glm::vec3 realAngle;
		glm::vec3 realViewAngle;
		float realAmbience;
		
		float renderDist;
	};
	extern SunClass thesun;
	
}

#endif
