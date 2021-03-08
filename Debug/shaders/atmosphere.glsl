#version 150
#define VERTEX_PROGRAM
#define FRAGMENT_PROGRAM

#ifdef VERTEX_PROGRAM
uniform mat4 matmvp;
uniform vec3 v3LightPos; // The direction vector to the light source
uniform float above;     // 1.0 above horizon, 0.0 below horizon

const float PI = 3.1415926;

// 1 / pow(wavelength, 4) for the red, green, and blue channels
const vec3 v3Wavelength = pow(vec3(0.650, 0.570, 0.475), vec3(4.0));
const vec3 v3InvWavelength = vec3(1.0) / v3Wavelength;

const float Kr   = 0.0025;	// Rayleigh scattering constant
const float Km   = 0.0025;	// Mie scattering constant
const float ESun = 15.0;	// Sun brightness constant

const float fKrESun = Kr * ESun;
const float fKmESun = Km * ESun;
const float fKr4PI = Kr * 4.0 * PI;
const float fKm4PI = Km * 4.0 * PI;

const float fInnerRadius = 10.0;  // The inner (planetary) radius
const float fOuterRadius = 10.25; // The outer (atmosphere) radius
const float fRayleighScaleDepth = 0.25; // The scale depth (i.e. the altitude at which the atmosphere's average density is found)

const float fScale = 1.0 / (fOuterRadius - fInnerRadius);
const float fScaleDepth = fRayleighScaleDepth;
const float fScaleOverScaleDepth = fScale / fScaleDepth;

const vec3  v3CameraPos   = vec3(0.0, fInnerRadius, 0.0); // The camera's current position
const float fCameraHeight = fInnerRadius; // The camera's current height

#define nSamples 4
#define fSamples 4.0

in vec3 in_vertex;

out vec3 v3x;
out vec3 v3Direction;
out vec3 color_rayleigh;
out vec3 color_mie;
out vec3 starc;

float scale(float fCos)
{
	float x = 1.0 - fCos;
	return fScaleDepth * exp(-0.00287 + x*(0.459 + x*(3.83 + x*(-6.80 + x*5.25))));
}

void main(void)
{
	v3x = in_vertex.xyz - v3CameraPos;
	v3x.y = v3x.y * above + 0.001 * v3x.y * (1.0 - above);

	v3Direction = v3CameraPos - in_vertex.xyz;

	// Get the ray from the camera to the vertex, and its length (which is the far point of the ray passing through the atmosphere)
	vec3 v3Ray = v3x;
	float fFar = length(v3Ray);
	v3Ray /= fFar;

	// Calculate the ray's starting position, then calculate its scattering offset
	vec3 v3Start = v3CameraPos;
	float fHeight = length(v3Start);
	float fDepth = exp(fScaleOverScaleDepth * (fInnerRadius - fCameraHeight));
	float fStartAngle = dot(v3Ray, v3Start) / fHeight;
	float fStartOffset = fDepth * scale(fStartAngle);

	// Initialize the scattering loop variables
	float fSampleLength = fFar / fSamples;
	float fScaledLength = fSampleLength * fScale;
	vec3 v3SampleRay = v3Ray * fSampleLength;
	vec3 v3SamplePoint = v3Start + v3SampleRay * 0.5;

	// Now loop through the sample rays
	vec3 v3FrontColor = vec3(0.0);
	for(int i = 0; i < nSamples; i++)
	{
		float fHeight = length(v3SamplePoint);
		float fDepth = exp(fScaleOverScaleDepth * (fInnerRadius - fHeight));
		float fLightAngle = dot(v3LightPos, v3SamplePoint) / fHeight;
		float fCameraAngle = dot(v3Ray, v3SamplePoint) / fHeight;
		float fScatter = (fStartOffset + fDepth*(scale(fLightAngle) - scale(fCameraAngle)));
		vec3 v3Attenuate = exp(-fScatter * (v3InvWavelength * fKr4PI + fKm4PI));
		v3FrontColor += v3Attenuate * (fDepth * fScaledLength);
		v3SamplePoint += v3SampleRay;
	}

	// Finally, scale the Mie and Rayleigh colors and set up the varying variables for the pixel shader
	color_mie = v3FrontColor * fKmESun;
	color_rayleigh = v3FrontColor * (v3InvWavelength * fKrESun);
	// camera look direction
	starc = v3Ray;

	gl_Position = matmvp * vec4(in_vertex.xyz, 1.0);
}

#endif

#ifdef FRAGMENT_PROGRAM
uniform samplerCube skymap;
uniform sampler2D   starmap;

uniform float above;
uniform float sunAngle;
uniform float starBrightness;

uniform vec3 v3LightPos;
const float g  = -0.50; // The Mie phase asymmetry factor
const float g2 = g * g;

in vec3 v3x;
in vec3 v3Direction;
in vec3 color_rayleigh;
in vec3 color_mie;
in vec3 starc;

out vec4 color;

void main (void)
{
	float fCos = dot(v3LightPos, -v3x) / length(v3x);
	float fRayleighPhase = 0.75 * (1.0 + fCos*fCos);
	float fMiePhase = 1.5 * ((1.0 - g2) / (2.0 + g2)) * (1.0 + fCos*fCos) / pow(1.0 + g2 - 2.0*g*fCos, 1.5);

	color.rgb = fRayleighPhase * color_rayleigh + fMiePhase * color_mie;

	vec3 norm = normalize(v3Direction);
	norm.y *= (0.5 - above) * 2.0;

	vec3 skyColor = texture(skymap, norm).rgb;
	color.rgb = mix(color.rgb, skyColor, (color.b - color.r * 0.5) * 0.5);

	float darkness = max(0.0, 0.16 - length(color.rgb)) * 6.0;
	if (darkness > 0.05)
	{
		// stars
		const float PI = 3.1415926;
		vec2 coord = vec2(((atan(norm.y, norm.x) + sunAngle) / PI + 1.0) * 0.5, asin(norm.z) / PI + 0.5 );
		vec3 stars = texture(starmap, coord).rgb;
		stars = pow(stars, vec3(3.0)) * starBrightness;

		color.rgb = mix(color.rgb, stars, darkness * darkness);
	}

	//color.rgb = vec3(1.0) - exp(color.rgb * -2.0);
	color.a = 1.0;
}

#endif
