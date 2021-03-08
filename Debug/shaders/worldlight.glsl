
// vertex-face-based dot-light
worldLight = 0.84 + 0.16 * dot(in_normal.xyz, lightVector.xyz);
// extra contribution from atmosphere
worldLight += 0.05 * in_normal.y;
