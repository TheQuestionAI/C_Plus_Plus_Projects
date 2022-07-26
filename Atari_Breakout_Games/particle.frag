#version 460 core
in vec2 texCoords;

uniform vec4 color;
uniform sampler2D image;

out vec4 fragColor;

void main()
{
	fragColor = color * texture(image, texCoords);
}
