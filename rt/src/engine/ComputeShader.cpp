#include "ComputeShader.hpp"

namespace rt {

    ComputeShader::ComputeShader(const std::string& path, int w, int h)
    {
        _width = w;
        _height = h;
        _id = glCreateProgram();
        unsigned int c_sh = glCreateShader(GL_COMPUTE_SHADER);
        std::string c_str = ResourceManager::read_file(path).c_str();
        const char* c_src = c_str.c_str();
        glShaderSource(c_sh, 1, &c_src, nullptr);
        glCompileShader(c_sh);
        glAttachShader(_id, c_sh);
        glLinkProgram(_id);
        glDeleteShader(c_sh);

        /* INIT TEXTURE FOR WRITTING */
        glCreateTextures(GL_TEXTURE_2D, 1, &_tex);
        glBindTextureUnit(0, _tex);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA32F, _width, _height);
        glBindImageTexture(0, _tex, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
    }

    void ComputeShader::Bind()
    {
        glUseProgram(_id);
    }

    unsigned int ComputeShader::GetTextureId() const
    {
        return (_tex);
    }

    void ComputeShader::WaitFinished() const
    {
        glDispatchCompute((unsigned int)_width, (unsigned int)_height, 1);
    }

    void ComputeShader::SetUniform(const std::string& name, const glm::mat3& v) const
    {
        unsigned int loc = glGetUniformLocation(_id, name.c_str());
        glProgramUniformMatrix3fv(_id, loc, 1, GL_FALSE, glm::value_ptr(v));
    }

    void ComputeShader::SetUniform(const std::string& name, const glm::vec3& v) const
    {
        unsigned int loc = glGetUniformLocation(_id, name.c_str());
        glProgramUniform3fv(_id, loc, 1, glm::value_ptr(v));
    }
    void ComputeShader::SetUniform(const std::string& name, float v) const
    {
        unsigned int loc = glGetUniformLocation(_id, name.c_str());
        glProgramUniform1f(_id, loc, v);
    }

}