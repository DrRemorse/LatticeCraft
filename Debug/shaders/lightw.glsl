//
// Water lighting
//

float skylight   = lightdata.r;
float brightness = lightdata.g;

// scaled shadows
float shadow = min(1.0, skylight + brightness);
shadow = 0.4 + 0.6 * shadow;

color.rgb *= shadow;
