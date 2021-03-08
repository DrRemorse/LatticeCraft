#version 150
#define VERTEX_PROGRAM
#define FRAGMENT_PROGRAM

#ifdef VERTEX_PROGRAM
uniform mat4 matmvp;
uniform mat4 matview;
uniform samplerBuffer buftex;

uniform float daylight;
uniform vec3  lightVector;

in vec4 in_vertex;
in vec4 in_normal;
in vec4 in_texture;
in vec4 in_biome;

out vec3 texCoord;
out vec3 lightdata;
out vec4 biomeColor;
flat out float worldLight;

out vec3 v_normals;

const float VERTEX_SCALE_INV

void main(void)
{
  vec3 translation = texelFetch(buftex, int(in_vertex.w)).xyz;
  vec4 position = vec4(in_vertex.xyz * VERTEX_SCALE_INV + translation, 1.0);
	gl_Position = matmvp * position;

#ifdef VIEW_NORMALS
	v_normals = mat3(matview) * in_normal.xyz;
#endif

	texCoord = vec3(in_texture.st * VERTEX_SCALE_INV, in_texture.p);

  #include "worldlight.glsl"
  #include "unpack_light.glsl"
	biomeColor = in_biome;
}
#endif

#ifdef FRAGMENT_PROGRAM
#extension GL_EXT_gpu_shader4 : enable
#extension GL_ARB_explicit_attrib_location : enable

uniform sampler2DArray diffuse;
uniform sampler2DArray tonemap;

uniform float daylight;
uniform vec3  lightVector;
uniform float modulation;

in vec3 texCoord;
in vec3 lightdata;
in vec4 biomeColor;
flat in float worldLight;
in vec3 v_normals;

layout(location = 0) out vec4 color;
#ifdef VIEW_NORMALS
layout(location = 1) out vec4 normals;
#endif

const float ZFAR

void main(void)
{
	color = texture(diffuse, texCoord.stp);
	if (color.a < 0.25) discard;

	// read tonecolor from tonemap
	vec4 toneColor = texture(tonemap, texCoord.stp);
	color.rgb = mix(color.rgb, biomeColor.rgb * toneColor.rgb, toneColor.a);

	#include "degamma.glsl"
	#include "stdlight.glsl"
	#include "finalcolor.glsl"

#ifdef VIEW_NORMALS
	normals = vec4(v_normals, 1.0);
#endif
}

#endif
