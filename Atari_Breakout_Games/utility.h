#ifndef UTILITY_H
#define UTILITY_H

#include <GLFW/glfw3.h>     // This header provides all the constants, types and function prototypes of the GLFW API.

class Timer
{
public:
	static float Time();
	static float DeltaTime();
	static void Update();
private:
	static float deltaTime;
	static float lastFrameTime;
};

float Timer::deltaTime = 0.0f;
float Timer::lastFrameTime = 0.0f;

float Timer::Time() { return static_cast<float>(glfwGetTime()); }

float Timer::DeltaTime()
{
	return deltaTime;
}

void Timer::Update()
{
	float currentTime = static_cast<float>(glfwGetTime());
	deltaTime = currentTime - lastFrameTime;
	lastFrameTime = currentTime;
}

#endif // !UTILITY_H

