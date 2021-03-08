#define SUPERFOG
#ifdef SUPERFOG

void superFog(inout vec3 color,  // color of the pixel
              in float distance, // camera to point distance
              in vec3  rayDir,   // camera to point vector
              in vec3  sunDir )  // sun light direction
{
	const float FOG_MAX  = 0.65;
	const float FOG_DENS = 2.5;
	const float SUN_MULT = 1.1;
	const vec3 fogBaseColor = vec3(0.75);
	const vec3 sunBaseColor = vec3(1.0, 0.9, 0.7);
	
	float sunAmount = max( -dot( rayDir, sunDir ), 0.0 ) * SUN_MULT;
	
	const float euler = 2.718281828;
	float fogAmount = pow(euler, -pow(distance * FOG_DENS, 2.0));
	fogAmount *= FOG_MAX;
	
	vec3  fogColor  = mix(	fogBaseColor, // bluish
							sunBaseColor, // yellowish
							pow(sunAmount, 8.0) );
	
	color = mix(color, fogColor * daylight, fogAmount);
}

#endif
