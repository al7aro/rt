#pragma once

#include <string>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
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

        void SetUniform(const std::string& name, const glm::mat3& v) const;
        void SetUniform(const std::string& name, const glm::vec3& v) const;
        void SetUniform(const std::string& name, float v) const;
    };

}