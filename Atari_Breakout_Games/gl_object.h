#ifndef GL_OBJECT_H
#define GL_OBJECT_H

#include <glad/glad.h>
#include <iostream>

class VAO		// checked, no need to check again!
{
public:
	VAO() { glGenVertexArrays(1, &(this->id)); }
	~VAO() { glDeleteVertexArrays(1, &(this->id)); }
public:
	unsigned int ID() const { return this->id; }
	void Bind() { glBindVertexArray(this->id); }
	void UnBind() { glBindVertexArray(0); }
private:
	unsigned int id;
};

class VBO		// checked, no need to check again!
{
public:
	VBO() { glGenBuffers(1, &(this->id)); }
	~VBO() { glDeleteBuffers(1, &(this->id)); }
public:
	unsigned int ID() const { return this->id; }
	bool IsBound() const { return boundIdx != 0 && boundIdx == this->id; }		// 对于unsigned int类型, 不能够使用!boundIdx得不到取反.
	
	void Bind() { glBindBuffer(GL_ARRAY_BUFFER, this->id); boundIdx = this->id; }
	void UnBind() { glBindBuffer(1, 0); boundIdx = 0; }
	
	void LoadVertexData(unsigned int data_size, const void* data, GLenum usage = GL_STATIC_DRAW)
	{
		if (!IsBound())
		{
			fprintf(stderr, "%s @ %d: fail to call LoadVertexData() as the VBO is not bound to GL_ARRAY_BUFFER.\n", __FILE__, __LINE__);
			return;
		}

		glBufferData(GL_ARRAY_BUFFER, data_size, data, usage);
	}
	void ConfigureVertexAttribute(unsigned int location, unsigned int attrib_size, GLenum data_type, unsigned int stride, unsigned int offset)
	{
		if (!IsBound())
		{
			fprintf(stderr, "%s @ %d: fail to call ConfigureVertexAttribute() as the VBO is not bound to GL_ARRAY_BUFFER.\n", __FILE__, __LINE__);
			return;
		}

		glVertexAttribPointer(location, attrib_size, data_type, false, stride, (void*)offset);
		glEnableVertexAttribArray(location);
	}
private:
	unsigned int id;

	static unsigned int boundIdx;
};
unsigned int VBO::boundIdx = 0;

// A renderbuffer is a data storage object containing a single image of a renderable internal format.
class RBO
{
public:
	RBO() : width{ 0 }, height{ 0 }, internalImageFormat{ 0 } { glGenRenderbuffers(1, &(this->id)); }
	RBO(unsigned int w, unsigned int h, GLenum internal_format, unsigned int sample = 0) : RBO()
	{
		GenerateRenderBuffer(w, h, internal_format, sample);
	}
	~RBO() { glDeleteRenderbuffers(1, &this->id); }
public:
	unsigned int ID() const { return this->id; }
	unsigned int Width() const { return this->width; }
	unsigned int Height() const { return this->height; }

	// Renderbuffer object names are unsigned integers. The value zero is reserved, but there is no default renderbuffer object. 
	// Instead, renderbuffer set to zero effectively unbinds any renderbuffer object previously bound. 
	void Bind() { glBindRenderbuffer(GL_RENDERBUFFER, this->id); }
	void UnBind() { glBindRenderbuffer(GL_RENDERBUFFER, 0); }

	void GenerateRenderBuffer(unsigned int w, unsigned int h, GLenum internal_format, unsigned int sample = 0)
	{
		this->width = w;
		this->height = h;
		this->internalImageFormat = internal_format;

		Bind();
		if (sample == 0)
		{
			glRenderbufferStorage(GL_RENDERBUFFER, this->internalImageFormat, this->width, this->height);
		}
		else
		{
			glRenderbufferStorageMultisample(GL_RENDERBUFFER, sample, this->internalImageFormat, this->width, this->height);
		}
	}
private:
	unsigned int id;
	unsigned int width, height;
	unsigned int internalImageFormat;
};

#endif // !GL_OBJECT_H
