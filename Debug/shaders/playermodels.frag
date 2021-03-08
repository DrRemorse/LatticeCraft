#version 130
#extension GL_EXT_gpu_shader4 : enable

uniform sampler2DArray texture;
uniform float daylight;

in vec3 texCoord;
in vec4 colordata;

flat in float worldLight;
in float brightness;


void main(void) {

	vec4 color = texture2DArray(texture, texCoord);
	if (color.a < 0.1) discard; // le discard!
	
	#include "degamma.glsl"
	
	// shadows & torchlight
	color.rgb *= worldLight * min(1.0, daylight + brightness);
	color.rgb = mix(color.rgb, colordata.rgb, max(0.0, colordata.a - brightness));
	
	#include "finalcolor.glsl"
}
