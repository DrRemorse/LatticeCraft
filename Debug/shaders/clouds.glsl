#version 150
#define VERTEX_PROGRAM
#define FRAGMENT_PROGRAM

const float ZFAR
const float SKYFAR = ZFAR * 1.6;

#ifdef VERTEX_PROGRAM
uniform mat4 matproj;
uniform mat4 matview;

uniform float frameCounter;
uniform vec3  worldOffset;

in vec3 in_vertex;

out vec2 texCoord;
out float vertdist;
out vec3 v_eye;

void main()
{
	vec4 position = matview * vec4(in_vertex, 1.0);
	
	vec3 lpos = position.xyz * mat3(matview);
	// view/rotated vertex distance
	vertdist = length(lpos);
	
	v_eye = -position.xyz;
	
	texCoord = (lpos.xz - worldOffset.xz) / 48.0 / 8.0;
	// movement speed
	texCoord.s += frameCounter * 0.0001;
	
	gl_Position = matproj * position;
}

#endif

#ifdef FRAGMENT_PROGRAM
uniform sampler2D cloudstex;

uniform mat4 matview;
uniform vec3 v_ldir;
uniform float frameCounter;
uniform float daylight; // multiplier

in vec2 texCoord;
in float vertdist;

in  vec3 v_eye;
out vec4 color;

void main(void)
{
	vec3 vEye = normalize(v_eye);
	
	// normal-textured clouds
	vec4 normal = texture(cloudstex, texCoord);
	normal.rgb = normal.rgb * 2.0 - vec3(1.0);
	
	const vec3 in_normal  = vec3( 0.0,  1.0,  0.0);
	const vec3 in_tangent = vec3( 1.0,  0.0,  0.0);
	const vec3 binormal   = vec3( 0.0,  0.0, -1.0);
	
	const mat3 tbn = mat3(in_tangent, binormal, in_normal);
	normal.xyz = mat3(matview) * (normal.xyz * tbn);
	
	// convert to white + alpha
	color = vec4(vec3(1.0 - normal.w), 1.5 * normal.w);
	// sharply reduce left-over alpha
	color.a = pow(color.a, 6.0);
	color.a = smoothstep(0.0, 1.0, color.a);
	
	// lighten the insides a bit
	//color.rgb *= 1.9;
	color.rgb = pow(color.rgb, vec3(0.2));
	
	// blue
	color.b += (1.0 - color.b) * 0.15;
	
	// darken parts dense to player
	float away = -dot( reflect(vEye, normal.xyz), v_ldir );
	color.rgb += min(0.0, away * 0.25);
	
	// fade out to distance
	const float fadedist = SKYFAR * 0.5;
	
	float depthalpha = max(0.0, vertdist - fadedist);
	depthalpha = 1.0 - depthalpha / (SKYFAR - fadedist);
	
	color.a *= depthalpha;
	
	// sun -> clouds
	vec3 sunBaseColor = color.rgb * vec3(1.0, 0.9, 0.8);
	float sunAmount = max( -dot( vEye, v_ldir ), 0.0 ) * 1.3;
	color.rgb = mix(color.rgb, sunBaseColor, pow(sunAmount, 4.0) * 0.3);
	
	// daylight multipliers
	color.rgb *= daylight * daylight;
	color.a   *= daylight;
	
	color.rgb = pow(color.rgb, vec3(2.2));
}

#endif

