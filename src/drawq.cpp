#include "drawq.hpp"

#include <library/log.hpp>
#include "columns.hpp"
#include "sectors.hpp"
#include <string>

using namespace library;

namespace cppcraft
{
	DrawQueue drawq(false);
	DrawQueue reflectionq(true);

	// create all queue shaderlines
	void DrawQueue::init()
	{
		logger << Log::INFO << "* Initializing rendering queue" << Log::ENDL;

		// initialize each queue
		reset();
	}

}
