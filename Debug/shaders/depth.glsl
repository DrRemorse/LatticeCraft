#version 330
#define VERTEX_PROGRAM
#define FRAGMENT_PROGRAM

#ifdef VERTEX_PROGRAM
precision mediump float;

uniform mat4 matproj;
uniform mat4 matview;
uniform vec3 vtrans;

in vec3 in_vertex;
in vec3 in_normal;
in vec3 in_texture;
in vec4 in_biome;
in vec4 in_color;
in vec4 in_color2;

const float VERTEX_SCALE

void main(void)
{
	vec4 position = vec4(in_vertex / VERTEX_SCALE + vtrans, 1.0);
	gl_Position = matproj * matview * position;
}

#endif

#ifdef FRAGMENT_PROGRAM

void main(void)
{
	gl_FragData[0] = vec4(1.0);
}

#endif
