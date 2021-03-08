#version 130
// uniform sampler2D texture;
// in vec4 texCoord;
in vec4 colordata;

void main(void) {
	
	// vec4 color = texture2D(texture, texCoord.st);
	// if (color[3] < 0.1) discard; //le discard!
	
	gl_FragColor = colordata;
	
}
