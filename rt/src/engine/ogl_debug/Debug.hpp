#pragma once

#include <iostream>

#include <glad/glad.h>

namespace rt {

    void APIENTRY DebugOutput(GLenum source, 
                            GLenum type, 
                            unsigned int id, 
                            GLenum severity, 
                            GLsizei length, 
                            const char *message, 
                            const void *userParam);
}