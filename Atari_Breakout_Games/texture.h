#ifndef TEXTURE_H
#define TEXTURE_H

#include <glad/glad.h>
#include <vector>

class Texture2D		// checked, no need to check again!
{
public:
	Texture2D();	// 由于texture unit多达至少80多个, 所以我们认定每个texture object会绑定到不同的texture unit上, 并且自始至终都绑定其texture target. 所以直接在构造函数中即独占一个texture unit.
	Texture2D(unsigned int w, unsigned int h, unsigned int internal_format, unsigned int source_format, unsigned char* data, unsigned int sample = 0, bool mipmap = true);
	~Texture2D();
public:
	unsigned int ID() const;
	unsigned int UnitID() const;

	unsigned int Width() const;
	unsigned int Height() const;

	bool IsMultisample() const;

	// texture object不提供bind和unbind函数. 因为每个texture object将会独占一个texture unit和texture unit对应的texture target.
	// texture unit多达80多个, 基本不可能用完. 所以在构造texture object时就为其分配独占的texture unit. 
	// 并且texture object在载入了纹理图像后, 纹理图像的变更频率也极低(我们可以创建新的texture object), 因此texture object独占一个texture target.

	// LoadImage使用必定不会特别频繁, 所以对于texture object, 自始至终都让其在调用LoadImage时进行必要的texture unite enable和texture target绑定操作.
	void LoadImage(unsigned int w, unsigned int h, unsigned int internal_format, unsigned int source_format, unsigned char* data, bool mipmap = true);
	void GenerateRenderBuffer(unsigned int w, unsigned int h, unsigned int internal_format, unsigned int sample = 0);
	void GenerateMipmap();
private:
	unsigned int id;
	unsigned int unitID;

	unsigned int width, height;
	unsigned int internalImageFormat, sourceImageFormat;		// format of texture object image and original loaded pixel data.
	
	unsigned int numSample;

	static unsigned int unUsedUnitIdx;		// The number of texture units is implementation dependent, but must be at least 80. 所以根本用不完.
};

unsigned int Texture2D::unUsedUnitIdx = 0;

Texture2D::Texture2D()
{
	this->width = 0;
	this->height = 0;
	this->internalImageFormat = 0;
	this->sourceImageFormat = 0;
	this->numSample = 0;

	glGenTextures(1, &(this->id));
	this->unitID = unUsedUnitIdx;	// 在构造texture object时, 就分配好独占的texture unit, 并不激活, 在Bind或者Unbind的时候激活.
	unUsedUnitIdx = ++unUsedUnitIdx % 80;
}
Texture2D::Texture2D(unsigned int w, unsigned int h, unsigned int internal_format, unsigned int source_format, unsigned char* data, unsigned int sample, bool mipmap) : Texture2D()
{
	if (data)
	{
		LoadImage(w, h, internal_format, source_format, data, mipmap);
	}
	else
	{
		GenerateRenderBuffer(w, h, internal_format, sample);
	}
}
Texture2D::~Texture2D()
{
	glDeleteTextures(1, &(this->id));
}

unsigned int Texture2D::ID() const
{
	return this->id;
}
unsigned int Texture2D::UnitID() const
{
	return this->unitID;
}

unsigned int Texture2D::Width() const
{
	return this->width;
}
unsigned int Texture2D::Height() const
{
	return this->height;
}

bool Texture2D::IsMultisample() const
{
	return this->numSample > 0;
}

// 默认data是通过sti_image读取的, data是一个1D array, element数据类型是unsigned byte类型.
void Texture2D::LoadImage(unsigned int w, unsigned int h, unsigned int internal_format, unsigned int source_format, unsigned char* data, bool mipmap)
{
	if (!data)
	{
		fprintf(stderr, "%s @ %d: fail to call LoadImage() as data pointer is nullptr.\n", __FILE__, __LINE__);
		return;
	}

	if (this->IsMultisample())
	{
		fprintf(stderr, "%s @ %d: fail to call LoadImage() as texture has already defined as multisample buffer texture.\n", __FILE__, __LINE__);
		return;
	}

	glActiveTexture(GL_TEXTURE0 + this->unitID);
	glBindTexture(GL_TEXTURE_2D, this->id);

	this->width = w;
	this->height = h;
	this->internalImageFormat = internal_format;
	this->sourceImageFormat = source_format;

	// 默认wrap模式是repeat模式.
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	if (mipmap)
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
	}
	else
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	}
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// generate base 2D texture image.
	glTexImage2D(GL_TEXTURE_2D, 0, this->internalImageFormat, this->width, this->height, 0, this->sourceImageFormat, GL_UNSIGNED_BYTE, data);

	if (mipmap)
	{
		glGenerateMipmap(GL_TEXTURE_2D);
	}
}
void Texture2D::GenerateRenderBuffer(unsigned int w, unsigned int h, unsigned int internal_format, unsigned int sample)
{
	glActiveTexture(GL_TEXTURE0 + this->unitID);

	this->width = w;
	this->height = h;
	this->internalImageFormat = internal_format;
	this->sourceImageFormat = internal_format;
	this->numSample = sample;

	// generate texture image buffer for rendering purpose.
	if (!this->IsMultisample())
	{
		glBindTexture(GL_TEXTURE_2D, this->id);

		// texture object必须显式对纹理和采样参数进行配置后, 才能够成功建立起采样映射关系. 不配置texture object的话, 采样结果什么都没.
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glTexImage2D(GL_TEXTURE_2D, 0, this->internalImageFormat, this->width, this->height, 0, this->sourceImageFormat, GL_UNSIGNED_BYTE, nullptr);
	}
	else
	{
		glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, this->id);
		glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, this->numSample, this->internalImageFormat, this->width, this->height, true);		// 一般都选择固定采样点在像素区域的位置.
	}
}

void Texture2D::GenerateMipmap()
{
	if (IsMultisample())
	{
		fprintf(stderr, "%s @ %d: fail to call GenerateMipmap() as the tetxure is defined as multisample rendering attachment.\n", __FILE__, __LINE__);
		return;
	}

	glActiveTexture(GL_TEXTURE0 + this->unitID);
	glBindTexture(GL_TEXTURE_2D, this->id);

	glGenerateMipmap(GL_TEXTURE_2D);
}

#endif // !TEXTURE_H

