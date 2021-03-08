#ifndef VERTEX_PLAYER_HPP
#define VERTEX_PLAYER_HPP

namespace cppcraft
{
	typedef float GLfloat;
	typedef signed char GLbyte;
	typedef short GLshort;
	typedef unsigned short GLushort;
	typedef int GLint;
	typedef unsigned int GLuint;
	
	#pragma pack(push, 4)
	struct player_vertex_t
	{
		GLfloat x; // 0
		GLfloat y;
		GLfloat z;
		
		GLbyte nx; // 12
		GLbyte ny;
		GLbyte nz;
		GLbyte padding1;
		
		GLbyte u; // 16
		GLbyte v;
		GLbyte w;
		GLbyte padding2;
		
	};  // 20
	#pragma pack(pop)
}

#endif
