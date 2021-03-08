
int light = int(in_texture.w);
lightdata = vec3(float(light & 0xFF) / 255.0, float((light >> 8) & 0xFF) / 255.0, in_normal.w);
