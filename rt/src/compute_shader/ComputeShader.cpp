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
        glCreateTextures(GL_TEXTURE_2D, MAX_TEXTURES, _tex);
        for (int i = 0; i < MAX_TEXTURES; i++)
        {
            glActiveTexture(GL_TEXTURE0 + i);
            glBindTextureUnit(i, _tex[i]);
            glTextureParameteri(_tex[i], GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTextureParameteri(_tex[i], GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTextureParameteri(_tex[i], GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTextureParameteri(_tex[i], GL_TEXTURE_MIN_FILTER, GL_NEAREST);

            glTextureStorage2D(_tex[i], 1, GL_RGBA32F, _width, _height);
            glBindImageTexture(i, _tex[i], 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
        }
    }

    unsigned int ComputeShader::GetWidth() const
    {
        return (_width);
    }
    unsigned int ComputeShader::GetHeight() const
    {
        return (_height);
    }

    void ComputeShader::Bind()
    {
        glUseProgram(_id);
    }

    unsigned int ComputeShader::GetDisplayTextureId() const
    {
        return (_tex[DISPLAY_TEXTURE]);
    }

    unsigned int ComputeShader::GetVarianceTextureId() const
    {
        return (_tex[VARIANCE_TEXTURE]);
    }

    unsigned int ComputeShader::GetMeanTextureId() const
    {
        return (_tex[MEAN_TEXTURE]);
    }

    unsigned int ComputeShader::GetSum2TextureId() const
    {
        return (_tex[SUM2_TEXTURE]);
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
    void ComputeShader::SetUniform(const std::string& name, const glm::vec4& v) const
    {
        unsigned int loc = glGetUniformLocation(_id, name.c_str());
        glProgramUniform4fv(_id, loc, 1, glm::value_ptr(v));
    }
    void ComputeShader::SetUniform(const std::string& name, float v) const
    {
        unsigned int loc = glGetUniformLocation(_id, name.c_str());
        glProgramUniform1f(_id, loc, v);
    }
    void ComputeShader::SetUniform(const std::string& name, int v) const
    {
        unsigned int loc = glGetUniformLocation(_id, name.c_str());
        glProgramUniform1i(_id, loc, v);
    }

}