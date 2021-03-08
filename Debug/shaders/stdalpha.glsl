#version 150
#define VERTEX_PROGRAM
#define FRAGMENT_PROGRAM

#ifdef VERTEX_PROGRAM
uniform mat4 matproj;
uniform mat4 matview;
uniform samplerBuffer buftex;
uniform float frameCounter;

in vec4 in_vertex;
in vec4 in_normal;
in vec4 in_texture;
in vec4 in_biome;
in vec4 in_data1;
in vec4 in_data2;

out vec3 texCoord;
out vec3 lightdata;
out vec4 biomeColor;
flat out vec3 out_normal;
flat out vec3 v_normals;

const float ZFAR
const float VERTEX_SCALE_INV
const float CROSSWIND_STRENGTH  = 0.5;
const float PI2                 = 6.28318530717;

void main(void)
{
  vec3 translation = texelFetch(buftex, int(in_vertex.w)).xyz;
  vec4 position = vec4(in_vertex.xyz * VERTEX_SCALE_INV + translation, 1.0);
	position = matview * position;

	texCoord = vec3(in_texture.st * VERTEX_SCALE_INV, in_texture.p);

	// standing
	float speed  = frameCounter * 0.02;
	float factor = CROSSWIND_STRENGTH * in_data1.r;
	// crosses waving in the wind
	vec2 pos = in_vertex.xz * VERTEX_SCALE_INV / 16.0;
	position.x += factor * sin(PI2 * (2.0*pos.x + pos.y) + speed);

	gl_Position = matproj * position;
	v_normals = mat3(matview) * in_normal.xyz;

  #include "unpack_light.glsl"
	biomeColor = in_biome;
	out_normal = in_normal.xyz;
}
#endif

#ifdef FRAGMENT_PROGRAM
#extension GL_EXT_gpu_shader4 : enable
#extension GL_ARB_explicit_attrib_location : enable

uniform sampler2DArray diffuse;
uniform sampler2DArray tonemap;

uniform vec3  lightVector;
uniform float daylight;
uniform float modulation;
uniform int   texrange;

in vec3 texCoord;
in vec3 lightdata;
in vec4 biomeColor;
flat in vec3 out_normal;
flat in vec3 v_normals;

layout(location = 0) out vec4 color;
#ifdef VIEW_NORMALS
layout(location = 1) out vec4 normals;
#endif

const float ZFAR

void main(void)
{
	color = texture(diffuse, texCoord.stp);
  // alpha-to-coverage solution
  //const float CUTOFF = 0.5;
  //color.a = (color.a - CUTOFF) / max(fwidth(color.a), 0.0001) + 0.5;
	if (color.a < 0.1) discard;

	// read tonecolor from tonemap
	vec4 toneColor = texture(tonemap, texCoord.stp);
	color.rgb = mix(color.rgb, biomeColor.rgb * toneColor.rgb, toneColor.a);

	#include "degamma.glsl"
	#include "worldlight_cross.glsl"
	#include "stdlight.glsl"
	#include "finalcolor.glsl"

#ifdef VIEW_NORMALS
	normals = vec4(v_normals, 1.0);
#endif
}

#endif
