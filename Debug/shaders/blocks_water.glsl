#version 150
#define VERTEX_PROGRAM
#define FRAGMENT_PROGRAM

#define REFLECTIONS

#ifdef VERTEX_PROGRAM
uniform mat4 matproj;
uniform mat4 matview;
uniform samplerBuffer buftex;

uniform vec3 worldOffset;
uniform float frameCounter;

in vec4 in_vertex;
in vec4 in_normal;
in vec4 in_texture;
in vec4 in_biome;

out vec3 lightdata;
out vec4 waterColor;

out vec3 v_pos;
flat out vec3 v_normal;
out vec3 w_vertex;

const float VERTEX_SCALE_INV

void main(void)
{
  vec3 translation = texelFetch(buftex, int(in_vertex.w)).xyz;
  vec4 position = vec4(in_vertex.xyz * VERTEX_SCALE_INV + translation, 1.0);

	position = matview * position;
	gl_Position = matproj * position;

	// galactic coordinates
	vec3 w_vertex  = position.xyz * mat3(matview) - worldOffset;

	// light and eye direction in view space
	v_pos = -position.xyz;
	// reflect light in view space using view-normal
	v_normal = mat3(matview) * in_normal.xyz;

  #include "unpack_light.glsl"
	waterColor = in_biome;
}

#endif

#ifdef FRAGMENT_PROGRAM
#extension GL_EXT_gpu_shader4 : enable
#extension GL_ARB_explicit_attrib_location : enable
uniform sampler2D underwatermap;
uniform sampler2D depthtexture;
uniform sampler2D reflectionmap;

uniform mat4 matview;
uniform vec3 screendata;
uniform vec2 nearPlaneHalfSize;

uniform float frameCounter;
uniform float daylight;
uniform float modulation;
uniform vec3  v_ldir;

in vec3 lightdata;
in vec4 waterColor;

in vec3 v_pos;
flat in vec3 v_normal;
in vec3 w_vertex;

layout(location = 0) out vec4 color;
layout(location = 1) out vec4 normals;

const float ZNEAR
const float ZFAR

#include "noise.glsl"

float getDepth(in vec2 uv)
{
	float wsDepth = texture(depthtexture, uv).x;
	wsDepth = ZNEAR / (ZFAR - wsDepth * (ZFAR - ZNEAR));

	return wsDepth * length(vec3((uv * 2.0 - 1.0) * nearPlaneHalfSize, -1.0));
}

float rand(vec2 co)
{
  return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

void main(void)
{
	// derivative simplex noise
	vec2 grad;
	grad.x = snoise(w_vertex.xz);
	grad.y = snoise(w_vertex.xz * 0.5);

	vec3 Normal = vec3(0.0, 1.0, 0.0);

	float vertdist = length(v_pos);

	vec3 vNormal = Normal; //mat3(matview) * Normal;
	#define viewNormal  v_normal

	// normalize inputs (water planes are complex)
	vec3 vEye   = v_pos / vertdist;
	vec3 vLight = v_ldir;
	vec3 vHalf = normalize(vEye + vLight);

	// screenspace tex coords
	vec2 texCoord = gl_FragCoord.xy / screendata.xy;
	// normalized distance
	float dist = vertdist / ZFAR;

	//----- REFRACTION -----

	// wave modulation
	vec2 refcoord = texCoord + vNormal.xz * 0.005 * (1.0 - dist);
  //+ vec2(sin(frameCounter*0.1 + sin(texCoord.x*60.0)), sin(frameCounter*0.1 + texCoord.y*80.0)*0.25) * 0.001;

	// read underwater, use as base color
	vec4 underw = texture(underwatermap, refcoord);
	// read underwater depth
	float wdepth = getDepth(refcoord) - dist;

	// COSTLY re-read to avoid reading inside terrain
	wdepth = max(0.0, wdepth);
	/*if (wdepth < 0.0)
	{
		wdepth = getDepth(texCoord) - dist;
	}*/
	// degamma underwater color
	underw.rgb = pow(underw.rgb, vec3(1.0 / 2.2));

	// nicer depth, adding a little extra
	wdepth += 5.0 / ZFAR;

	// above water we want the sky to appear as 100% seafloor
	const float DEPTH_TRESHOLD = 20.0 / ZFAR;
	float dep = 1.0 - smoothstep(0.0, DEPTH_TRESHOLD, wdepth);
	float shallowValue = min(1.0, wdepth * 16.0);

	//----- SEACOLOR -----
	const vec3 shallowWater = vec3(0.35, 0.6, 0.45);

	// create primary water color
  shallowValue *= waterColor.a;
  color.rgb = mix(shallowWater, waterColor.rgb, shallowValue) * daylight;
	color.a = 1.0;

	// mix watercolor and refraction/seabed
	color.rgb = mix(color.rgb, underw.rgb, dep);

	//----- REFLECTIONS -----

#ifdef REFLECTIONS
	// world/terrain reflection
  vec3 wreflection = texture(reflectionmap, texCoord).rgb;
	wreflection = pow(wreflection, vec3(1.0 / 2.2));

	//----- fresnel term -----
	float fresnel = max(0.0, dot(vEye, viewNormal));
	fresnel = pow(1.0 - fresnel, 2.0);

	// add reflections to final color
	color.rgb = mix(color.rgb, wreflection, fresnel);
	//color.rgb = vec3(fresnel);
#endif

	//----- LIGHTING -----
	#include "lightw.glsl"

	//- sun/specular -//
	const vec3  SUNCOLOR = vec3(0.9, 0.7, 0.5);

	float shine = max(0.0, dot(vHalf, viewNormal) );
	float specf = max(0.0, dot(reflect(-vLight, viewNormal), vEye));

	// shiny/specular sun reflection
	vec3 specular = SUNCOLOR * pow(specf, 20.0) * 1.5 * pow(shine, 8.0);
	color.rgb += specular * shadow * shadow;

	// final color
	#include "finalcolor.glsl"

#ifdef VIEW_NORMALS
	normals = vec4(viewNormal, 1.0);
#endif
}

#endif
