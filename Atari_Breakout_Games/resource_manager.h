#ifndef RESOURCE_MANAGER_H
#define RESOURCE_MANAGER_H

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <memory>
#include <unordered_map>
#include "shader.h"
#include "texture.h"
#include "vs_stb_image.h"

/*
* 设计模式六大原则之一, 面向对象编程的单一职责原则: 在OOP领域中, 单一功能原则(Single responsibility principle)规定每个类都应该有一个单一的功能, 并且该功能应该由这个类完全封装起来.
* 
* 单例模式静态资源管理类:
* 一种更有组织的方法来创建一个单一的实体, 用于加载游戏相关资源, 称为资源管理器. 
* 我们选择使用单例静态资源管理器(由于其静态特性)在整个项目中始终可用, 托管所有加载的资源及其相关的加载功能, 这个2D Game资源管理器将用于管理着色器程序资源和纹理资源.
* Define a singleton static class to load all shaders& textures resource.
*/
class ResourceManager		// checked, no need to check again!
{
public:
	// loads (and generates) a shader program from file loading vertex, fragment (and geometry) shader's source code. If gShaderFile is not nullptr, it also loads a geometry shader
	static std::shared_ptr<Shader> LoadShader(const std::string& name, const char* vShaderFile, const char* fShaderFile, const char* gShaderFile = nullptr);
	static std::shared_ptr<Shader> GetShader(const std::string& name);

	// loads (and generates) a texture from file
	static std::shared_ptr<Texture2D> LoadTexture(const std::string& name, const char* file, bool mipmap = true);
	static std::shared_ptr<Texture2D> GetTexture(const std::string& name);
private:
	static std::unordered_map<std::string, std::shared_ptr<Shader>> shaders;
	static std::unordered_map<std::string, std::shared_ptr<Texture2D>> textures;

	static std::shared_ptr<Shader> LoadShaderFromFile(const char* vShaderFile, const char* fShaderFile, const char* gShaderFile);
	static std::shared_ptr<Texture2D> LoadTextureFromFile(const char* file, bool mipmap);
};

std::unordered_map<std::string, std::shared_ptr<Shader>> ResourceManager::shaders;
std::unordered_map<std::string, std::shared_ptr<Texture2D>> ResourceManager::textures;

std::shared_ptr<Shader> ResourceManager::LoadShader(const std::string& name, const char* vShaderFile, const char* fShaderFile, const char* gShaderFile)
{
	shaders.insert({ name, LoadShaderFromFile(vShaderFile, fShaderFile, gShaderFile) });
	return shaders[name];
}
std::shared_ptr<Shader> ResourceManager::GetShader(const std::string& name)
{
	return shaders.at(name);
}
std::shared_ptr<Shader> ResourceManager::LoadShaderFromFile(const char* vShaderFile, const char* fShaderFile, const char* gShaderFile)
{
	// retrieve the vertex/fragment source code from filePath
	std::ifstream vShaderSourceFile, fShaderSourceFile, gShaderSourceFile;
	std::string vShaderSourceCode, fShaderSourceCode, gShaderSourceCode;

	vShaderSourceFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	fShaderSourceFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	gShaderSourceFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	try
	{
		vShaderSourceFile.open(vShaderFile);
		fShaderSourceFile.open(fShaderFile);

		std::stringstream vShaderSource, fShaderSource;
		vShaderSource << vShaderSourceFile.rdbuf();
		fShaderSource << fShaderSourceFile.rdbuf();

		vShaderSourceFile.close();
		fShaderSourceFile.close();

		vShaderSourceCode = vShaderSource.str();
		fShaderSourceCode = fShaderSource.str();

		if (gShaderFile)
		{
			gShaderSourceFile.open(gShaderFile);
			std::stringstream gShaderSource;
			gShaderSource << gShaderSourceFile.rdbuf();
			gShaderSourceFile.close();
			gShaderSourceCode = gShaderSource.str();
		}
	}
	catch (std::ifstream::failure e)
	{
		fprintf(stderr, "%s @ %d: ERROR::SHADER::FILE_NOT_SURCESSFULLY_READ.\n", __FILE__, __LINE__);
	}

	const char* vShaderSource = vShaderSourceCode.c_str();
	const char* fShaderSource = fShaderSourceCode.c_str();
	const char* gShaderSource = gShaderSourceCode.c_str();

	return std::make_shared<Shader>(vShaderSource, fShaderSource, gShaderFile ? gShaderSource : nullptr);
}

std::shared_ptr<Texture2D> ResourceManager::LoadTexture(const std::string& name, const char* file, bool mipmap)
{
	textures.insert({ name, LoadTextureFromFile(file, mipmap) });
	return textures[name];
}
std::shared_ptr<Texture2D> ResourceManager::GetTexture(const std::string& name)
{
	return textures.at(name);
}
std::shared_ptr<Texture2D> ResourceManager::LoadTextureFromFile(const char* file, bool mipmap)
{
	int width, height, channel;
	unsigned int format = 0;

	// stbi_set_flip_vertically_on_load(true); // tell stb_image.h to flip loaded texture's on the y-axis.
	unsigned char* data = stbi_load(file, &width, &height, &channel, 0);		// It reads in an image into a big 1D array of unsigned char.
	if (data)
	{
		switch (channel)
		{
		case 1:
			format = GL_R;
			std::cout << "image format: GL_R" << std::endl;
			break;
		case 2:
			format = GL_RG;
			std::cout << "image format: GL_RG" << std::endl;
			break;
		case 3:
			format = GL_RGB;
			std::cout << "image format: GL_RGB" << std::endl;
			break;
		case 4:
			format = GL_RGBA;
			std::cout << "image format: GL_RGBA" << std::endl;
			break;
		default:
			break;
		}
	}
	else
	{
		fprintf(stderr, "%s @ %d: ERROR: IMAGE LOAD FAIL.\n", __FILE__, __LINE__);
	}

	std::shared_ptr<Texture2D> texture = std::make_shared<Texture2D>(width, height, format, format, data, mipmap);

	stbi_image_free(data);	// free image resource as already load into texture object.

	return texture;
}

#endif // !RESOURCE_MANAGER_H

