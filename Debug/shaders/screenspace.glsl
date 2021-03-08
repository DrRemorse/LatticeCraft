#version 150
#define VERTEX_PROGRAM
#define FRAGMENT_PROGRAM
#define LENSFLARE

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
uniform sampler2D terrain;
uniform sampler2D lensflare;

uniform float frameCounter;
uniform float daylight;
uniform int   submerged;

in  vec2 texCoord;
out vec4 color;

const float ZFAR
const float ZNEAR

void main()
{
	color = texture(terrain, texCoord);
	float depth = color.a;
	
	if (submerged != 0)
	{
		// re-read waved texcoord
		float speed = frameCounter * 0.01;
		
		// camera bobbing
		vec2 waves = vec2(sin(speed), cos(speed)) * 0.01;
		// depth-based waves
		vec2 waveCoords = vec2(speed * 5) + texCoord.xy * 6.28 * 32;
		float wavyDepth = min(0.25, depth * ZFAR / 20.0);
		waves += vec2(cos(waveCoords.x), sin(waveCoords.y)) * wavyDepth * 0.008;
		
		color = texture(terrain, texCoord + waves);
		depth = color.a;
		
		if (submerged == 1)
		{
			float wdepth = depth + 2.0 / ZFAR;
			
			const vec3 deepWater    = pow(vec3(42, 73, 87) * vec3(1.0 / 255.0), vec3(2.2));
			const vec3 shallowWater = pow(vec3(0.35, 0.6, 0.45), vec3(2.2));
			
			float shallowValue = min(1.0, wdepth * 16.0);
			vec3 waterColor = mix(shallowWater, deepWater, shallowValue) * daylight;
			
			const float DEPTH_TRESHOLD = 30.0 / ZFAR;
			float dep = smoothstep(0.0, DEPTH_TRESHOLD, wdepth);
			
			// submerged in water
			color.rgb = mix(color.rgb, waterColor, dep) * 0.7;
		}
		else
		{
			float wdepth = depth + 2.0 / ZFAR;
			
			const float DEPTH_TRESHOLD = 10.0 / ZFAR;
			float dep = smoothstep(0.0, DEPTH_TRESHOLD, wdepth);
			
			const vec3 lavaColor  = vec3(0.28, 0.06, 0.0);
			
			// submerged in lava
			color.rgb = mix(color.rgb, lavaColor, dep);
		}
	}
	
	// fade-in
	/*
	const vec3 luma = vec3(0.299, 0.587, 0.114);
	float luminance = dot(luma, color.rgb);
	float fadein    = min(1.0, frameCounter / 250.0);
	
	color.rgb = mix(vec3(luminance), color.rgb, fadein);
	*/
	
	// degamma ramp
	//color.rgb = pow(color.rgb, vec3(2.2));
	
#ifdef LENSFLARE
	// add lens flare & dirt
	color.rgb += texture(lensflare, texCoord).rgb;
#endif
	
}

#endif
