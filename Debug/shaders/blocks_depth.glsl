#version 150
#define VERTEX_PROGRAM
#define FRAGMENT_PROGRAM

#ifdef VERTEX_PROGRAM
uniform mat4 matmvp;
uniform vec3 vtrans;

in vec3 in_vertex;
in vec3 in_normal;
in vec3 in_texture;
in vec4 in_biome;
in vec4 in_color;
in vec4 in_color2;

out vec3 pos;
const float VERTEX_SCALE_INV

void main(void)
{
	vec4 position = vec4(in_vertex * VERTEX_SCALE_INV + vtrans, 1.0);
	gl_Position = matmvp * position;
	
	pos = position.xyz;
}

#endif

#ifdef FRAGMENT_PROGRAM
uniform sampler2D diffuse;
uniform vec3 screensize;

in vec3 pos;
const float ZFAR

out vec4 color;

void main(void)
{
	vec2 texCoord = gl_FragCoord.xy / screensize.xy;
	color = texture(diffuse, texCoord);
}

#endif
