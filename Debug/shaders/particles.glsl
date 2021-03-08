#version 150
#define VERTEX_PROGRAM
#define FRAGMENT_PROGRAM

#define POSTPROCESS

#ifdef VERTEX_PROGRAM
uniform mat4 matproj;
uniform mat4 matview;

uniform vec2 screensize;
uniform float fovRadians;

in vec3 in_vertex;
in vec4 in_data;
in vec4 in_normdata;
in vec4 in_color;

flat out float tileID;
flat out float uvsize;
flat out float magic;
flat out vec4 out_normdata;
flat out vec4 out_color;

void main(void)
{
	vec4 position = matview * vec4(in_vertex, 1.0);
	gl_Position = matproj * position;
	
	// transform size
	vec4 projsize = matproj * vec4(in_data.xx, position.zw);
	gl_PointSize = max(2.0, in_data.x / 64.0 * screensize.x / projsize.z);
	
	tileID = in_data.y;
	uvsize = in_data.z / 255.0;
	magic  = in_data.w / 255.0;
	out_normdata = in_normdata;
	out_color = in_color;
}
#endif

#ifdef FRAGMENT_PROGRAM
#extension GL_EXT_gpu_shader4 : enable

uniform sampler2DArray texture;
uniform float timeElapsed;

flat in float tileID;
flat in float uvsize;
flat in float magic;
flat in vec4 out_normdata;
flat in vec4 out_color;

out vec4 color;

void main(void)
{
	vec3 texCoord = vec3(gl_PointCoord.xy, tileID);
	texCoord.xy /= uvsize;          // uvscale
	texCoord.xy += out_normdata.zw; // (offsetX, offsetY)
	
	color = texture2DArray(texture, texCoord);
	
	#include "degamma.glsl"
	
	color.rgb = mix(color.rgb, out_color.rgb, out_color.a);
	color *= out_normdata.yyyx;
	
	// magical strobe
	vec2 magitex = abs(vec2(0.5) - gl_PointCoord.xy) * 2.0;
	vec3 magiColor = vec3(0.5) + sin(vec3(magitex.x, timeElapsed*0.03, magitex.y) * 3.0) * 0.5;
	color.rgb = mix(color.rgb, magiColor, magic);
	
	#include "finalcolor.glsl"
}
#endif
