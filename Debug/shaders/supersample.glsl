#version 150
#define VERTEX_PROGRAM
#define FRAGMENT_PROGRAM

#ifdef VERTEX_PROGRAM
in  vec2 in_vertex;
out vec2 texCoord;

void main(void)
{
	texCoord = in_vertex;
	gl_Position = vec4(in_vertex * 2.0 - 1.0, 0.0, 1.0);
}
#endif

#ifdef FRAGMENT_PROGRAM
uniform sampler2D colorbuffer;
uniform int       samples;
uniform vec2      offsets;

in  vec2 texCoord;
out vec4 color;

void main()
{
	// base color
	//color = texture(colorbuffer, texCoord + offsets * 0.5);
	color = vec4(0.0);
	
	for (int y = 0; y < samples; y++)
	for (int x = 0; x < samples; x++)
	{
		color += texture2D(colorbuffer, texCoord + vec2(x, y) * offsets);
	}
	color /= float(samples * samples);
}

#endif
