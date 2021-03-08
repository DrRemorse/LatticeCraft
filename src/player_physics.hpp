#ifndef PLAYER_PHYSICS_HPP
#define PLAYER_PHYSICS_HPP

namespace cppcraft
{
	class PlayerPhysics
	{
	public:
		static const double PLAYER_SIZE;
		
		static const double bump_requirement;
		static const double bump_reflect_treshold;
		static const double bump_reflection;
		static const double bump_friction;
		static const double bump_jumpiness;
		static const double bump_maxjump;
		
		static const double movement_determinator;
		static const double crouch_determinator;
		
		static const double spdForward;
		static const double spdCrouch;
		static const double spdSprint;
		
		static const double spdLadder;
		static const double spdLadderSprint;
		
		static const double spdJump;
		static const double spdAutoJump;
		static const double spdFluid;
		static const double spdJumpFluid;
		static const double gravity;
		static const double gravity_max;
		static const double gravity_fluid;
		
		static const double spdFlying;
		static const double spdUpDown;
		
		static const double spdJetpack;
		static const double spdJetpackBurst;
		static const float  JETPACK_POWER;
		
		// sound related
		static const double splash_min_fall;
		static const double splash_big_fall;
		// sound intervals
		static const double INT_SoundWalk;
		static const double INT_SoundCrouch;
		static const double INT_SoundSprint;
		static const double INT_WalkSoundBias;
		
		PlayerPhysics();
		void handleMomentum();
		
	private:
		double curspeed;
	};
	extern PlayerPhysics playerPhysics;
}

#endif
