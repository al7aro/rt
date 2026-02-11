#include "KeyHandler.hpp"

namespace rt {

    KeyHandler::KeyHandler()
        : InputHandler(true)
    {
    }

    KeyHandler::KeyHandler(const std::vector<int>& active_keys)
        : InputHandler(true)
    {
        for (int key : active_keys)
            _active_keys[key] = false;
    }

    KeyHandler::~KeyHandler()
    {}

    void KeyHandler::UpdateState(int key, int scancode, int action, int mods)
    {
        std::map<int, bool>::iterator it = _active_keys.find(key);
        if (it != _active_keys.end())
            it->second = (action != GLFW_RELEASE);
    }

    const bool KeyHandler::IsKeyDown(int key) const
    {
        if (!_is_active)
            return false;
        std::map<int, bool>::const_iterator it = _active_keys.find(key);
        if (it != _active_keys.end())
            return (it->second);
        return (false);
    }

}