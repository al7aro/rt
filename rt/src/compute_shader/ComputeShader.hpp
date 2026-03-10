#pragma once

#include <string>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glad/glad.h>

#include "../engine/resource_manager/ResourceManager.hpp"
#include "../scene/Scene.hpp"

namespace rt {

    class ComputeShader
    {
    public:
        static constexpr int DISPLAY_TEXTURE = 0;
        static constexpr int ACCUMULATED_TEXTURE = 1;
    private:

        unsigned int _id;
        unsigned int _tex[2];
        unsigned int _width, _height;

    public:
        ComputeShader(const std::string& path, int w, int h);

        void Bind();
        unsigned int GetDisplayTextureId() const;
        unsigned int GetAccumulatedTextureId() const;
        void WaitFinished() const;
        unsigned int GetWidth() const;
        unsigned int GetHeight() const;

        void SetUniform(const std::string& name, const glm::mat3& v) const;
        void SetUniform(const std::string& name, const glm::vec3& v) const;
        void SetUniform(const std::string& name, float v) const;
        void SetUniform(const std::string& name, int v) const;
    };

}