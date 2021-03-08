#version 150
#define VERTEX_PROGRAM
#define FRAGMENT_PROGRAM

#ifdef VERTEX_PROGRAM
uniform mat4 matmvp;
uniform vec3 vtrans;
uniform float miningTile;

in vec3 in_vertex;
in vec2 in_texture;

out vec3 texCoord;

void main()
{
	vec4 position = vec4(in_vertex.xyz + vtrans, 1.0);
	gl_Position = matmvp * position;
	
	texCoord = vec3(in_texture, miningTile);
}
#endif

#ifdef FRAGMENT_PROGRAM
#extension GL_EXT_gpu_shader4 : enable
uniform sampler2DArray diffuse;

in vec3 texCoord;
out vec4 color;

void main(void)
{
	color = texture(diffuse, texCoord);
}
#endif
