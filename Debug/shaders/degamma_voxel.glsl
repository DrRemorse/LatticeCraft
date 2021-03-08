//
// degamma (voxels)
//
//#define POSTPROCESS

//#ifdef POSTPROCESS
	
	const vec3 degamma = vec3(1.0 / 2.2);
	
	// degamma
	out_color.rgb = pow(in_color.rgb, degamma);
	
//#endif
