#version 130

in vec4 out_color;

void main(void) {
	
	gl_FragData[0] = vec4(out_color.rgb, 1.0);
	
}
