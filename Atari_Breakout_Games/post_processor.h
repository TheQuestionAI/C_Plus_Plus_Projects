#ifndef POST_PROCESSOR_H
#define POST_PROCESSOR_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <memory>

#include "shader.h"
#include "texture.h"
#include "gl_object.h"
#include "framebuffer.h"

/*
* PostProcessor后处理器: hosts all PostProcessing effects for the Breakout Game. 
* 后处理器做的事情就是对于已经得到的渲染结果进行再加工, 其本质就是对存储在framebuffer中的resulted pixel data (基本上都是对color buffer)再施加处理工作.
* 后处理操作只能在user自定义的framebuffer object上进行(因为default framebuffer的渲染结果直接输出到渲染窗口), 并且该FBO必须要绑定texture形式的attachment(renderbuffer用户无法直接读写).
* 
* the rendering steps are启用后处理器的游戏渲染步骤通常为:
*		1. Bind to multisampled framebuffer. 
		   后处理器内部维持一个多重采样FBO, 并且多重采样FBO通常启用renderbuffer object作为color attachment(因为我们基本不会对该FBO进行读写操作). 采用多重采样FBO是我们想要得到初步的高质量的渲染结果.
*		2. Render game as normal. 
		   对游戏进行正常渲染操作, 以多重采样FBO作为渲染容器, 渲染结果将存储于多重采样FBO中.
*		3. Blit multisampled framebuffer to normal framebuffer with texture attachment. 
		   后处理器内部维持一个普通FBO, 普通FBO通常以texture object作为color attachment(对渲染结果的后处理工作就是作用于这一个包含渲染结果的texture object上). 
		   将多重采样FBO得到的渲染结果Blit(这一操作为自适应的对每个像素寻求最佳采样点)到普通FBO中.
*		4. Unbind framebuffer (use default framebuffer).
*          解绑普通FBO, 将default framebuffer绑定, 准备对渲染结果的后处理操作.
*		5. Use color buffer texture from normal framebuffer in postprocessing shader.
*          启用我们想要的后处理操作, 在着色器中对texture采样的color进行后期处理.
*		6. Render quad of screen-size as output of postprocessing shader.
*		   后期处理过的渲染结果将会写入到default framebuffer然后渲染到窗口中.
* 
* 后处理器的核心四个成员函数:
*		1. Init();					用于初始化一切后处理器需要的渲染数据.
*		2. BeginSceneRender();		启用后处理器对game进行初始渲染: 即绑定后处理器维持的多重采样FBO, 让game类中的初始游戏渲染结果落入多重采样FBO中.
*		3. EndSceneRender();		结束后处理器对game进行的初始渲染: 即多重采样FBO已经盛放好初始游戏渲染结果后, 解除绑定, 将渲染结果Blit到附着了texture object的普通FBO上.
*		4. RenderToWindow(dt);		启用真正的后处理器操作, 执行完毕后渲染到游戏窗口中.
* 
* 本Game主要由三种预处理效果:
*		1. shake模糊抖动: slightly shakes the scene with a small blur.
*		2. confuse颜色和坐标反转: inverses the colors of the scene, but also the x and y axis.
*		3. chaos边界视觉和圆形贴图渲染: makes use of an edge detection kernel to create interesting visuals and also moves the textured image in a circular fashion for an interesting chaotic effect.
* 
*/
class PostProcessor
{
public:
	PostProcessor();
	PostProcessor(std::shared_ptr<Shader> sd, unsigned int w, unsigned int h);
public:
	// post-processing main loop
	void BeginSceneRender();
	void EndSceneRender();
	void RenderToWindow(float time);
public:
	void Shake(bool state);
	void Reverse(bool state);
	void Chaos(bool state);

	bool IsShaked() const;
	bool IsReversed() const;
	bool IsChaotic() const;
private:
	std::shared_ptr<Shader> shader;
	unsigned int width, height;

	bool isShaked, isConfused, isChaotic;

	Framebuffer multiFbo, fbo;
	Texture2D texture;
	RBO rbo;
	VBO vbo;
	VAO vao;

	void Init();
};

PostProcessor::PostProcessor() 
	: shader{ nullptr }, width{ 0 }, height{ 0 }, isShaked{ false }, isConfused{ false }, isChaotic{ false }, multiFbo{ }, fbo{ }, texture{ }, rbo{ }, vbo{ }, vao{ } { }
PostProcessor::PostProcessor(std::shared_ptr<Shader> sd, unsigned int w, unsigned int h) : PostProcessor()
{
	this->shader = sd;
	this->width = w;
	this->height = h;

	Init();
}
void PostProcessor::Init()
{
	// ~~~~~~~~~~~~~~~~~~~~~~~ initialize render vertex data. 把实例顶点和纹理坐标数据存储在GPU buffer object中. ~~~~~~~~~~~~~~~~
	// 我们想要进行纹理图像映射, 那我们首先必须要定义顶点数据, 对于一个2D纹理图像, 我们只需要定义长方形的四个顶点以及相应的纹理坐标, 然后进行线性插值就可以获得纹理图像下所有坐标的纹理.
	float square[] = {				// 后处理器的square直接将顶点定义到(-1,-1), (-1,1), (1,-1), (1,1)四个顶点, 如此在vertex shader中就无需要任何矩阵变换, 直接会cover NDC空间中的整个游戏平面.
		// positions // texCoords
	   -1.0f,  1.0f, 0.0f, 1.0f,
	   -1.0f, -1.0f, 0.0f, 0.0f,
		1.0f, -1.0f, 1.0f, 0.0f,
	   -1.0f,  1.0f, 0.0f, 1.0f,
		1.0f, -1.0f, 1.0f, 0.0f,
		1.0f,  1.0f, 1.0f, 1.0f
	};

	this->vao.Bind();
	this->vbo.Bind();
	this->vbo.LoadVertexData(sizeof(square), square);
	this->vbo.ConfigureVertexAttribute(0, 4, GL_FLOAT, 4 * sizeof(float), 0);
	this->vbo.UnBind();
	this->vao.UnBind();
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	
	// ~~~~~~~~~~~~~~~~~~~~~~~ initialize renderbuffer storage with a multisampled color buffer (don't need a depth/stencil buffer) ~~~~~~~~~~~~~~~~
	this->multiFbo.Bind();
	this->rbo.GenerateRenderBuffer(this->width, this->height, GL_RGB, 4);
	this->multiFbo.AttachColorBuffer(rbo);
	if (!this->multiFbo.IsComplete())
	{
		fprintf(stderr, "%s @ %d: POSTPROCESSOR: Failed to initialize mutisample framebuffer.\n", __FILE__, __LINE__);
	}
	this->multiFbo.UnBind();
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


	// ~~~~~~~~~~~~~~~~~~~~~~~ initialize the FBO/texture to blit multisampled color-buffer to; used for shader operations (for postprocessing effects) ~~~~~~~~~~~~~~~~
	this->fbo.Bind();
	this->texture.GenerateRenderBuffer(this->width, this->height, GL_RGB);
	this->fbo.AttachColorBuffer(texture);
	if (!this->fbo.IsComplete())
	{
		fprintf(stderr, "%s @ %d: POSTPROCESSOR: Failed to initialize normal framebuffer.\n", __FILE__, __LINE__);
	}
	this->fbo.UnBind();
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

	// ~~~~~~~~~~~~~~~~~~~~~~~ initialize shader uniforms ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	this->shader->Use();
	this->shader->SetInteger("scene", texture.UnitID());		// associate the texture uniform variable to texture unit.
	
	const float OFFSET = 1.0f / 300.0f;
	float OFFSETS[9][2] = {
		{ -OFFSET,  OFFSET  },  // top-left
		{  0.0f,    OFFSET  },  // top-center
		{  OFFSET,  OFFSET  },  // top-right
		{ -OFFSET,  0.0f    },  // center-left
		{  0.0f,    0.0f    },  // center-center
		{  OFFSET,  0.0f    },  // center - right
		{ -OFFSET, -OFFSET  },  // bottom-left
		{  0.0f,   -OFFSET  },  // bottom-center
		{  OFFSET, -OFFSET  }   // bottom-right    
	};
	this->shader->SetVector2fv("offsets", 9, OFFSETS);

	float EDGE_KERNEL[9] = {
		-1.0f, -1.0f, -1.0f,
		-1.0f,  8.0f, -1.0f,
		-1.0f, -1.0f, -1.0f
	};
	this->shader->SetFloatv("edge_kernel", 9, EDGE_KERNEL);
	
	float BLUR_KERNEL[9] = {
		1.0f / 16.0f, 2.0f / 16.0f, 1.0f / 16.0f,
		2.0f / 16.0f, 4.0f / 16.0f, 2.0f / 16.0f,
		1.0f / 16.0f, 2.0f / 16.0f, 1.0f / 16.0f
	};
	this->shader->SetFloatv("blur_kernel", 9, BLUR_KERNEL);
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

}

void PostProcessor::BeginSceneRender()
{
	this->multiFbo.Bind();
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
}
void PostProcessor::EndSceneRender()
{
	this->multiFbo.BlitColorToFrameBuffer(this->fbo);
	Framebuffer::BindToDefault();
}
void PostProcessor::RenderToWindow(float time)
{
	this->shader->Use();

	this->shader->SetFloat("time", time);
	this->shader->SetBool("shake", this->isShaked);
	this->shader->SetBool("confuse", this->isConfused);
	this->shader->SetBool("chaos", this->isChaotic);

	this->vao.Bind();
	glDrawArrays(GL_TRIANGLES, 0, 6);
	this->vao.UnBind();
}

void PostProcessor::Shake(bool state)
{
	this->isShaked = state;
}
void PostProcessor::Reverse(bool state)
{
	this->isConfused = state;
}
void PostProcessor::Chaos(bool state)
{
	this->isChaotic = state;
}

bool PostProcessor::IsShaked() const
{
	return this->isShaked;
}
bool PostProcessor::IsReversed() const
{
	return this->isConfused;
}
bool PostProcessor::IsChaotic() const
{
	return this->isChaotic;
}

#endif // !POST_PROCESSOR_H

