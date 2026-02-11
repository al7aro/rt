#include "Canvas.hpp"

void canvas_init_texture(t_canvas* canvas)
{
    canvas->tex_data = (unsigned char*)malloc(canvas->w * canvas->h * sizeof(int));
    glCreateTextures(GL_TEXTURE_2D, 1, &canvas->tex);
    glTextureParameteri(canvas->tex, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTextureParameteri(canvas->tex, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTextureParameteri(canvas->tex, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTextureParameteri(canvas->tex, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTextureStorage2D(canvas->tex, 1, GL_RGBA8, canvas->w, canvas->h);
}

t_canvas canvas_create(unsigned int w, unsigned int  h)
{
    t_canvas canvas;
    canvas.w = w;
    canvas.h = h;
    float quad[5 * 6] = {
        -1.0, -1.0, 0.0, 0.0, 0.0,    // BOTTOM LEFT
         1.0, -1.0, 0.0, 1.0, 0.0,    // BOTTOM RIGHT
         1.0,  1.0, 0.0, 1.0, 1.0,    // TOP RIGHT

        -1.0, -1.0, 0.0, 0.0, 0.0,    // BOTTOM LEFT
         1.0,  1.0, 0.0, 1.0, 1.0,    // TOP RIGHT
        -1.0,  1.0, 0.0, 0.0, 1.0     // TOP LEFT
    };
    glCreateVertexArrays(1, &canvas.vao);
    glCreateBuffers(1, &canvas.vbo);
    glNamedBufferData(canvas.vbo, sizeof(quad), quad, GL_STATIC_DRAW);
    /* POS ATTRIBUTE */
    glEnableVertexArrayAttrib(canvas.vao, 0);
    glVertexArrayAttribFormat(canvas.vao, 0, 3, GL_FLOAT, GL_FALSE, 0);
    glVertexArrayAttribBinding(canvas.vao, 0, 0);
    /* TEX COORD ATTRIBUTE */
    glEnableVertexArrayAttrib(canvas.vao, 1);
    glVertexArrayAttribFormat(canvas.vao, 1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 3);
    glVertexArrayAttribBinding(canvas.vao, 1, 0);
    glVertexArrayVertexBuffer(canvas.vao, 0, canvas.vbo, 0, sizeof(float) * 5);
    const char* v_src = "#version 460 core\n\
        layout (location = 0) in vec3 a_pos;\n\
        layout (location = 1) in vec2 a_tex_coord;\n\
        out vec2 v_tex_coord;\n\
        void main()\n\
        {\n\
            v_tex_coord = a_tex_coord;\n\
            gl_Position = vec4(a_pos, 1.0);\n\
        }\n";
    const char* f_src = "#version 460 core\n\
        out vec4 frag_color;\n\
        in vec2 v_tex_coord;\n\
        uniform sampler2D u_texture;\n\
        void main()\n\
        {\n\
            frag_color = vec4(0.5, 0.5, 0.5, 1.0);\n\
            frag_color = vec4(v_tex_coord.x, v_tex_coord.y, 1.0, 1.0);\n\
            frag_color = texture(u_texture, v_tex_coord);\n\
        }\n";
    canvas.program = glCreateProgram();
    canvas.v_sh = glCreateShader(GL_VERTEX_SHADER);
    canvas.f_sh = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(canvas.v_sh, 1, &v_src, nullptr);
    glShaderSource(canvas.f_sh, 1, &f_src, nullptr);
    glCompileShader(canvas.v_sh);
    glCompileShader(canvas.f_sh);
    glAttachShader(canvas.program, canvas.v_sh);
    glAttachShader(canvas.program, canvas.f_sh);
    glLinkProgram(canvas.program);
    glDeleteShader(canvas.v_sh);
    glDeleteShader(canvas.f_sh);
    canvas_init_texture(&canvas);
    return (canvas);
}

void canvas_draw_pixel(t_canvas* canvas, int x, int y, char r, char g, char b)
{
    canvas->tex_data[4 * (x + y * canvas->w) + 0] = r;
    canvas->tex_data[4 * (x + y * canvas->w) + 1] = g;
    canvas->tex_data[4 * (x + y * canvas->w) + 2] = b;
    canvas->tex_data[4 * (x + y * canvas->w) + 3] = 255;
}

void canvas_draw_image(t_canvas* canvas, unsigned int* data,
    int offset_x, int offset_y, int size_x, int size_y)
{
    for (int i = offset_x; i < size_x; i++)
    {
        for (int j = offset_y; j < size_y; j++)
        {
            int index = 4 * (i + j * canvas->w);
            canvas->tex_data[index + 0] = data[index + 0];
            canvas->tex_data[index + 1] = data[index + 1];
            canvas->tex_data[index + 2] = data[index + 2];
            canvas->tex_data[index + 3] = data[index + 3];
        }
    }
}

void canvas_render(t_canvas* canvas)
{
    glTextureSubImage2D(canvas->tex, 0, 0, 0,
        canvas->w, canvas->h, GL_RGBA, GL_UNSIGNED_BYTE, canvas->tex_data);
    glUseProgram(canvas->program);
    glBindTextureUnit(0, canvas->tex);
    unsigned int loc = glGetUniformLocation(canvas->program, "u_texture");
    glProgramUniform1i(canvas->program, loc, 0);
    glBindVertexArray(canvas->vao);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}
