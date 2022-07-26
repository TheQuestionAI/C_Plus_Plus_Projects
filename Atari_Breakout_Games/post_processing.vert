#version 460 core
layout (location = 0) in vec4 vertex;	// <vec2 position, vec2 texCoords>

/*
	The postprocessing shader allows for three type of effects: shake, confuse, and chaos.
		1. shake: slightly shakes the scene with a small blur.
		2. confuse: inverses the colors of the scene, but also the x and y axis.
		3. chaos: makes use of an edge detection kernel to create interesting visuals and also moves the textured image in a circular fashion for an interesting chaotic effect.
	shake是对game object在游戏场景中的position造成影响, confuse和chaos是对game object的纹理图像映射造成影响.
*/
uniform bool chaos;
uniform bool confuse;
uniform bool shake;
uniform float time;		// 所有预处理操作都必须要有time作为参数.

out vec2 texCoords;


void main() 
{	// 2D game, 直接就是从世界坐标使用正交投影把顶点投影到NDC坐标下.
	gl_Position = vec4(vertex.xy, 0.0f, 1.0f);		// 直接将顶点定义到(-1,-1), (-1,1), (1,-1), (1,1)四个顶点, 无需任何变换.
	
	vec2 coords = vertex.zw;
	if (chaos)			// 利用边缘检测来创建有趣的视觉效果, 并以圆形方式移动纹理图像以获得有趣的混沌效果.
	{	// 因为我们将纹理wapper方法设置为 GL_REPEAT, 混沌效果会导致场景在四边形的各个部分重复.
		float strength = 0.3f;
		texCoords = vec2(coords.x + sin(time) * strength, coords.y + cos(time) * strength);

		//texCoords = coords;
	}
	else if (confuse)	// 反转场景的颜色, 但也反转 x 和 y 轴
	{
		texCoords = vec2(1.0f - coords.x, 1.0f - coords.y);
	}
	else				// 无后处理效果, 则直接是coords.
	{
		texCoords = coords;
	}

	// 这里我们让shake和chaos或者confuse可以同时发生, 但是chaos和confuse不可以同时发生.
	if (shake)			// 轻微对游戏场景的模糊抖动
	{	// 如果shake 设置为true, 它会稍微移动顶点位置, 这就好像是屏幕在抖动一样.
		float strength = 0.01;
		gl_Position.x += cos(time * 10) * strength;
		gl_Position.y += cos(time * 15) * strength;
	}
}


