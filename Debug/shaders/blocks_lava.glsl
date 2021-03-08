#version 150
#define VERTEX_PROGRAM
#define FRAGMENT_PROGRAM

#ifdef VERTEX_PROGRAM
uniform mat4 matproj;
uniform mat4 matview;
uniform samplerBuffer buftex;

uniform vec3 worldOffset;

in vec4 in_vertex;
in vec4 in_normal;
in vec3 in_texture;
in vec4 in_biome;

out vec2 wave;

const float VERTEX_SCALE_INV
const float ZFAR

void main(void)
{
  vec3 translation = texelFetch(buftex, int(in_vertex.w)).xyz;
  vec4 position = vec4(in_vertex.xyz * VERTEX_SCALE_INV + translation, 1.0);

	position = matview * position;
	gl_Position = matproj * position;

	// transformed wave coordinates
	wave = (position.xyz * mat3(matview) - worldOffset).xz;
}

#endif

#ifdef FRAGMENT_PROGRAM
uniform sampler2D lavatex;

uniform float frameCounter;

in  vec2  wave;
out vec4 color;

void main(void)
{
	float timer = frameCounter / 50.0;
	vec2 subwave  = sin(timer * vec2(2.0) + wave * 0.5) * sin(timer);
	vec2 lavawave = wave * 0.25 + subwave * 0.01;

	// waving lava texture
	color = texture2D(lavatex, lavawave);
	color.rgb *= 0.4;

	// cheap sparkling
	color.r += pow(color.r, 1.0 / abs(subwave.x * subwave.y)) * 0.5;
}

#endif
