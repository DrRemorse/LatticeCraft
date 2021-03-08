//
// final color, gamma (voxels)
//
#define POSTPROCESS

#ifndef POSTPROCESS
	
	//  no postprocess --> go back to gamma space
	const vec3 gamma = vec3(2.2);
	
	// back to gamma space
	out_color.rgb = pow(out_color.rgb, gamma);
	
#endif
