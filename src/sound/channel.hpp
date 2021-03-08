#pragma once

#include <vector>

namespace sound
{
	class Stream;

	struct ManagedStream
	{
		Stream* stream;
		float   volume;
	};

	class Channel
	{
	public:
		Channel(float delta, float maxv);

		void play(Stream& newStream);
		void stop();
		void fullStop();
		void integrate();

	private:
		ManagedStream* streamExists(Stream& stream);
		void removeStream(Stream& stream);

		ManagedStream current;
		std::vector<ManagedStream> older;

		float delta;
		float maxVolume;
	};
}
