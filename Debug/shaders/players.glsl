#version 150
#define VERTEX_PROGRAM
#define FRAGMENT_PROGRAM

#ifdef VERTEX_PROGRAM
uniform mat4 matproj;
uniform mat4 matview;

uniform vec3  v_ldir;
uniform float modulation;
uniform float skinmodel;

in vec3 in_vertex;
in vec3 in_normal;
in vec4 in_texture;

out vec3 texCoord;
flat out float worldLight;

const float TEX_SCALE = 16.0;
const float ZFAR

void main()
{
	vec4 position = matview * vec4(in_vertex.xyz, 1.0);
	gl_Position = matproj * position;
	
	texCoord = in_texture.stp;
	texCoord.st /= TEX_SCALE;
	texCoord.p  += skinmodel * 16.0;
	
	vec3 normal = mat3(matview) * in_normal;
	
	// worldlight //
	const float ambience = 0.5;
	float dotlight = dot(normal, v_ldir);
	worldLight = max(ambience * 0.6 + 0.5 * (0.5 + 0.5*dotlight), 0.2);
	// worldlight //
}

#endif

#ifdef FRAGMENT_PROGRAM
#extension GL_EXT_gpu_shader4 : enable
uniform sampler2DArray texture;
uniform float daylight;

in vec3 texCoord;
flat in float worldLight;

out vec4 color;

void main(void)
{
	color = texture2DArray(texture, texCoord);
	if (color.a < 0.1) discard; // le discard!
	
	#include "degamma.glsl"
	
	// FIXME: shadows & stuff
	color.rgb *= worldLight; // * daylight;
	
	#include "finalcolor.glsl"
}

#endif
