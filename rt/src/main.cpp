#include <stdio.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "engine/rt_engine.hpp"

int main(void)
{
    rt::Window win("rt", 800, 800);
    rt::Canvas canvas;
    rt::ComputeShader c_sh(ASSETS_DIRECTORY"/shaders/basic.c.glsl", 500, 500);

    glClearColor(0.9, 0.6, 0.3, 1.0);
    while (!win.IsRunning())
    {
        win.PollEvents();

        glClear(GL_COLOR_BUFFER_BIT);
        c_sh.Bind();
        c_sh.WaitFinished();
        canvas.Render(c_sh.GetTextureId());

        win.SwapBuffers();
    }
    return (0);
}