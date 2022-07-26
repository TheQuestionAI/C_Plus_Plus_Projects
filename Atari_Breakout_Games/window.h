#ifndef WINDOW_H
#define WINDOW_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <string>
#include <glm/glm.hpp>

void error_callback(int code, const char* description);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);

enum class Primitive
{
	POINT,
	LINE,
	TRIANGLE,
	QUAD,
	PATCH
};

class Window		// checked, no need to check again!
{
public:
	Window();
	Window(unsigned int w, unsigned int h, const char* name = "", GLFWmonitor* monitor = nullptr, GLFWwindow* share = nullptr);
	~Window();
public:
	void Init(unsigned int w, unsigned int h, const char* name = "", GLFWmonitor* monitor = nullptr, GLFWwindow* share = nullptr);
	
	bool ShouldClose() const;
	//void SetWindowClosed();
	GLFWwindow* RawPointer() const;
	
	void SwapBufferAndPollEvent();

	void SetKeyCallback(void (*key_callback)(GLFWwindow* win, int key, int scancode, int action, int mode));

	void EnableDepthTest(GLenum func = GL_LESS);
	bool IsDepthTestEnabled() const;

	void ClearWindow(float x = 0.0f, float y = 0.0f, float z = 0.0f, float alpha = 1.0f);

	void Draw(Primitive primitive, unsigned int first, unsigned int count);

private:
	GLFWwindow* window;
	unsigned int width, height;
	std::string title;
	// 暂不记录monitor和share. 以后要记录的话直接加上即可.

	bool isDepthTestEnabled;
};

Window::Window() : window{ nullptr }, width{ 0 }, height{ 0 }, title{ "" }, isDepthTestEnabled{ false } { }

Window::Window(unsigned int w, unsigned int h, const char* name, GLFWmonitor* monitor, GLFWwindow* share)
{
	Init(w, h, name, monitor, share);
}

Window::~Window() { glfwTerminate(); }

void Window::Init(unsigned int w, unsigned int h, const char* name, GLFWmonitor* monitor, GLFWwindow* share)
{
	this->width = w;
	this->height = h;
	this->title = name;
	this->isDepthTestEnabled = false;

	glfwSetErrorCallback(error_callback);       // error_callback是唯一一个可以在glfw创建GL context和渲染窗口前注册的callback函数.
	if (!glfwInit())
	{
		fprintf(stderr, "%s @ %d: fail to call glfwInit() to initialize the GLFW libary.\n", __FILE__, __LINE__);
		return;
	}

	// window configuration setting.
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	this->window = glfwCreateWindow(this->width, this->height, this->title.c_str(), monitor, share);
	if (!this->window)
	{
		fprintf(stderr, "%s @ %d: fail to call glfwCreateWindow() to create the OpenGL context and rendering window.\n", __FILE__, __LINE__);
		glfwTerminate();
		return;
	}
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);		// Register window related callback function.

	glfwMakeContextCurrent(window);		// make the created GL context to be the current context.

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		fprintf(stderr, "%s @ %d: fail to call gladLoadGLLoader() to load OpenGL API address.\n", __FILE__, __LINE__);
		glfwTerminate();
		return;
	}

	// Enable various of testing. 
	// glEnable(GL_DEPTH_TEST);	// 2D游戏, no need to enable depth testing.
	glEnable(GL_BLEND);			// enable blending mode, so alpha value will take effect.
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

bool Window::ShouldClose() const
{
	return glfwWindowShouldClose(this->window);
}

GLFWwindow* Window::RawPointer() const
{
	return this->window;
}

void Window::SetKeyCallback(void (*key_callback)(GLFWwindow* win, int key, int scancode, int action, int mode))
{
	glfwSetKeyCallback(this->window, key_callback);
}

void Window::SwapBufferAndPollEvent()
{
	glfwSwapBuffers(this->window);
	glfwPollEvents();
}

void Window::EnableDepthTest(GLenum func)
{
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(func);
	this->isDepthTestEnabled = true;
}

bool Window::IsDepthTestEnabled() const 
{ 
	return isDepthTestEnabled; 
}

void Window::ClearWindow(float x, float y, float z, float alpha)
{
	auto mode = GL_COLOR_BUFFER_BIT;
	if (IsDepthTestEnabled())
	{
		mode |= GL_DEPTH_BUFFER_BIT;
	}

	glClearColor(x, y, z, alpha);
	glClear(mode);
}

void Window::Draw(Primitive primitive, unsigned int first, unsigned int count)
{
	unsigned int shape = 0;
	switch (primitive)
	{
	case Primitive::POINT:
		shape = GL_POINTS;
		break;
	case Primitive::LINE:
		shape = GL_LINES;
		break;
	case Primitive::TRIANGLE:
		shape = GL_TRIANGLES;
		break;
	case Primitive::QUAD:
		shape = GL_QUADS;
		break;
	case Primitive::PATCH:
		shape = GL_PATCHES;
		break;
	default:
		fprintf(stderr, "%s @ %d: fail to call Draw() as the input draw primitive is unknown.\n", __FILE__, __LINE__);
		return;
	}
	glDrawArrays(shape, first, count);
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ callback function ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void error_callback(int code, const char* description)
{
	fprintf(stderr, "GLFW Error: %s.\n", description);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);        // glViewport函数接受的是分辨率参数, 即每行每列视窗口有多少个pixel number.
}

#endif // !WINDOW_H

