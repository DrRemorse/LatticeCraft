in vec2 texcoordFS;

uniform sampler2D normalTexture;
uniform sampler2D depthTexture;

uniform mat4 invProjection;

const float ZNEAR
const float ZFAR

out vec4 fragColor;

const float distanceThreshold;
const float filterRadius;

const int sample_count = 16;
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

vec3 decodeNormal(in vec2 normal)
{
	// restore normal
	return restoredNormal;
}

vec3 calulatePosition(in vec2 coord, in float depth)
{
	// restore position
	return restoredDepth;
}

void main()
{
	// reconstruct position from depth, USE YOUR CODE HERE
	float depth = texture(depthTexture, texcoordFS).r;
	vec3 viewPos = calulatePosition(texcoordFS, depth);
	
	// get the view space normal, USE YOUR CODE HERE
	vec2 normalXY = texture(normalTexture, texcoordFS).xy * 2.0 - 1.0;
	vec3 viewNormal = decodeNormal(normalXY);
	
    float ambientOcclusion = 0.0;
    // perform AO
    for (int i = 0; i < sample_count; ++i)
    {
        // sample at an offset specified by the current Poisson-Disk sample and scale it by a radius (has to be in Texture-Space)
        vec2 sampleTexCoord = texcoordFS + (poisson16[i] * (filterRadius));
        float sampleDepth = texture(depthTexture, sampleTexCoord).r;
        vec3 samplePos = calculatePosition(sampleTexCoord, sampleDepth * 2 - 1);
        vec3 sampleDir = normalize(samplePos - viewPos);
		
        // angle between SURFACE-NORMAL and SAMPLE-DIRECTION (vector from SURFACE-POSITION to SAMPLE-POSITION)
        float NdotS = max(dot(viewNormal, sampleDir), 0);
        // distance between SURFACE-POSITION and SAMPLE-POSITION
        float VPdistSP = distance(viewPos, samplePos);
		
        // a = distance function
        float a = 1.0 - smoothstep(distanceThreshold, distanceThreshold * 2, VPdistSP);
        // b = dot-Product
        float b = NdotS;
		
        ambientOcclusion += a * b;
    }
    fragColor.a = 1.0 - (ambientOcclusion / sample_count);
}
