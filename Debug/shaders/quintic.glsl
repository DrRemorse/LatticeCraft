#ifdef QUINTIC

uniform vec4 tiles;

void quinticCoords(inout vec3 p) {
	// by iñigo quilez, 2009
	p.st = p.st * tiles[3] + 0.5;
	
	vec2 i = floor(p.st);
	vec2 f = p.st - i;
	f = f*f*f*(f*(f*6.0-15.0)+10.0);
	p.st = i + f;
	
	p.st = (p.st - 0.5) / tiles[3];
}

#endif
