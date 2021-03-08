
// pixel-based dot-light
vec3  WLfaceVector = (gl_FrontFacing) ? out_normal.xyz : vec3(-out_normal.x, out_normal.yz);
float worldLight = 0.25 * dot(WLfaceVector, lightVector.xyz) + 0.75;
