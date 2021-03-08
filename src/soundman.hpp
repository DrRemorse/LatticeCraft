#ifndef SOUNDMAN_HPP
#define SOUNDMAN_HPP

#include "sound/sound.hpp"
#include <library/math/vector.hpp>
#include <fmod/fmod.hpp>
#include <string>
#include <map>

namespace cppcraft
{
	class Soundman {
	public:
		void init();

    static constexpr int SOUNDS_PER_MAT = 4;
		static const int sound_place = 0;
		static const int sound_land  = 1;
		static const int sound_mine  = 2;
		static const int sound_remove = 3;

		// single sounds
		void playSound(const std::string&, glm::vec3 pos);
		void playSound(const std::string&);
		// material sounds, sets of SOUNDS_PER_MAT (4)
		void playMaterial(const std::string& sound, int num, glm::vec3 pos);
		void playMaterial(const std::string& sound, int num);

    void create_sound(const std::string& name, const std::string& file);
    void create_stream(const std::string& name, const std::string& file);

		void sound_processing();

	private:
    void create_material(std::string name, std::string path);
		static std::string blockToMaterial(int id);

		std::map<std::string, sound::Sound> sounds;
		std::map<std::string, sound::Stream> streams;
    sound::Stream* current_stream = nullptr;

    // the sound system
    FMOD::System* system = nullptr;
    FMOD_VECTOR last_pos { 0.0, 0.0, 0.0 };
    double      last_time = 0.0;
	};
	extern Soundman soundman;
}

#endif
