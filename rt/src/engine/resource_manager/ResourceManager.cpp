#include "ResourceManager.hpp"

namespace rt {

    ResourceManager::ResourceManager() {}
    ResourceManager::~ResourceManager() {}

    std::string ResourceManager::read_file(const std::string& path)
    {
        std::ifstream file(path);
        std::string src, line;

        while (std::getline(file, line))
            src += (line + "\n");
        return (src);
    }

    // TextureSource ResourceManager::read_image_file(const std::string& path)
    // {
    //     TextureSource src;
    //     src.filename = path;
    //     stbi_set_flip_vertically_on_load(true);
    //     src.data = stbi_load(path.c_str(), &src.width, &src.height, &src.chn, 4);
    //     return (src);
    // }
}