#version 130
precision mediump float;

uniform sampler2D texture;
uniform sampler2D depthtexture;
uniform sampler2D suntexture;

in vec2 in_texture;
in vec4 sunproj;

void main(void)
{
	
	// god rays
	const int GodRays = 128;
	float Intensity = 0.125, Decay = 0.96875;
	vec2 Direction = (sunproj.xy - in_texture) / GodRays;
	vec2 TexCoord = in_texture;
	
	vec3 color = texture2D(texture, in_texture).rgb;
	
	for(int Sample = 0; Sample < GodRays; Sample++)
	{
		color += texture2D(texture, TexCoord).rgb * Intensity;
		Intensity *= Decay;
		TexCoord += Direction;
	}
	
	gl_FragColor = vec4(color, 1.0);
}
