#include "render_scene.hpp"

#include <library/log.hpp>
#include "block.hpp"
#include "player.hpp"
#include "player_logic.hpp"
#include <cmath>

using namespace library;

namespace cppcraft
{
	static const double PI2 = 4 * atan(1) * 2;

	double SceneRenderer::cameraDeviation(double frameCounter, double dtime)
	{
		if (playerMoved)
		{
			switch (plogic.movestate)
			{
				case PMS_Normal:
					motionTimed += 0.15 * dtime;
					break;
				case PMS_Crouch:
					motionTimed += 0.10 * dtime;
					break;
				case PMS_Sprint:
					motionTimed += 0.225 * dtime;
					break;
			}
		}
		else motionTimed = 0.0;

		#define NORMAL_CAMERA_DEV()  deviation = sin(motionTimed) * 0.07;
		#define LADDER_CAMERA_DEV()  deviation = (motionTimed != 0) ? sin(motionTimed * 1.0) * 0.2 : sin(frameCounter / 20) * 0.1;

		double deviation = 0.0;
		bool deviating = false;

		if (player.Flying)
		{
			// do nothing
		}
		else if (this->isUnderwater())
		{
			// always if underwater
			deviation = sin(frameCounter / 40) * 0.04;
			deviating = true;
		}
		else if (plogic.Submerged)
		{
			// only if NOT standing on ground (needs mutex)
			if (plogic.Falling)
			{
				deviation = sin(frameCounter / 20) * 0.1;
				deviating = true;
			}
		}
		else if (plogic.Ladderized)
		{
			if (plogic.block)
			{
				// determine what block we are standing on
				if (plogic.block->isLadder() || plogic.block->isAir())
				{
					// ladder, air --> ladder
					LADDER_CAMERA_DEV();
					deviating = true;
				}
				else
				{
					// something else --> normal
					NORMAL_CAMERA_DEV();
					deviating = true;
				}
			}
		}
		else if (motionTimed != 0)
		{
			NORMAL_CAMERA_DEV();
			deviating = true;
		}

		const double lerp = 0.6;

		// when crouching we need to lower camera quite a bit
		if (plogic.movestate == PMS_Crouch)
		{
			if (deviating)
				deviation -= 0.25;
			else
				deviation = -0.25;
		}

		// interpolate deviation with old
		deviation = deviation * lerp + lastCameraDeviation * (1.0 - lerp);

		// remember new deviation
		lastCameraDeviation = deviation;
		return deviation;
	}
}
