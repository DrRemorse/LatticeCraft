#ifndef SOUND_STREAM_HPP
#define SOUND_STREAM_HPP

#include <SDL_mixer.h>
#include <string>
#include <memory>

namespace sound
{
	class SoundHandle;

	class Stream
	{
	public:
		static void setMasterVolume(float vol);

		Stream(const std::string& file);
    ~Stream();

		bool play();
		void stop();

		// returns true if the stream is currently active
		bool isPlaying() const noexcept;

		// set volume to level (0..1) in time_ms (milliseconds)
		void setVolume(float vol);

	private:
		Mix_Music* stream = nullptr;

		// crossfade related
		float  volume;
		double startTime;
		// master volume for streams [0..1]
		static float masterVolume;

		friend class StreamChannel;
	};
}

#endif
