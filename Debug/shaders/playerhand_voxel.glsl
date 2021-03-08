#version 130
#define VERTEX_PROGRAM
#define FRAGMENT_PROGRAM

#ifdef VERTEX_PROGRAM
uniform mat4 matmvp;
uniform mat4 matrot;
uniform mat4 matnrot;

uniform vec3 lightVector;

in vec3 in_vertex;
in vec3 in_normal;
in vec4 in_color;

flat out vec4  out_color;
flat out float worldLight;

void main()
{
	out_color = in_color;
	vec3 normal = (mat3(matnrot) * in_normal) * mat3(matrot);
	
	float dotl = dot(normal, lightVector);
	
	const float ambience = 0.75;
	worldLight  = ambience + dotl * (1.0 - ambience);
	
	gl_Position = matmvp * vec4(in_vertex, 1.0);
}
#endif

#ifdef FRAGMENT_PROGRAM
#extension GL_EXT_gpu_shader4 : enable

uniform float daylight;
uniform vec2  lightdata;
uniform float modulation;

flat in vec4  out_color;
flat in float worldLight;

void main(void)
{
	/// shadows & torchlight ///
	float brightness = lightdata.g;
	// shadow is smallest between shadow-value and daylight level
	float shadow = min(1.0, min(daylight, lightdata.r) + brightness);
	
	vec3 color = out_color.rgb;
	#include "degamma.glsl"
	
	color.rgb *= worldLight;
	// mix in shadows
	color.rgb *= shadow;
	
	// back to gamma space
	const vec3 gamma = vec3(2.2);
	gl_FragColor = vec4(pow(color.rgb, gamma), 1.0);
}

#endif
