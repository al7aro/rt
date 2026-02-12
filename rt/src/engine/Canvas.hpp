#pragma once

#include <stdlib.h>

#include <glad/glad.h>

#include "resource_manager/ResourceManager.hpp"

namespace rt {

    class Canvas
    {
    private:
        unsigned int _vao, _vbo;
        unsigned int _program, _v_sh, _f_sh;
    public:
        Canvas();
        ~Canvas();

        void Render(unsigned int texture) const;
        void SetUniform(const std::string& name, float v) const;
    };

}
