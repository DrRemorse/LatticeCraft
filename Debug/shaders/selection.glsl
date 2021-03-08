#version 150
#define VERTEX_PROGRAM
#define FRAGMENT_PROGRAM

#ifdef VERTEX_PROGRAM
uniform mat4 matmvp;
uniform vec3 vtrans;

in vec3 in_vertex;
in vec2 in_texture;

out vec2 texCoord;

void main()
{
	vec4 position = vec4(in_vertex.xyz + vtrans, 1.0);
	gl_Position = matmvp * position;
	
	texCoord = in_texture;
}
#endif

#ifdef FRAGMENT_PROGRAM
uniform sampler2D diffuse;

const vec3 selectionColor = vec3(0.0);

in vec2 texCoord;
out vec4 color;

void main(void)
{
	color = vec4(vec3(0.0), texture(diffuse, texCoord).r);
}
#endif
