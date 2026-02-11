#pragma once

#include <string>

#include <glad/glad.h>

#include "resource_manager/ResourceManager.hpp"

namespace rt {

    class ComputeShader
    {
    private:
        unsigned int _id;
        unsigned int _tex;
        unsigned int _width, _height;

    public:
        ComputeShader(const std::string& path, int w, int h);

        void Bind();
        unsigned int GetTextureId() const;
        void WaitFinished() const;
    };

}