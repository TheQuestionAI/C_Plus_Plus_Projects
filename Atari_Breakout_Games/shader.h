#ifndef SHADER_H
#define SHADER_H

#include <glad/glad.h> // include glad to get the required OpenGL headers

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>

class Shader		// checked, no need to check again!
{
public:
	// constructor
	Shader();	// default constructor
	Shader(const char* vertexSource, const char* fragmentSource, const char* geometrySource = nullptr);		// parameter constructor
	~Shader();
public:
	void Compile(const char* vertexSource, const char* fragmentSource, const char* geometrySource = nullptr);	// compile the shader using the source code.

	unsigned int ID() const;		// get the shader program ID.

	void Use();		// sets the current shader as active
	void UnUse();
	bool IsInUse() const;

	// utility functions
	void SetBool(const char* name, bool value);
	void SetInteger(const char* name, int value);
	void SetFloat(const char* name, float value);
	void SetVector2f(const char* name, float x, float y);
	void SetVector2f(const char* name, const glm::vec2& value);
	void SetVector3f(const char* name, float x, float y, float z);
	void SetVector3f(const char* name, const glm::vec3& value);
	void SetVector4f(const char* name, float x, float y, float z, float w);
	void SetVector4f(const char* name, const glm::vec4& value);

	// array的声明对于维度可以全给定、也可全空、也可以最后一个维度给定其他维度推导. 但不可以给定中间维度而最后维度不给.
	void SetBoolv(const char* name, unsigned int count, bool value[]);
	void SetIntegerv(const char* name, unsigned int count, int value[]);
	void SetFloatv(const char* name, unsigned int count, float value[]);
	void SetVector2fv(const char* name, unsigned int count, float value[][2]);		
	void SetVector3fv(const char* name, unsigned int count, float value[][3]);
	void SetVector4fv(const char* name, unsigned int count, float value[][4]);

	void SetMat2x2(const char* name, const glm::mat2& matrix);
	void SetMat3x3(const char* name, const glm::mat3& matrix);
	void SetMat4x4(const char* name, const glm::mat4& matrix);

private:
	unsigned int id;	// record shader program ID.
	bool isActive;

	void CheckCompileErrors(unsigned int shader, const char* type);			// error check function.
	bool CheckUseErrors(const char* func) const;
};

Shader::Shader() : id{ 0 }, isActive{ false } { }
Shader::Shader(const char* vertexSource, const char* fragmentSource, const char* geometrySource) : isActive{ false }
{
	Compile(vertexSource, fragmentSource, geometrySource);
}
Shader::~Shader() { glDeleteProgram(id); }	// A value of 0 for program will be silently ignored.

void Shader::Compile(const char* vertexSource, const char* fragmentSource, const char* geometrySource)
{
	// 1. compile vertex shader、fragment shader and geometry shader.
	unsigned int vertexShaderID, fragmentShaderID, geometryShaderID;

	vertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShaderID, 1, &vertexSource, nullptr);
	glCompileShader(vertexShaderID);
	CheckCompileErrors(vertexShaderID, "VERTEX");

	fragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShaderID, 1, &fragmentSource, nullptr);
	glCompileShader(fragmentShaderID);
	CheckCompileErrors(fragmentShaderID, "FRAGMENT");

	if (geometrySource)
	{
		geometryShaderID = glCreateShader(GL_GEOMETRY_SHADER);
		glShaderSource(geometryShaderID, 1, &geometrySource, nullptr);
		glCompileShader(geometryShaderID);
		CheckCompileErrors(geometryShaderID, "GEOMETRY");
	}

	// 2. link to create the shader program.
	this->id = glCreateProgram();
	glAttachShader(this->id, vertexShaderID);
	glAttachShader(this->id, fragmentShaderID);
	if (geometrySource)
	{
		glAttachShader(this->id, geometryShaderID);
	}
	glLinkProgram(this->id);
	CheckCompileErrors(fragmentShaderID, "PROGRAM");

	// 3. free vertex and fragment shader.
	glDeleteShader(vertexShaderID);
	glDeleteShader(fragmentShaderID);
	if (geometrySource)
	{
		glDeleteShader(geometryShaderID);
	}
}

unsigned int Shader::ID() const
{
	return this->id;
}

void Shader::Use()
{
	glUseProgram(this->id);
	isActive = true;
}
void Shader::UnUse()
{
	glUseProgram(0);
	isActive = false;
}
bool Shader::IsInUse() const { return isActive; }

void Shader::SetBool(const char* name, bool value)
{
	if (CheckUseErrors("SetBool")) return;
	return glUniform1i(glGetUniformLocation(ID(), name), value);
}
void Shader::SetInteger(const char* name, int value)
{
	if (CheckUseErrors("SetInteger")) return;
	return glUniform1i(glGetUniformLocation(ID(), name), value);
}
void Shader::SetFloat(const char* name, float value)
{
	if (CheckUseErrors("SetFloat")) return;
	glUniform1f(glGetUniformLocation(ID(), name), value);
}
void Shader::SetVector2f(const char* name, float x, float y)
{
	if (CheckUseErrors("SetVector2f")) return;
	glUniform2f(glGetUniformLocation(ID(), name), x, y);
}
void Shader::SetVector2f(const char* name, const glm::vec2& value)
{
	if (CheckUseErrors("SetVector2f")) return;
	glUniform2f(glGetUniformLocation(ID(), name), value.x, value.y);
}
void Shader::SetVector3f(const char* name, float x, float y, float z)
{
	if (CheckUseErrors("SetVector3f")) return;
	glUniform3f(glGetUniformLocation(ID(), name), x, y, z);
}
void Shader::SetVector3f(const char* name, const glm::vec3& value)
{
	if (CheckUseErrors("SetVector3f")) return;
	glUniform3f(glGetUniformLocation(ID(), name), value.x, value.y, value.z);
}
void Shader::SetVector4f(const char* name, float x, float y, float z, float w)
{
	if (CheckUseErrors("SetVector4f")) return;
	glUniform4f(glGetUniformLocation(ID(), name), x, y, z, w);
}
void Shader::SetVector4f(const char* name, const glm::vec4& value)
{
	if (CheckUseErrors("SetVector4f")) return;
	glUniform4f(glGetUniformLocation(ID(), name), value.x, value.y, value.z, value.w);
}
void Shader::SetBoolv(const char* name, unsigned int count, bool value[])
{
	if (CheckUseErrors("SetBoolv")) return;
	return glUniform1iv(glGetUniformLocation(ID(), name), count, reinterpret_cast<int*>(value));		// bool*指针类型无法隐式转换成int*类型, 我们强制转换
}
void Shader::SetIntegerv(const char* name, unsigned int count, int value[])
{
	if (CheckUseErrors("SetIntegerv")) return;
	return glUniform1iv(glGetUniformLocation(ID(), name), count, value);
}
void Shader::SetFloatv(const char* name, unsigned int count, float value[])
{
	if (CheckUseErrors("SetIntegerv")) return;
	return glUniform1fv(glGetUniformLocation(ID(), name), count, value);
}
void Shader::SetVector2fv(const char* name, unsigned int count, float value[][2])
{
	if (CheckUseErrors("SetVector2fv")) return;
	return glUniform2fv(glGetUniformLocation(ID(), name), count, reinterpret_cast<float*>(value));			// float(*)[2]类型指针无法转换成float*类型, 我们强制转换.
}
void Shader::SetVector3fv(const char* name, unsigned int count, float value[][3])
{
	if (CheckUseErrors("SetVector3fv")) return;
	return glUniform3fv(glGetUniformLocation(ID(), name), count, reinterpret_cast<float*>(value));			// float(*)[3]类型指针无法转换成float*类型, 我们强制转换.
}
void Shader::SetVector4fv(const char* name, unsigned int count, float value[][4])
{
	if (CheckUseErrors("SetVector4fv")) return;
	return glUniform4fv(glGetUniformLocation(ID(), name), count, reinterpret_cast<float*>(value));			// float(*)[4]类型指针无法转换成float*类型, 我们强制转换.
}
void Shader::SetMat2x2(const char* name, const glm::mat2& matrix)
{
	if (CheckUseErrors("SetMat2x2")) return;
	glUniformMatrix2fv(glGetUniformLocation(ID(), name), 1, GL_FALSE, glm::value_ptr(matrix));
}
void Shader::SetMat3x3(const char* name, const glm::mat3& matrix)
{
	if (CheckUseErrors("SetMat3x3")) return;
	glUniformMatrix3fv(glGetUniformLocation(ID(), name), 1, GL_FALSE, glm::value_ptr(matrix));
}
void Shader::SetMat4x4(const char* name, const glm::mat4& matrix)
{
	if (CheckUseErrors("SetMat4x4")) return;
	glUniformMatrix4fv(glGetUniformLocation(ID(), name), 1, GL_FALSE, glm::value_ptr(matrix));
}

void Shader::CheckCompileErrors(unsigned int shader, const char* type)
{
	int success;
	char infoLog[1024];

	if (type == "PROGRAM")
	{
		glGetProgramiv(shader, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			glGetProgramInfoLog(shader, 1024, nullptr, infoLog);
			std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
		}
	}
	else
	{
		glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			glGetShaderInfoLog(shader, 1024, nullptr, infoLog);
			std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
		}
	}
}
bool Shader::CheckUseErrors(const char* func) const
{
	if (!IsInUse())
	{
		fprintf(stderr, "%s @ %d: fail to call %s() as the shader program is not in use.\n", __FILE__, __LINE__, func);
		return true;
	}

	return false;
}

#endif // !SHADER_H
