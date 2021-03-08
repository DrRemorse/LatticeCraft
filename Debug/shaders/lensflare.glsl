#version 130
#define VERTEX_PROGRAM
#define FRAGMENT_PROGRAM
precision mediump float;

#ifdef VERTEX_PROGRAM

in  vec2 in_vertex;
out vec2 in_texture;

void main(void)
{
	in_texture  = in_vertex;
	gl_Position = vec4(in_vertex * 2.0 - 1.0, 0.0, 1.0);
}
#endif

#ifdef FRAGMENT_PROGRAM
uniform sampler2D HighBlurredSunTexture;
uniform sampler2D LowBlurredSunTexture;
uniform sampler2D DirtTexture;

uniform float Dispersal, HaloWidth, Intensity;
uniform vec3 Distortion;
uniform vec2 sunproj;

in vec2 in_texture;

vec3 texture2DDistorted(sampler2D Texture, vec2 TexCoord, vec2 Offset)
{
	return vec3(
		texture2D(Texture, TexCoord + Offset * Distortion.x).r,
		texture2D(Texture, TexCoord + Offset * Distortion.y).g,
		texture2D(Texture, TexCoord + Offset * Distortion.z).b
	);
}

void main()
{
	vec3 RadialBlur = vec3(0.0);
	vec2 TexCoord = in_texture;
	const int RadialBlurSamples = 128;
	vec2 RadialBlurVector = (sunproj - in_texture) / RadialBlurSamples;
	
	for(int i = 0; i < RadialBlurSamples; i++)
	{
		RadialBlur += texture2D(LowBlurredSunTexture, TexCoord).rgb;
		TexCoord += RadialBlurVector;
	}
	
	RadialBlur /= RadialBlurSamples;
	
	vec3 LensFlareHalo = vec3(0.0);
	TexCoord = 1.0 - in_texture;
	vec2 LensFlareVector = (vec2(0.5) - TexCoord) * Dispersal;
	vec2 LensFlareOffset = vec2(0.0);
	
	for(int i = 0; i < 5; i++)
	{
		LensFlareHalo += texture2DDistorted(HighBlurredSunTexture, TexCoord, LensFlareOffset).rgb;
		LensFlareOffset += LensFlareVector;
	}
	
	LensFlareHalo += texture2DDistorted(HighBlurredSunTexture, TexCoord, normalize(LensFlareVector) * HaloWidth);
	LensFlareHalo /= 6.0;
	
	vec3 finalColor = texture2D(HighBlurredSunTexture, in_texture).rgb + 
		(RadialBlur + LensFlareHalo) * texture2D(DirtTexture, in_texture).rgb;
	
	gl_FragColor = vec4(finalColor * Intensity, 1.0);
}
#endif
