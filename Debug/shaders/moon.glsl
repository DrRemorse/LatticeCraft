#version 150
#define VERTEX_PROGRAM
#define FRAGMENT_PROGRAM

#ifdef VERTEX_PROGRAM
uniform mat4 matproj;
uniform mat4 matview;
uniform mat4 matrot;

in vec3 in_vertex;

out vec2 texCoord;
out vec3 norm;

void main(void)
{
	texCoord = in_vertex.xy + vec2(0.5);
	
	vec4 position = matview * vec4(in_vertex.xyz, 1.0);
	norm = normalize(position.xyz * mat3(matrot));
	
	gl_Position = matproj * position;
}

#endif

#ifdef FRAGMENT_PROGRAM
uniform sampler2D texture;

in  vec2 texCoord;
in  vec3 norm;
out vec4 color;

void main(void)
{
	// limit moon to horizon
	float horizon = clamp(-norm.y / 0.025,  0.0, 1.0);
	
	//vec2 tex = pow(texCoord.st, vec2(1.0, 1.0 + horizon));
	
	color = texture2D(texture, texCoord.st);
	// make invisible below horizon
	color.a *= pow(1.0 - horizon, 2.0);
	
	//color.rgb += horizon * 0.5;
}

#endif
