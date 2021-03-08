/**
 * 
 * 
**/

#ifndef COMPRESSOR_HPP
#define COMPRESSOR_HPP

#include <fstream>

namespace cppcraft
{
	class Compressor
	{
	public:
		static void init();
		static void cleanup();
		static void load(std::ifstream&, int PL, int x, int z);
		
	};
}

#endif
