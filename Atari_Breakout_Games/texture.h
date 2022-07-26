#ifndef TEXTURE_H
#define TEXTURE_H

#include <glad/glad.h>
#include <vector>

class Texture2D		// checked, no need to check again!
{
public:
	Texture2D();	// ����texture unit�������80���, ���������϶�ÿ��texture object��󶨵���ͬ��texture unit��, ������ʼ���ն�����texture target. ����ֱ���ڹ��캯���м���ռһ��texture unit.
	Texture2D(unsigned int w, unsigned int h, unsigned int internal_format, unsigned int source_format, unsigned char* data, unsigned int sample = 0, bool mipmap = true);
	~Texture2D();
public:
	unsigned int ID() const;
	unsigned int UnitID() const;

	unsigned int Width() const;
	unsigned int Height() const;

	bool IsMultisample() const;

	// texture object���ṩbind��unbind����. ��Ϊÿ��texture object�����ռһ��texture unit��texture unit��Ӧ��texture target.
	// texture unit���80���, ��������������. �����ڹ���texture objectʱ��Ϊ������ռ��texture unit. 
	// ����texture object������������ͼ���, ����ͼ��ı��Ƶ��Ҳ����(���ǿ��Դ����µ�texture object), ���texture object��ռһ��texture target.

	// LoadImageʹ�ñض������ر�Ƶ��, ���Զ���texture object, ��ʼ���ն������ڵ���LoadImageʱ���б�Ҫ��texture unite enable��texture target�󶨲���.
	void LoadImage(unsigned int w, unsigned int h, unsigned int internal_format, unsigned int source_format, unsigned char* data, bool mipmap = true);
	void GenerateRenderBuffer(unsigned int w, unsigned int h, unsigned int internal_format, unsigned int sample = 0);
	void GenerateMipmap();
private:
	unsigned int id;
	unsigned int unitID;

	unsigned int width, height;
	unsigned int internalImageFormat, sourceImageFormat;		// format of texture object image and original loaded pixel data.
	
	unsigned int numSample;

	static unsigned int unUsedUnitIdx;		// The number of texture units is implementation dependent, but must be at least 80. ���Ը����ò���.
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
	this->unitID = unUsedUnitIdx;	// �ڹ���texture objectʱ, �ͷ���ö�ռ��texture unit, ��������, ��Bind����Unbind��ʱ�򼤻�.
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

// Ĭ��data��ͨ��sti_image��ȡ��, data��һ��1D array, element����������unsigned byte����.
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

	// Ĭ��wrapģʽ��repeatģʽ.
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

		// texture object������ʽ������Ͳ��������������ú�, ���ܹ��ɹ����������ӳ���ϵ. ������texture object�Ļ�, �������ʲô��û.
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glTexImage2D(GL_TEXTURE_2D, 0, this->internalImageFormat, this->width, this->height, 0, this->sourceImageFormat, GL_UNSIGNED_BYTE, nullptr);
	}
	else
	{
		glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, this->id);
		glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, this->numSample, this->internalImageFormat, this->width, this->height, true);		// һ�㶼ѡ��̶������������������λ��.
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

