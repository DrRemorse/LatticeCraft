//
// Standard lighting code snippet
//

float skylight   = lightdata.r;
float brightness = lightdata.g;

// shadow is smallest between skylight and daylight level
float shadow = min(1.0, daylight * skylight + brightness);
//shadow = 0.2 + 0.8 * shadow;

// corner shadows and dotlight
float cornershad = smoothstep(0.0, 1.0, lightdata.b);
cornershad = min(1.0, cornershad * worldLight + brightness * 0.1);

// final shadow applied
color.rgb *= cornershad * shadow;
//color.rgb = vec3(brightness);

const vec3 torchColor = vec3(1.0, 0.94, 0.7);
color.rgb = mix(color.rgb, torchColor, brightness * 0.45);
