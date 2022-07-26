#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include <glad/glad.h>
#include <iostream>
#include "gl_object.h"
#include "texture.h"

class Framebuffer
{
public:
	Framebuffer();
	~Framebuffer();
public:
	unsigned int ID() const;

	void Bind();
	void UnBind();

	bool IsBound() const;
	bool IsMultisample() const;
	bool IsComplete() const;		// IsComplete()会检查FBO是否完备, 强烈建议在创建完FBO后都调用这一API进行完备性检查, 确保使用前FBO是完备的.

	// 暂时设计成只有一个color_attachment的形式. multisample texture object和multisample renderingbuffer object的attach FBO的API和没有多重采样是一样的, 没有区别.
	void AttachColorBuffer(Texture2D& texture);		// 如果color attachment采用了multisample, 那么相应的, depth attachment和stencil attachment也要采用multisample.
	void AttachDepthBuffer(Texture2D& texture);
	void AttachStencilBuffer(Texture2D& texture);
	void AttachDepthStencilBuffer(Texture2D& texture);

	void AttachColorBuffer(RBO& rbo);
	void AttachDepthBuffer(RBO& rbo);
	void AttachStencilBuffer(RBO& rbo);
	void AttachDepthStencilBuffer(RBO& rbo);

	void BlitColorToFrameBuffer(Framebuffer& dst);		// 只做整个framebuffer的blit. 也即认定两个FBO的size完全相同.
	void BlitColorToDefault();

	static void BindToDefault();

private:
	unsigned int id;
	unsigned int width, height;
	bool isMultisample;

	static unsigned int boundIdx;

	bool CheckBoundError(const char* func) const;
};

unsigned int Framebuffer::boundIdx = 0;

Framebuffer::Framebuffer()
{ 
	this->width = 0;
	this->height = 0;
	this->isMultisample = false;
	glGenFramebuffers(1, &(this->id)); 
}
Framebuffer::~Framebuffer()
{
	glDeleteFramebuffers(1, &(this->id));
}

unsigned int Framebuffer::ID() const
{
	return this->id;
}

void Framebuffer::Bind()
{
	glBindFramebuffer(GL_FRAMEBUFFER, this->id);
	boundIdx = this->id;
}
void Framebuffer::UnBind()
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	boundIdx = 0;
}

bool Framebuffer::IsBound() const		
{
	// 对于unsigned int类型, 不能够使用!boundIdx得不到取反.
	return boundIdx != 0 && boundIdx == this->id;
}		
bool Framebuffer::IsMultisample() const
{
	return isMultisample;
}
bool Framebuffer::IsComplete() const
{
	if (CheckBoundError("IsComplete")) return false;
	
	return glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
}

void Framebuffer::AttachColorBuffer(Texture2D& texture)		// 暂时设计成只有一个color_attachment的形式.
{
	if (CheckBoundError("AttachColorBuffer")) return;

	if (!texture.IsMultisample())
	{
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture.ID(), 0);
		this->isMultisample = false;
	}
	else
	{
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, texture.ID(), 0);
		this->isMultisample = true;
	}

	this->width = texture.Width();
	this->height = texture.Height();
}
void Framebuffer::AttachDepthBuffer(Texture2D& texture)
{
	if (CheckBoundError("AttachDepthBuffer")) return;
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, texture.ID(), 0);
}
void Framebuffer::AttachStencilBuffer(Texture2D& texture)
{
	if (CheckBoundError("AttachStencilBuffer")) return;
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_TEXTURE_2D, texture.ID(), 0);
}
void Framebuffer::AttachDepthStencilBuffer(Texture2D& texture)
{
	if (CheckBoundError("AttachDepthStencilBuffer")) return;
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, texture.ID(), 0);
}

void Framebuffer::AttachColorBuffer(RBO& rbo)
{
	if (CheckBoundError("AttachColorBuffer")) return;

	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, rbo.ID());

	this->width = rbo.Width();
	this->height = rbo.Height();
}
void Framebuffer::AttachDepthBuffer(RBO& rbo)
{
	if (CheckBoundError("AttachDepthBuffer")) return;
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo.ID());
}
void Framebuffer::AttachStencilBuffer(RBO& rbo)
{
	if (CheckBoundError("AttachStencilBuffer")) return;
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo.ID());
}
void Framebuffer::AttachDepthStencilBuffer(RBO& rbo)
{
	if (CheckBoundError("AttachDepthStencilBuffer")) return;
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo.ID());
}

void Framebuffer::BlitColorToFrameBuffer(Framebuffer& dst)
{
	if (this->width != dst.width || this->height != dst.height)
	{
		fprintf(stderr, "%s @ %d: fail to call Blit() as the two FBOs are not in same size.\n", __FILE__, __LINE__);
		return;
	}
	
	glBindFramebuffer(GL_READ_FRAMEBUFFER, this->id);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, dst.id);
	glBlitFramebuffer(0, 0, this->width, this->height, 0, 0, this->width, this->height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
	this->Bind();
}
void Framebuffer::BlitColorToDefault()
{
	glBindFramebuffer(GL_READ_FRAMEBUFFER, this->id);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	glBlitFramebuffer(0, 0, this->width, this->height, 0, 0, this->width, this->height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
	this->Bind();
}

bool Framebuffer::CheckBoundError(const char* func) const
{
	if (!IsBound())
	{
		fprintf(stderr, "%s @ %d: fail to call %s() as the FBO is not bound to GL_FRAMEBUFFER.\n", __FILE__, __LINE__, func);
		return true;
	}

	return false;
}

void Framebuffer::BindToDefault()
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	boundIdx = 0;
}
#endif // !FRAMEBUFFER_H

