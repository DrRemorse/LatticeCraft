#version 130
#extension GL_EXT_gpu_shader4 : enable
#define VERTEX_SHADER
#define FRAGMENT_SHADER

#ifdef VERTEX_SHADER
uniform mat4 matproj;
uniform mat4 matview;
uniform vec3 vtrans;

uniform float daylight;
uniform vec4 lightVector;

in vec3 in_vertex;
in vec3 in_texture;
in vec3 in_normal;
in vec4 in_color;
in vec4 in_color2;
in vec4 in_biome;

out float vertdist;
out vec3 coord;

out vec4 lightdata;
out vec4 torchlight;
out vec3 biomeColor;
flat out float worldLight;

const float VERTEX_SCALE

void main(void)
{
	
	vec4 position = matview * vec4(in_vertex / VERTEX_SCALE + vtrans, 1.0);
	
	vertdist = length(position);
	
	gl_Position = matproj * position;
	
	coord = vec3(in_texture.st / VERTEX_SCALE, in_texture.p);
	
	lightdata  = in_color;
	torchlight = in_color2;
	biomeColor = in_biome.rgb;
	
	// dotlight
	#include "worldlight.glsl"
	
}

#endif

#ifdef FRAGMENT_SHADER

uniform sampler2DArray texture;
uniform sampler2DArray tonemap;
uniform sampler2D skybuffer;

uniform float daylight;
uniform float modulation;
uniform vec3 screendata;

in float vertdist;
in vec3 coord;

in vec4 lightdata;
in vec4 torchlight;
in vec3 biomeColor;
flat in float worldLight;

const float ZFAR

void main(void)
{
	vec4 color = texture2DArray(texture, coord.stp);
	if (color.a < 0.1) discard;
	
	// read tonecolor from tonemap
	vec4 toneColor = texture2DArray(tonemap, coord.stp);
	color.rgb = mix(color.rgb, biomeColor.rgb * toneColor.rgb, toneColor.a);
	
	const vec3 degamma = vec3(1.0 / 2.2);
	
	// degamma
	color.rgb = pow(color.rgb, degamma);
	
	#include "stdlight.glsl"
	
	#include "horizonfade.glsl"
	
	gl_FragColor = vec4(color.rgb, 1.0);
}

#endif
