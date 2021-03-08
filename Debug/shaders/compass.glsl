#version 130
#define VERTEX_PROGRAM
#define FRAGMENT_PROGRAM
precision mediump float;

#ifdef VERTEX_PROGRAM
uniform mat4 mvp;
uniform vec3 spawn;

in vec3 in_vertex;
in vec2 in_texture;
in vec4 in_color;

out vec2 position;
out vec2 texCoord;
flat out vec2 direction;

void main(void)
{
	// TODO: 2d rotation matrix here
	direction = normalize(spawn.xz);
	
	position = in_vertex.xy;
	texCoord = in_texture;
	gl_Position = mvp * vec4(in_vertex, 1.0);
}
#endif

#ifdef FRAGMENT_PROGRAM
uniform sampler2D texture;

in vec2 position;
in vec2 texCoord;
flat in vec2 direction;

const vec3 arrowColor = vec3(0.75, 1.0, 0.75);

void main(void)
{
	float rad = max(0.0, 1.0 - length(position) / 0.5);
	
	// create arrow pointing towards spawn
	vec2 radix = normalize(-position);
	float arrow = max(0.0, -dot(direction, radix));
	
	// get color from compass texture
	vec4 color = texture2D(texture, texCoord);
	
	arrow = max(0.0, pow(arrow, 64.0) - color.a * 0.33);
	
	color.rgb = mix(color.rgb, arrowColor, arrow);
	// fade out insides
	color.a *= smoothstep(0.4, 1.0, 1.0 - rad) * 0.25;
	// arrow -> spawn
	color.a += 2.0 * arrow * rad * (1.0 - rad);
	
	gl_FragData[0] = color;
}
#endif
