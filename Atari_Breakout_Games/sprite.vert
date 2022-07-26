#version 460 core
layout (location = 0) in vec4 vertex;

uniform mat4 model;
uniform mat4 projection;

out vec2 texCoords;

void main()
{
	// 2D game, ֱ�Ӵ���������ʹ������ͶӰ�Ѷ���ͶӰ��NDC������.
	gl_Position = projection * model * vec4(vertex.xy, 0.0f, 1.0f);
	texCoords = vertex.zw;
}

