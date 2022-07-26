#version 460 core
in vec2 texCoords;

uniform sampler2D scene;
uniform vec2 offsets[9];
uniform float edge_kernel[9];			// 边界视觉.
uniform float blur_kernel[9];		// Gaussian模糊.

// uniform变量是全局的, 但是必须要在想要使用的shader中进行声明.
uniform bool chaos;
uniform bool confuse;
uniform bool shake;

out vec4 fragColor;


void main()
{
	fragColor = vec4(0.0f);
	vec3 samples[9];			// sample from texture offsets if using convolution matrix
	if (chaos || shake)
	{
		for(unsigned int idx = 0; idx < 9; ++idx)
		{
			samples[idx] = vec3(texture(scene, texCoords.st + offsets[idx]));
		}
	}

	// process effects.
	if (chaos)
	{
		for(unsigned int idx = 0; idx < 9; ++idx)
		{
			fragColor += vec4(samples[idx] * edge_kernel[idx], 0.0f);
			fragColor.a = 1.0f;
		}
	}
	else if (confuse)
	{
		fragColor = vec4(1.0f - texture(scene, texCoords).rgb, 1.0f);
	}
	else if (shake)
	{
		for(unsigned int idx = 0; idx < 9; ++idx)
		{
			fragColor += vec4(samples[idx] * blur_kernel[idx], 0.0f);
			fragColor.a = 1.0f;
		}
	}
	else
	{
		fragColor = texture(scene, texCoords);
	}
}
