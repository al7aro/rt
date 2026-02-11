#include "Canvas.hpp"

namespace rt {

    Canvas::~Canvas() {}

    Canvas::Canvas()
    {
        float quad[5 * 6] = {
            -1.0, -1.0, 0.0, 0.0, 0.0,    // BOTTOM LEFT
             1.0, -1.0, 0.0, 1.0, 0.0,    // BOTTOM RIGHT
             1.0,  1.0, 0.0, 1.0, 1.0,    // TOP RIGHT

            -1.0, -1.0, 0.0, 0.0, 0.0,    // BOTTOM LEFT
             1.0,  1.0, 0.0, 1.0, 1.0,    // TOP RIGHT
            -1.0,  1.0, 0.0, 0.0, 1.0     // TOP LEFT
        };
        glCreateVertexArrays(1, &_vao);
        glCreateBuffers(1, &_vbo);
        glNamedBufferData(_vbo, sizeof(quad), quad, GL_STATIC_DRAW);
        /* POS ATTRIBUTE */
        glEnableVertexArrayAttrib(_vao, 0);
        glVertexArrayAttribFormat(_vao, 0, 3, GL_FLOAT, GL_FALSE, 0);
        glVertexArrayAttribBinding(_vao, 0, 0);
        /* TEX COORD ATTRIBUTE */
        glEnableVertexArrayAttrib(_vao, 1);
        glVertexArrayAttribFormat(_vao, 1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 3);
        glVertexArrayAttribBinding(_vao, 1, 0);
        glVertexArrayVertexBuffer(_vao, 0, _vbo, 0, sizeof(float) * 5);
        std::string v_str = ResourceManager::read_file(ASSETS_DIRECTORY"/shaders/canvas/quad.v.glsl");
        const char* v_src = v_str.c_str();
        std::string f_str = ResourceManager::read_file(ASSETS_DIRECTORY"/shaders/canvas/quad.f.glsl");
        const char* f_src = f_str.c_str();
        _program = glCreateProgram();
        _v_sh = glCreateShader(GL_VERTEX_SHADER);
        _f_sh = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(_v_sh, 1, &v_src, nullptr);
        glShaderSource(_f_sh, 1, &f_src, nullptr);
        glCompileShader(_v_sh);
        glCompileShader(_f_sh);
        glAttachShader(_program, _v_sh);
        glAttachShader(_program, _f_sh);
        glLinkProgram(_program);
        glDeleteShader(_v_sh);
        glDeleteShader(_f_sh);
    }

    void Canvas::Render(unsigned int texture) const
    {
        glUseProgram(_program);
        glBindTextureUnit(0, texture);
        glBindVertexArray(_vao);
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }

}