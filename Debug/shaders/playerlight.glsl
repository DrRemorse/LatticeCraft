//
// player-held lights
//
#define PLAYERLIGHT

#ifdef PLAYERLIGHT
	
	// L = reach - dist
	// alpha = (L * L / falloff) ^ emission
	
	// reach = 0, falloff = 1, emission = 2, brightness = 3
	
	float ldist = max(0.0, 1.0 - vertdist / playerLight[0]);
	ldist = pow(ldist, playerLight[1]);
	
	brightness += ldist * playerLight[2] * modulation;
	
#endif
