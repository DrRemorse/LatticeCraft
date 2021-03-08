#include "channel.hpp"

#include "stream.hpp"

namespace sound
{
	Channel::Channel(float delta, float maxv)
	{
		current.stream = nullptr;
		current.volume = 0;
		this->delta = delta;
		this->maxVolume = maxv;
	}

	void Channel::stop()
	{
		// nothing to do if the stream is already ending
		if (current.stream == nullptr) return;

		// fade-out stream that we want to stop
		older.push_back(current);
		// invalidate current
		current.stream = nullptr;
		current.volume = 0;
	}

	void Channel::fullStop()
	{
		// stop current
		if (current.stream)
		{
			current.stream->stop();
			current.stream = nullptr;
			current.volume = 0;
		}
		// stop fade-out queue
		if (older.size())
		{
			for (size_t i = 0; i < older.size(); i++)
				older[i].stream->stop();
			older.clear();
		}
	}

	void Channel::play(Stream& newStream)
	{
		// nothing to do if the new stream is the same as the current
		if (current.stream == &newStream) return;
		// if we already have a current stream, it needs to be faded out
		if (current.stream)
		{
			older.push_back(current);
		}
		// now, find out if the new current is already active
		ManagedStream* previous = streamExists(newStream);
		if (previous)
		{
			// set new current
			current = *previous;
			// remove stream from fade-out queue
			removeStream(*previous->stream);
			return;
		}
		// set new current, and start it
		current.stream = &newStream;
		current.volume = 0;
		current.stream->play();
		current.stream->setVolume(current.volume);
	}

	ManagedStream* Channel::streamExists(Stream& stream)
	{
		for (size_t i = 0; i < older.size(); i++)
			if (older[i].stream == &stream) return &older[i];
		return nullptr;
	}

	void Channel::removeStream(Stream& stream)
	{
		for (int i = older.size()-1; i >= 0; i--)
		{
			if (older[i].stream == &stream)
				older.erase(older.begin() + i);
		}
	}

	void Channel::integrate()
	{
		if (current.stream)
		{
			if (current.volume < maxVolume)
			{
				current.volume += delta;
				current.stream->setVolume(current.volume);
			}
		}
		for (int i = older.size()-1; i >= 0; i--)
		{
			older[i].volume -= delta;
			// if volume is not yet zero, continue fade-out
			if (older[i].volume > 0)
			{
				older[i].stream->setVolume(older[i].volume);
			}
			else
			{
				// stop stream entirely, and also removing it from queue
				// telling us that there is no previous stream anymore
				older[i].stream->stop();
				older.erase(older.begin() + i);
			}
		}
	}
} // namespace sound
