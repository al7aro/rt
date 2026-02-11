#ifndef CANVAS_H
# define CANVAS_H

#include <stdlib.h>

#include <glad/glad.h>

typedef struct s_canvas
{
    unsigned int vao, vbo;
    unsigned int program, v_sh, f_sh, tex;
    unsigned char* tex_data;
    unsigned int w;
    unsigned int h;
} t_canvas;

t_canvas canvas_create(unsigned int w, unsigned int h);
void canvas_render(t_canvas* canvas);
void canvas_draw_pixel(t_canvas* canvas, int x, int y, char r, char g, char b);

#endif