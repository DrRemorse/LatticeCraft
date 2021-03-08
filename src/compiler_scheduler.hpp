#pragma once
#include <memory>
#include <vector>

namespace cppcraft
{
	class Precomp;

	class CompilerScheduler
	{
	public:
		static void add(std::unique_ptr<Precomp> prc);

		// returns all objects queued
		static inline std::vector<std::unique_ptr<Precomp>> get();

		// used by teleport
		static void reset();
	};

}
