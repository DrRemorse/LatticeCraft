#version 150
#define VERTEX_PROGRAM
#define FRAGMENT_PROGRAM

#ifdef VERTEX_PROGRAM
uniform vec2 nearPlaneHalfSize;

in  vec2 in_vertex;
out vec2 texCoord;
out vec3 eye_direction;

void main(void)
{
	texCoord = in_vertex;
	gl_Position = vec4(in_vertex * 2.0 - 1.0, 0.0, 1.0);

	eye_direction = vec3(gl_Position.xy * nearPlaneHalfSize, -1.0);
}
#endif

#ifdef FRAGMENT_PROGRAM
uniform sampler2D terrain;
uniform sampler2D skytexture;
uniform sampler2D depthtexture;
#ifdef AMBIENT_OCCLUSION
uniform sampler2D normalstex;
#endif

uniform vec3  sunAngle;
uniform float daylight;

uniform mat4 matview;
uniform vec3 worldOffset;
uniform float timeElapsed;
uniform vec2 nearPlaneHalfSize;
uniform vec2 screenSize;

uniform vec4  fogData;
uniform float fogHeight;

in  vec2 texCoord;
in  vec3 eye_direction;
out vec4 color;

const float ZFAR
const float ZNEAR
const float WATERLEVEL

float fog(in vec3  ray,
		  in vec3  point,
		  in float depth)
{
	// fog is strongest at the waterline
	const float fogY = WATERLEVEL;

	// distance in fog
	float foglen = sqrt(depth);

	// how far are we from center of fog?
	float foglevel = max(0.0, 1.0 - abs(point.y - fogY) / fogHeight);
	// make S-curve/gradient
	foglevel = smoothstep(0.0, 1.0, foglevel);
	// steeper curve under fog center (fogY)
	float above = 1.0 - step(point.y, fogY);
	foglevel = above * foglevel + (1.0 - above) * 1.0;

	return foglevel * foglen;
}

float linearDepth(in vec2 uv)
{
	float wsDepth = texture(depthtexture, uv).x;
	return ZNEAR / (ZFAR - wsDepth * (ZFAR - ZNEAR)) * ZFAR;
}
vec3 getPosition(in vec2 uv)
{
	vec3 viewPos = vec3((uv * 2.0 - 1.0) * nearPlaneHalfSize, -1.0);
	return viewPos * linearDepth(uv);
}

void main()
{
	// base color
	color = texture2D(terrain, texCoord);

	// reconstruct position from window-space depth
	vec3 viewPos = eye_direction * linearDepth(texCoord);
	// viewspace/linear depth
	float depth = min(1.0, length(viewPos.xyz) / ZFAR);

	// reconstruct view to world coordinates
	vec4 wpos = vec4(viewPos, 1.0) * matview;
	// camera->point ray
	vec3 ray = normalize(wpos.xyz);
	// to world coordinates
	wpos.xyz -= worldOffset;

	// volumetric fog
	float fogAmount = fog(ray, wpos.xyz, depth) * fogData.a;

	// mix in the sun effect
	const vec3 sunBaseColor = vec3(1.0, 0.8, 0.5);
	float sunAmount = max(0.0, dot(ray, sunAngle));
	sunAmount *= 0.5 * depth * daylight * daylight;
	// subtract from the fog where there is sun
	//fogAmount = max(0.0, fogAmount - sunAmount);

	// mix in sun glow on terrain only (BEFORE WE ADD SKY)
	color.rgb = mix(color.rgb, sunBaseColor, sunAmount);

  // mix in sky to fade out the world
  vec3 skyColor = texture(skytexture, texCoord).rgb;
  float edge = smoothstep(0.9, 0.99, depth);
  color.rgb = mix(color.rgb, skyColor, edge);

	// final fog color (AFTER sky, and after sun)
	color.rgb = mix(color.rgb, fogData.rgb, fogAmount);
	// alpha-channel is linearized depth
	color.a   = depth;
}

#endif
