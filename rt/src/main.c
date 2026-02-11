#include <stdio.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "window.h"
#include "canvas.h"

int main(void)
{
    t_window win = window_create(1000, 1000, "rt");
    t_canvas canvas = canvas_create(100, 100);

    for (int i = 0; i < 100; i++)
    {
        for (int j = 0; j < 100; j++)
        {
            if (i == j)
                canvas_draw_pixel(&canvas, i, j, 255, 0, 0);
            else
                canvas_draw_pixel(&canvas, i, j, 0, 0, 0);
            
        }
    }

    glClearColor(0.9, 0.6, 0.3, 1.0);
    while (!window_should_close(win))
    {
        window_poll_events(win);

        glClear(GL_COLOR_BUFFER_BIT);
        canvas_render(&canvas);

        window_swap_buffers(win);
    }
    return (0);
}