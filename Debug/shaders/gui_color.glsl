#version 130
#define VERTEX_PROGRAM
#define FRAGMENT_PROGRAM

#ifdef VERTEX_PROGRAM
uniform mat4 mvp;

in vec3 in_vertex;
in vec4 in_color;

out vec4 color;

void main()
{
	gl_Position = mvp * vec4(in_vertex, 1.0);
	color = in_color;
}
#endif
#ifdef FRAGMENT_PROGRAM
uniform vec4 multcolor;

in vec4 color;

void main(void)
{
	gl_FragData[0] = color * multcolor;
}
#endif
