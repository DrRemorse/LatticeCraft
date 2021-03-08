#include "soundman.hpp"

#include "game.hpp"
#include "gameconf.hpp"
#include "player.hpp"
#include "sectors.hpp"
#include "sound/system.hpp"
#include "generator/terrain/terrains.hpp"
#include "worldmanager.hpp"
#include <library/log.hpp>
#include <library/math/vector.hpp>
#include <library/timing/timer.hpp>
#include <glm/glm.hpp>
#include <rapidjson/document.h>

using namespace glm;
using namespace sound;
using namespace library;
//#define DISABLE_AUDIO

namespace cppcraft
{
 	Soundman soundman;

	void Soundman::init()
	{
#ifdef DISABLE_AUDIO
    return;
#endif
    static const float DISTANCEFACTOR = 1.0f;
		logger << Log::INFO << "* Initializing audio system" << Log::ENDL;

    Timer timer;
    FMOD_RESULT result = FMOD::System_Create(&system);
    SND_CHECK(result);

    unsigned int version;
    result = system->getVersion(&version);
    SND_CHECK(result);
    assert (version >= FMOD_VERSION && "Header version vs library version mismatch");

    result = system->init(32, FMOD_INIT_NORMAL, nullptr);
    SND_CHECK(result);
    result = system->set3DSettings(1.0, DISTANCEFACTOR, 1.0f);
    SND_CHECK(result);

    // load sounds and streams from mods
    for (const auto& mod : cppcraft::game.mods())
    {
      for (const auto& mod_file : mod.json_files())
      {
        std::ifstream file(mod_file);
        const std::string str((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        rapidjson::Document doc;
        doc.Parse(str.c_str());
        CC_ASSERT(doc.IsObject(), "Audio JSON must be valid");

        if (doc.HasMember("sounds"))
        {
          const auto& sound_type = doc["sounds"];
          if (sound_type.HasMember("single"))
          {
            const auto& single = sound_type["single"];
            for (auto itr = single.MemberBegin(); itr != single.MemberEnd(); ++itr)
            {
              const auto name = itr->name.GetString();
              const auto value = itr->value.GetString();
              // load single sound
              this->create_sound(name, mod.modpath() + value);
            }
          }
          if (sound_type.HasMember("materials"))
          {
            const auto& smat = sound_type["materials"];
            for (auto itr = smat.MemberBegin(); itr != smat.MemberEnd(); ++itr)
            {
              const auto name = itr->name.GetString();
              const auto value = itr->value.GetString();
              // load material sound
              this->create_material(name, mod.modpath() + value);
            }
          }
        }
        if (doc.HasMember("streams"))
        {
          const auto& strims = doc["streams"];
          for (auto itr = strims.MemberBegin(); itr != strims.MemberEnd(); ++itr)
          {
            const auto name = itr->name.GetString();
            const auto value = itr->value.GetString();
            // load stream
            this->create_stream(name, mod.modpath() + value);
          }
        }
      } // mod_file
    } // mod
    logger << "* Audio system loaded " << sounds.size() << " sounds and "
            << streams.size() << " streams" << Log::ENDL;
		logger << Log::INFO << "* Sound system initialized (" << timer.getTime() << " secs)" << Log::ENDL;
	}

	void Soundman::playSound(const std::string& name, vec3 v)
	{
    const FMOD_VECTOR pos {  v.x,  v.y,  v.z };
    const FMOD_VECTOR vel { 0.0f, 0.0f, 0.0f };

    auto it = this->sounds.find(name);
    if (it != this->sounds.end())
    {
      auto& sound = it->second;
      if (sound.is_ready()) {
        FMOD::Channel* chan;
        system->playSound(sound.get(), 0, false, &chan);
        chan->set3DAttributes(&pos, &vel);
      }
      return;
    }
    printf("Could not find sound: %s\n", name.c_str());
	}
	void Soundman::playSound(const std::string& name)
	{
    auto& sound = this->sounds.at(name);
    if (sound.is_ready()) {
      system->playSound(sound.get(), 0, false, nullptr);
    }
	}
  void Soundman::playMaterial(const std::string& name, int num)
	{
    this->playSound(name + std::to_string(num));
	}
	void Soundman::playMaterial(const std::string& name, int num, vec3 v)
	{
    this->playSound(name + std::to_string(num), v);
	}

	void Soundman::create_material(std::string name, std::string path)
	{
		// load sounds
		for (int i = 0; i < SOUNDS_PER_MAT; i++)
		{
			// create filename
			const std::string filename = path + std::to_string(i + 1) + ".ogg";
      //printf("mat %s -> %s\n", name.c_str(), filename.c_str());
			create_sound(name + std::to_string(i), filename);
		}
	}

  inline void Soundman::create_sound(const std::string& name, const std::string& file)
  {
    sounds.emplace(std::piecewise_construct,
            std::forward_as_tuple(name),
            std::forward_as_tuple(this->system, file));
  }

  inline void Soundman::create_stream(const std::string& name, const std::string& file)
  {
    streams.emplace(std::piecewise_construct,
            std::forward_as_tuple(name),
            std::forward_as_tuple(this->system, file));
  }

	// returns the id of a random song in the playlist
	void Soundman::sound_processing()
	{
#ifdef DISABLE_AUDIO
    return;
#endif
    const glm::vec3 forw = library::lookVector(player.rot);
    const glm::vec3 right = glm::normalize(glm::cross(forw, glm::vec3(0.0f, 1.0f, 0.0f)));
    const glm::vec3 upv  = glm::cross(right, forw);
    //printf("Forward: (%f, %f, %f)\n", forw.x, forw.y, forw.z);
    //printf("Up:      (%f, %f, %f)\n", upv.x, upv.y, upv.z);

    const FMOD_VECTOR pos      { player.pos.x, player.pos.y, player.pos.z };
    const FMOD_VECTOR forward  { forw.x, forw.y, forw.z };
    const FMOD_VECTOR up       { upv.x,  upv.y,  upv.z };

    // vel = how far we moved last FRAME (m/f), then time compensate it to SECONDS (m/s).
    FMOD_VECTOR vel;
    vel.x = (pos.x - last_pos.x) * (1.0 / WorldManager::TIMING_TICKTIMER);
    vel.y = (pos.y - last_pos.y) * (1.0 / WorldManager::TIMING_TICKTIMER);
    vel.z = (pos.z - last_pos.z) * (1.0 / WorldManager::TIMING_TICKTIMER);
    // store pos for next time
    last_pos = pos;

    auto result = system->set3DListenerAttributes(0, &pos, &vel, &forward, &up);
    SND_CHECK(result);
    // mui importante
    result = system->update();
    SND_CHECK(result);
    return;

		// if player is under the terrain, somehow change
		// ambience & music to cave themes
		Flatland::flatland_t* flat = sectors.flatland_at(player.pos.x, player.pos.z);
    if (flat == nullptr) return;
    // get terrain
    auto& terrain = terragen::terrains[flat->terrain];
    const int groundLevel = flat->groundLevel;

		const int CAVE_DEPTH = 6;
		const bool in_caves = (player.pos.y < groundLevel - CAVE_DEPTH);

		if (gameconf.ambience)
		{

		}
	}

}
