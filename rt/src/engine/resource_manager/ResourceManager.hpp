#pragma once

#include <string>
#include <iostream>
#include <sstream>
#include <fstream>

namespace rt {

    class ResourceManager
    {
    public:
        ResourceManager();
        ~ResourceManager();
        static std::string read_file(const std::string& path);
    };

}