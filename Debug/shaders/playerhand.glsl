#version 150
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

void main()
{
	texCoord = in_texture;
	vec3 normal = in_normal * mat3(matrot);
	
	float dotl = dot(normal, lightVector);
	
	const float ambience = 0.75;
	worldLight  = ambience + dotl * (1.0 - ambience);
	
	gl_Position = matproj * matview * vec4(in_vertex, 1.0);
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
out vec4 color;

void main(void)
{
	/// shadows & torchlight ///
	float brightness = lightdata.g;
	// shadow is smallest between shadow-value and daylight level
	float shadow = min(1.0, min(daylight, lightdata.r) + brightness);
	
	color = texture2DArray(texture, texCoord);
	#include "degamma.glsl"
	
	color.rgb *= worldLight;
	// mix in shadows
	color.rgb *= shadow;
	
	#include "finalcolor.glsl"
}

#endif
