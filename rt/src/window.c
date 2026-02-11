#include "window.h"

t_window window_create(int w, int h, const char* title)
{
    t_window win;
    win.w = w;
    win.h = h;
    if (!glfwInit())
    {
        win.win = (void*)0;
        return (win);
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    win.win = glfwCreateWindow(500, 500, title, (void*)0, (void*)0);
    if (!win.win)
    {
        glfwTerminate();
        return (win);
    }
    glfwMakeContextCurrent(win.win);
    if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        win.win = (void*)0;
        glfwTerminate();
    }
    return (win);
}

int window_should_close(t_window win)
{
    return (glfwWindowShouldClose(win.win));
}

void window_swap_buffers(t_window win)
{
    glfwSwapBuffers(win.win);
}

void window_poll_events(t_window win)
{
    glfwPollEvents();
}