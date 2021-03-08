#include "sound.hpp"
#include "system.hpp"

namespace sound
{
  Sound::Sound(FMOD::System* sys, const std::string& filename)
	{
    auto result = sys->createSound(filename.c_str(),
          FMOD_3D | FMOD_NONBLOCKING, 0, &this->sound);
    SND_CHECK(result);
    //result = this->sound->set3DMinMaxDistance(0.5f, 256.0f);
    //SND_CHECK(result);
	}
  Sound::Sound(Sound&& other)
  {
    other.sound  = this->sound;
    this->sound  = nullptr;
  }
  Sound::~Sound()
  {
    auto result = this->sound->release();
    SND_CHECK(result);
  }

  bool Sound::check_ready() const noexcept
  {
    FMOD_OPENSTATE state;
    auto result = sound->getOpenState(&state, nullptr, nullptr, nullptr);
    SND_CHECK(result);
    if (state == FMOD_OPENSTATE_READY) {
      this->ready = true;
      return true;
    }
    return false;
  }
}
