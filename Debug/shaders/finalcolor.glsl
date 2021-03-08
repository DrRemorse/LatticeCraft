//
// Final color code snippet
//
#define USE_GAMMA

#ifdef USE_GAMMA
	
	const vec3 gamma = vec3(2.2);
	
	// back to gamma space
	color.rgb = pow(color.rgb, gamma);
	
#endif
