#include <stdio.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "engine/rt_engine.hpp"

int main(void)
{
    rt::Window win("rt", 800, 800);
    t_canvas canvas = canvas_create(100, 100);

    for (int i = 0; i < 100; i++)
    {
        for (int j = 0; j < 100; j++)
        {
            if (i == j)
                canvas_draw_pixel(&canvas, i, j, (char)255, 0, 0);
            else
                canvas_draw_pixel(&canvas, i, j, 0, 0, 0);
            
        }
    }

    glClearColor(0.9, 0.6, 0.3, 1.0);
    while (!win.IsRunning())
    {
        win.PollEvents();

        glClear(GL_COLOR_BUFFER_BIT);
        canvas_render(&canvas);

        win.SwapBuffers();
    }
    return (0);
}