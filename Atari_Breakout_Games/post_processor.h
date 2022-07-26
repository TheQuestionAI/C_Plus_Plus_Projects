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
* PostProcessor������: hosts all PostProcessing effects for the Breakout Game. 
* ����������������Ƕ����Ѿ��õ�����Ⱦ��������ټӹ�, �䱾�ʾ��ǶԴ洢��framebuffer�е�resulted pixel data (�����϶��Ƕ�color buffer)��ʩ�Ӵ�����.
* �������ֻ����user�Զ����framebuffer object�Ͻ���(��Ϊdefault framebuffer����Ⱦ���ֱ���������Ⱦ����), ���Ҹ�FBO����Ҫ��texture��ʽ��attachment(renderbuffer�û��޷�ֱ�Ӷ�д).
* 
* the rendering steps are���ú���������Ϸ��Ⱦ����ͨ��Ϊ:
*		1. Bind to multisampled framebuffer. 
		   �������ڲ�ά��һ�����ز���FBO, ���Ҷ��ز���FBOͨ������renderbuffer object��Ϊcolor attachment(��Ϊ���ǻ�������Ը�FBO���ж�д����). ���ö��ز���FBO��������Ҫ�õ������ĸ���������Ⱦ���.
*		2. Render game as normal. 
		   ����Ϸ����������Ⱦ����, �Զ��ز���FBO��Ϊ��Ⱦ����, ��Ⱦ������洢�ڶ��ز���FBO��.
*		3. Blit multisampled framebuffer to normal framebuffer with texture attachment. 
		   �������ڲ�ά��һ����ͨFBO, ��ͨFBOͨ����texture object��Ϊcolor attachment(����Ⱦ����ĺ�����������������һ��������Ⱦ�����texture object��). 
		   �����ز���FBO�õ�����Ⱦ���Blit(��һ����Ϊ����Ӧ�Ķ�ÿ������Ѱ����Ѳ�����)����ͨFBO��.
*		4. Unbind framebuffer (use default framebuffer).
*          �����ͨFBO, ��default framebuffer��, ׼������Ⱦ����ĺ������.
*		5. Use color buffer texture from normal framebuffer in postprocessing shader.
*          ����������Ҫ�ĺ������, ����ɫ���ж�texture������color���к��ڴ���.
*		6. Render quad of screen-size as output of postprocessing shader.
*		   ���ڴ��������Ⱦ�������д�뵽default framebufferȻ����Ⱦ��������.
* 
* �������ĺ����ĸ���Ա����:
*		1. Init();					���ڳ�ʼ��һ�к�������Ҫ����Ⱦ����.
*		2. BeginSceneRender();		���ú�������game���г�ʼ��Ⱦ: ���󶨺�����ά�ֵĶ��ز���FBO, ��game���еĳ�ʼ��Ϸ��Ⱦ���������ز���FBO��.
*		3. EndSceneRender();		������������game���еĳ�ʼ��Ⱦ: �����ز���FBO�Ѿ�ʢ�źó�ʼ��Ϸ��Ⱦ�����, �����, ����Ⱦ���Blit��������texture object����ͨFBO��.
*		4. RenderToWindow(dt);		���������ĺ���������, ִ����Ϻ���Ⱦ����Ϸ������.
* 
* ��Game��Ҫ������Ԥ����Ч��:
*		1. shakeģ������: slightly shakes the scene with a small blur.
*		2. confuse��ɫ�����귴ת: inverses the colors of the scene, but also the x and y axis.
*		3. chaos�߽��Ӿ���Բ����ͼ��Ⱦ: makes use of an edge detection kernel to create interesting visuals and also moves the textured image in a circular fashion for an interesting chaotic effect.
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
	// ~~~~~~~~~~~~~~~~~~~~~~~ initialize render vertex data. ��ʵ������������������ݴ洢��GPU buffer object��. ~~~~~~~~~~~~~~~~
	// ������Ҫ��������ͼ��ӳ��, ���������ȱ���Ҫ���嶥������, ����һ��2D����ͼ��, ����ֻ��Ҫ���峤���ε��ĸ������Լ���Ӧ����������, Ȼ��������Բ�ֵ�Ϳ��Ի������ͼ�����������������.
	float square[] = {				// ��������squareֱ�ӽ����㶨�嵽(-1,-1), (-1,1), (1,-1), (1,1)�ĸ�����, �����vertex shader�о�����Ҫ�κξ���任, ֱ�ӻ�cover NDC�ռ��е�������Ϸƽ��.
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

