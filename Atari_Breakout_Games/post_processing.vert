#version 460 core
layout (location = 0) in vec4 vertex;	// <vec2 position, vec2 texCoords>

/*
	The postprocessing shader allows for three type of effects: shake, confuse, and chaos.
		1. shake: slightly shakes the scene with a small blur.
		2. confuse: inverses the colors of the scene, but also the x and y axis.
		3. chaos: makes use of an edge detection kernel to create interesting visuals and also moves the textured image in a circular fashion for an interesting chaotic effect.
	shake�Ƕ�game object����Ϸ�����е�position���Ӱ��, confuse��chaos�Ƕ�game object������ͼ��ӳ�����Ӱ��.
*/
uniform bool chaos;
uniform bool confuse;
uniform bool shake;
uniform float time;		// ����Ԥ�������������Ҫ��time��Ϊ����.

out vec2 texCoords;


void main() 
{	// 2D game, ֱ�Ӿ��Ǵ���������ʹ������ͶӰ�Ѷ���ͶӰ��NDC������.
	gl_Position = vec4(vertex.xy, 0.0f, 1.0f);		// ֱ�ӽ����㶨�嵽(-1,-1), (-1,1), (1,-1), (1,1)�ĸ�����, �����κα任.
	
	vec2 coords = vertex.zw;
	if (chaos)			// ���ñ�Ե�����������Ȥ���Ӿ�Ч��, ����Բ�η�ʽ�ƶ�����ͼ���Ի����Ȥ�Ļ���Ч��.
	{	// ��Ϊ���ǽ�����wapper��������Ϊ GL_REPEAT, ����Ч���ᵼ�³������ı��εĸ��������ظ�.
		float strength = 0.3f;
		texCoords = vec2(coords.x + sin(time) * strength, coords.y + cos(time) * strength);

		//texCoords = coords;
	}
	else if (confuse)	// ��ת��������ɫ, ��Ҳ��ת x �� y ��
	{
		texCoords = vec2(1.0f - coords.x, 1.0f - coords.y);
	}
	else				// �޺���Ч��, ��ֱ����coords.
	{
		texCoords = coords;
	}

	// ����������shake��chaos����confuse����ͬʱ����, ����chaos��confuse������ͬʱ����.
	if (shake)			// ��΢����Ϸ������ģ������
	{	// ���shake ����Ϊtrue, ������΢�ƶ�����λ��, ��ͺ�������Ļ�ڶ���һ��.
		float strength = 0.01;
		gl_Position.x += cos(time * 10) * strength;
		gl_Position.y += cos(time * 15) * strength;
	}
}


