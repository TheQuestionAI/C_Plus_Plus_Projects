#version 460 core
in vec2 texCoords;

uniform vec3 color;
uniform sampler2D sprite;

out vec4 fragColor;

void main()
{
	fragColor = vec4(color, 1.0f) * texture(sprite, texCoords);
}
