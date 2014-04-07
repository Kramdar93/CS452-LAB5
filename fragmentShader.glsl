#version 130

in vec3 color;
in vec2 texPos;

out vec4 fColor;

uniform sampler2D texSampler;

void main () {
	// pass through the given color
	 fColor = vec4(color, 1.0f);

	//solid color
	//fColor = vec4(0.5f, 0.5, 0.5f, 1.0f);

	//texture
	fColor *= texture2D(texSampler, texPos);
}
