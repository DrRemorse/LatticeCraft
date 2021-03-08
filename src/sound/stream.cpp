#include "stream.hpp"

#include <library/math/toolbox.hpp>
#include <cassert>

using namespace library;

namespace sound
{
	float Stream::masterVolume = 1.0;

	Stream::Stream(const std::string& filename)
	{
		this->stream = Mix_LoadMUS(filename.c_str());
    if (this->stream == nullptr)
		{
			throw std::runtime_error("Stream::play(): Could not load " + filename);
		}
	}
  Stream::~Stream()
  {
    if (this->stream) Mix_FreeMusic(this->stream);
  }

	bool Stream::play()
	{
    printf("Stream::play()\n");
    assert(Mix_FadeInMusic(this->stream, -1, 1500) == 0);
    return true;
	}

	bool Stream::isPlaying() const noexcept
	{
    return Mix_PlayingMusic();
	}

	void Stream::stop()
	{
    if (isPlaying() && Mix_FadingMusic() != MIX_FADING_OUT) {
      Mix_FadeOutMusic(1500);
    }
	}

	void Stream::setVolume(float vol)
	{
		vol = clamp(0.0, 1.0, vol);
		Mix_VolumeMusic(vol * MIX_MAX_VOLUME);
	}

	void Stream::setMasterVolume(float vol)
	{
		Stream::masterVolume = vol;
	}
}
