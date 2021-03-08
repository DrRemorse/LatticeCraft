#ifndef SEAMLESS_HPP
#define SEAMLESS_HPP

#include "delegate.hpp"

namespace cppcraft
{
	class Seamless
	{
  public:
    static const int OFFSET;

		static bool run();
	  static void on_transition(delegate<void()> f);

	private:
		static bool seamlessness();
	};

}

#endif
