#version 150
#define VERTEX_PROGRAM
#define FRAGMENT_PROGRAM

#ifdef VERTEX_PROGRAM
in  vec2 in_vertex;
out lowp vec2 texCoord;

void main(void)
{
	texCoord = in_vertex;
	gl_Position = vec4(in_vertex * 2.0 - 1.0, 0.0, 1.0);
}
#endif

#ifdef FRAGMENT_PROGRAM
uniform sampler2D terrain;
uniform vec2 offset;

in  lowp vec2 texCoord;
out vec4 color;

void main()
{
	// terrain color
	color = texture(terrain, texCoord);

	const int   WIDTH = 3;
	const float MAXW = length(vec2(WIDTH));
	vec4  blur = vec4(0.0);
	float sum  = 0.0;

	for (int x = -WIDTH; x <= WIDTH; x++)
	for (int y = -WIDTH; y <= WIDTH; y++)
	{
		vec2 pos = vec2(float(x), float(y));
		vec4 c = texture(terrain, texCoord + offset * pos);

		float width = MAXW - length(pos);
		width *= 1.0 - abs(color.a - c.a);

		blur += c * width;
		sum += width;
	}
	blur /= sum;

	// depth from blur
	float depth = blur.a;
	depth = smoothstep(0.5, 0.95, depth) * step(depth, 0.997);

	// blur based on depth
	color = mix(color, blur, depth);
}

#endif
