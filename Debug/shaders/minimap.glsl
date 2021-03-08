#version 130
#define VERTEX_PROGRAM
#define FRAGMENT_PROGRAM
precision mediump float;

#ifdef VERTEX_PROGRAM
uniform mat4 matprojview;
uniform vec2 offset;

in vec3 in_vertex;

out vec2 vertex;
out vec2 texCoord;

void main(void)
{
	vertex = in_vertex.xy;
	texCoord = vertex + offset;
	gl_Position = matprojview * vec4(in_vertex, 1.0);
}
#endif

#ifdef FRAGMENT_PROGRAM
uniform sampler2D texture;

in vec2 vertex;
in vec2 texCoord;

const float visibility = 0.9;

void main(void)
{
	vec4 color = texture2D(texture, texCoord);
	//color.rgb = pow(color.rgb, vec3(2.2));
	
	float dist = 1.0 - min(1.0, length(vertex) / 0.5);
	// black dot in center of minimap
	color.rgb *= 1.0 - pow(dist, 80.0);
	// edge around minimap
	color.a *= smoothstep(0.0, 0.1, dist);
	// visibility setting
	color.a *= visibility;
	
	gl_FragColor = color;
}
#endif
