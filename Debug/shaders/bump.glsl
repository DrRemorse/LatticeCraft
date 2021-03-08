#ifdef BUMP_MAPPING
	if (vertdist < BUMP_REACH) {
		
		float cutoff = 1.0 - sqrt(vertdist / BUMP_REACH);
		
		#ifdef PARALLAX_OCCLUSION
			const float fbump = 0.05;
			const int   INTERVAL_POINTS = 10;
			
			// parallax interval mapping
			float fd = texture2DArray(normalmap, coord.stp).a;
			vec2 v_half = v_eye.xy * fd * fbump;
			
			for (int i = 0; i < INTERVAL_POINTS; i++) {
				fd = (fd + texture2DArray(normalmap, coord.stp + vec3(v_half, 0.0)).a) * 0.5;
				v_half = v_eye.xy * fd * fbump;
			}
			
			coord.st += v_half * cutoff;
		#endif
		
		color = texture2DArray(texture, coord.stp);
		
		/*
		#ifdef PARALLAX_OCCLUSION
			color.rgb *= 0.5 + fd * 0.5;
		#endif
		*/
		
		vec3 bump = 2.0 * texture2DArray(normalmap, coord.stp).rgb - vec3(1.0);
		
		float specular  = texture2DArray(specularmap, coord.stp).r;
		float shininess  = max(0.0, dot(reflect(-v_ldir, bump), v_eye));
		shininess += max(0.0, dot(reflect(-v_eye, bump), v_eye)) * 0.25;
		/*
		float self = 0.0;
		if (vertdist < 16.0) {
			self = max(0.0, dot(reflect(v_half, bump), -v_half));
			self = self * self * self * specular * (16.0 - vertdist) / 16.0;
		}
		*/
		
		// color.rgb *= 0.5 + max(dot(bump, v_ldir), 0.5);
		color.rgb += (shininess * shininess * specular) * daylight * shadow * cutoff;
		
 	} else {
#endif
