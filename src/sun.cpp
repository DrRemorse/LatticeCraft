#include "sun.hpp"

#include <library/log.hpp>
#include <library/timing/timer.hpp>
#include "camera.hpp"
#include "sectors.hpp"
#include <glm/geometric.hpp>
#include <glm/gtx/transform.hpp>
#include <library/math/matrix.hpp>
#include <cmath>

using namespace library;
using namespace glm;

namespace cppcraft
{
	const float PI = 4 * atan(1);
	const float PI2 = PI * 2;
	const float SunClass::SUN_DEF_ANGLE = PI / 4.0;
	SunClass thesun;
	library::Timer suntimer;
	
	void SunClass::init(float angle)
	{
		this->radianAngle = angle;
		this->setRadianAngle(this->radianAngle);
		this->renderDist = 4.0;
		
		this->realRadian   = angle + PI;
		this->realAmbience = daylight(realRadian);
		
		this->setStep(900);
		suntimer.restart();
	}
	
	void SunClass::setRadianAngle(float angle)
	{
		// angle fmod= PI2
		while (angle >= PI2) angle -= PI2;
		while (angle <  0.0) angle += PI2;
		
		this->radianAngle = angle;
		this->angle.x   = cos(angle);
		this->angle.y   = sin(angle);
		
		float direction = (this->angle.x >= 0) ? 1.0 : -1.0;
		
		half1 = vec3(this->angle.x + direction * 0.08, this->angle.y, 0.0);
		half1 = normalize(half1);
		half2 = vec3(this->angle.x + direction * 0.12, this->angle.y, 0.0);
		half2 = normalize(half2);
	}
	inline float SunClass::getStepValue()
	{
		return suntimer.getTime() * this->step;
	}
	
	void SunClass::integrate(float timestep)
	{
		// create dest angle + time step
		float rad = radianAngle + getStepValue();
		vec3 angle = vec3(cos(rad), sin(rad), 0.0);
		// interpolate with realtime angle
		vec3 newAngle = mix(getRealtimeAngle(), angle, timestep);
		// extract "angle" from vector
		realRadian = atan2(newAngle.y, newAngle.x);
		// create new realtime angle
		realAngle = vec3(cos(realRadian), sin(realRadian), 0.0);
		// recalculate ambience
		realAmbience = 1.0 - daylight(realAngle.y);
	}
	void SunClass::travelCheck()
	{
		if (suntimer.getTime() > 30)
		{
			// set new sun angle (before timer reset)
			setRadianAngle(radianAngle + getStepValue());
			// reset timer & traveldistance
			suntimer.restart();
			// update world
			sectors.updateAll();
		}
	}
	
	// returns the current light level
	float SunClass::daylight(float y)
	{
		const float transitional_light =  0.0;
		const float transitional_dark  = -0.2;
		
		const float noonlight = 0.2;
		const float nightlight = 0.6;
		const float moonlight  = 0.8;
		
		float transit = 0.0;
		
		if (y >= transitional_light)
		{
			transit = 1.0 - (y - transitional_light) / (1.0 - transitional_light);
			return noonlight * powf(transit, 3.0);
		}
		else if (y < transitional_dark)
		{
			transit = 1.0 - (fabs(y) - transitional_dark) / (1.0 - transitional_dark);
			return moonlight * (1.0 - transit) + nightlight * transit;
		}
		
		// inside transitional phase
		float translength = fabs(transitional_light - transitional_dark);
		
		transit = 1.0 - (y + fabs(transitional_dark)) / translength;
		
		return noonlight + (nightlight - noonlight) * transit;
	}
	
	vec3 SunClass::ambienceColor()
	{
		float yy = fabs(angle.y) * angle.y; // ogl.lastsun(1)) * ogl.lastsun(1);
		
		if (yy < -0.15)
		{
			return vec3(1.0 + yy, 1.0 + yy,  1.0);
		}
		else
		{
			float redness = 0.7 + 0.3 * yy;
			#define blueness 0
			
			return vec3(1.0 - blueness, 0.4 + redness * 0.6 - blueness, redness);
		}
	}
	
	void SunClass::setStep(int seconds)
	{
		this->step = PI2 / seconds;
	}
	
	void SunClass::setRealtimeSunView(const glm::mat4& matrot)
	{
		this->realViewAngle = mat3(matrot) * realAngle;
	}
	
	glm::mat4 SunClass::getSunMatrix() const
	{
		glm::mat4 mattemp = rotationMatrix(0.0f, -PI / 2.0f, 0.0f);
		glm::mat4 matsun = rotationMatrix(thesun.getRealtimeRadianAngle(), 0.0, 0.0);
		mattemp *= matsun;
		mattemp *= glm::translate(vec3(0.f, 0.f, -thesun.renderDist));
		
		return camera.getRotationMatrix() * mattemp;
	}
	
}
