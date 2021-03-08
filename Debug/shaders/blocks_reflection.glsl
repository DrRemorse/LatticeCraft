#version 150
#define VERTEX_PROGRAM
#define FRAGMENT_PROGRAM

#ifdef VERTEX_PROGRAM
uniform mat4 matmvp;
uniform samplerBuffer buftex;

uniform vec3  lightVector;
uniform float daylight;

in vec4 in_vertex;
in vec4 in_normal;
in vec4 in_texture;
in vec4 in_biome;

out vec3 texCoord;
out vec3 lightdata;
out vec4 biomeColor;
flat out float worldLight;
out float dist;

const float VERTEX_SCALE_INV
const float ZFAR
const float WATERLEVEL

void main(void)
{
  vec3 translation = texelFetch(buftex, int(in_vertex.w)).xyz;
  vec4 position = vec4(in_vertex.xyz * VERTEX_SCALE_INV + translation, 1.0);
	gl_ClipDistance[0] = position.y - WATERLEVEL;
	gl_Position = matmvp * position;

	dist = length(gl_Position.xyz) / ZFAR;
	texCoord = vec3(in_texture.st * VERTEX_SCALE_INV, in_texture.p);

	// dotlight
	#include "worldlight.glsl"

  #include "unpack_light.glsl"
	biomeColor = in_biome;
}

#endif

#ifdef FRAGMENT_PROGRAM
#extension GL_EXT_gpu_shader4 : enable

uniform sampler2DArray diffuse;
uniform sampler2DArray tonemap;

uniform float daylight;
uniform float modulation;

in vec3 texCoord;
in vec3 lightdata;
in vec4 biomeColor;
flat in float worldLight;
in float dist;

const float ZFAR

out vec4 color;

void main(void)
{
	// independent texture reads using inbound variable directly
	// read tonecolor from tonemap
	vec4 tone = texture(tonemap, texCoord);
	tone.rgb *= biomeColor.rgb;

	// mix diffuse map
	color = texture(diffuse, texCoord);
	color.rgb = mix(color.rgb, tone.rgb, tone.a);

	#include "degamma.glsl"

	#include "stdlight.glsl"

	// fake fog
	vec3 fogColor = vec3(1.0) * daylight;
	color.rgb = mix(color.rgb, fogColor, 0.1 + dist * 0.3);

	color.rgb = pow(color.rgb, vec3(2.2));
}
#endif
