// 001_BreakOutGame.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>

#include "window.h"
#include "game.h"
#include "utility.h"

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);

const unsigned int WIDTH = 1920;
const unsigned int HEIGHT = 1080;

Window window(WIDTH, HEIGHT, "BreakOut");
Game breakOut(WIDTH, HEIGHT);

int main()
{
    window.SetKeyCallback(key_callback);

    // render loop
    while (!window.ShouldClose())
    {
        // At beginning of each frame rendering loop, calculate the deltatime.
        Timer::Update();
        
        // process user input
        breakOut.ProcessInput(Timer::DeltaTime());

        // update program state.
        breakOut.Update(Timer::DeltaTime());
        
        // render frame.
        // (1) clear previous rendering.
        window.ClearWindow();    // default clear color = (0.0f, 0.0f, 0.0f, 1.0f);
        // (2) perform rendering command;
        breakOut.Render();
        
        // (3) glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        window.SwapBufferAndPollEvent();
    }

    return 0;
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, true);
    }

    if (key >= 0 && key < 1024)
    {  
        if (action == GLFW_PRESS)
        {
            breakOut.KeyPress(key);
        }
        if (action == GLFW_RELEASE)
        {
            breakOut.KeyRelease(key);
        }
    }
}
