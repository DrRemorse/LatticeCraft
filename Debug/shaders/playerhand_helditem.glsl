#version 130
#define VERTEX_PROGRAM
#define FRAGMENT_PROGRAM

#ifdef VERTEX_PROGRAM
uniform mat4 matproj;
uniform mat4 matview;
uniform mat4 matrot;

uniform vec3 lightVector;

in vec3 in_vertex;
in vec3 in_normal;
in vec3 in_texture;

out vec3 texCoord;
flat out float worldLight;

const float VERTEX_SCALE

void main()
{
	texCoord = vec3(in_texture.st / VERTEX_SCALE, in_texture.p);
	vec3 normal = in_normal * mat3(matrot);
	
	float dotl = dot(normal, lightVector);
	
	const float ambience = 0.75;
	worldLight  = ambience + dotl * (1.0 - ambience);
	
	gl_Position = matproj * matview * vec4(in_vertex / VERTEX_SCALE, 1.0);
}
#endif

#ifdef FRAGMENT_PROGRAM
#extension GL_EXT_gpu_shader4 : enable

uniform sampler2DArray texture;

uniform float daylight;
uniform vec2  lightdata;
uniform float modulation;

in vec3 texCoord;
flat in float worldLight;

void main(void)
{
	/// shadows & torchlight ///
	float brightness = lightdata.g;
	// shadow is smallest between shadow-value and daylight level
	float shadow = min(1.0, min(daylight, lightdata.r) + brightness);
	
	vec3 color = texture2DArray(texture, texCoord).rgb;
	#include "degamma.glsl"
	
	color.rgb *= worldLight; // min(1.0, worldLight + 0.35 * brightness)
	// mix in shadows
	color.rgb *= shadow;
	
	// back to gamma space
	const vec3 gamma = vec3(2.2);
	gl_FragColor = vec4(pow(color.rgb, gamma), 1.0);
}

#endif
