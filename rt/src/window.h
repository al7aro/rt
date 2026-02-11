#ifndef WINDOW_H
# define WINDOW_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>

typedef struct s_window
{
    GLFWwindow* win;
    const char* title;
    int w;
    int h;
} t_window;

t_window window_create(int w, int h, const char* title);
int window_should_close(t_window win);
void window_swap_buffers(t_window win);
void window_poll_events(t_window win);

#endif