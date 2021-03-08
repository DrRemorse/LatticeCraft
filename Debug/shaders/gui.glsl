#version 130
#define VERTEX_PROGRAM
#define FRAGMENT_PROGRAM
precision mediump float;

#ifdef VERTEX_PROGRAM
uniform mat4 mvp;

in vec3 in_vertex;
in vec2 in_texture;
in vec4 in_color;

out vec2 texCoord;
out vec4 colordata;

void main()
{
	gl_Position = mvp * vec4(in_vertex, 1.0);
	texCoord = in_texture;
	colordata = in_color;
}
#endif

#ifdef FRAGMENT_PROGRAM
uniform sampler2D texture;

in vec2 texCoord;
in vec4 colordata;

const vec3 degamma = vec3(1.0 / 2.2);
const vec3 gamma   = vec3(2.2);

void main(void)
{
	vec4 color = texture2D(texture, texCoord);
	if (color.a == 0.0) discard; // le discard
	
	// degamma
	color.rgb = pow(color.rgb, degamma);
	
	color.rgb = mix(color.rgb, colordata.rgb, colordata.a);
	
	// gamma
	gl_FragData[0] = vec4(pow(color.rgb, gamma), color.a);
}
#endif
