/**
 * Screenspace Ambient Occlusion (sux)
 * 
**/

vec3 getNormal(in vec2 uv)
{
	return normalize(texture(normalstex, uv).xyz);
}

const int SAMPLE_COUNT = 16;
const vec2 poisson16[] = vec2[](
	vec2( -0.94201624,  -0.39906216 ),
	vec2(  0.94558609,  -0.76890725 ),
	vec2( -0.094184101, -0.92938870 ),
	vec2(  0.34495938,   0.29387760 ),
	vec2( -0.91588581,   0.45771432 ),
	vec2( -0.81544232,  -0.87912464 ),
	vec2( -0.38277543,   0.27676845 ),
	vec2(  0.97484398,   0.75648379 ),
	vec2(  0.44323325,  -0.97511554 ),
	vec2(  0.53742981,  -0.47373420 ),
	vec2( -0.26496911,  -0.41893023 ),
	vec2(  0.79197514,   0.19090188 ),
	vec2( -0.24188840,   0.99706507 ),
	vec2( -0.81409955,   0.91437590 ),
	vec2(  0.19984126,   0.78641367 ),
	vec2(  0.14383161,  -0.14100790 ));

float getAO16(in vec3 viewPos, in float depth)
{
	float pixelSize = max(4.0, pow(1.0 - depth, 8.0) * 16.0);
	vec2 filterRadius = pixelSize / screenSize.xy;
	float distanceThreshold = 0.2 / ZFAR;
	
	// get the view space normal
	vec3 viewNormal = getNormal(texCoord);
	
    // perform AO
    float ambientOcclusion = 0.0;
	
    for (int i = 0; i < SAMPLE_COUNT; ++i)
    {
        vec2 sampleUV = texCoord + poisson16[i] * filterRadius;
        // sample at an offset specified by the current Poisson-Disk sample and scale it by a radius (has to be in Texture-Space)
		vec3 samplePos = getPosition(sampleUV);
        // distance between SURFACE-POSITION and SAMPLE-POSITION
		float VPdistSP = distance(samplePos, viewPos);
        // direction between SURFACE-POSITION and SAMPLE-POSITION
		vec3 sampleDir = (samplePos - viewPos) / VPdistSP;
		
        // angle between SURFACE-NORMAL and SAMPLE-DIRECTION (vector from SURFACE-POSITION to SAMPLE-POSITION)
        float NdotS = max(0.0, dot(viewNormal, sampleDir));
		
        // a = distance function
        float a = 1.0 - smoothstep(0.5 / ZFAR, 1.0 / ZFAR, VPdistSP / ZFAR);
		//float a = 1.0 - step(5.0, VPdistSP);
        ambientOcclusion += a * NdotS;
    }
    return ambientOcclusion / SAMPLE_COUNT;
}
