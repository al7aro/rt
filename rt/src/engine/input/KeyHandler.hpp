#pragma once

#include "Input.hpp"

namespace rt
{
    class KeyHandler : public InputHandler
    {
    private:
        std::map<int, bool> _active_keys;
    public:
        KeyHandler();
        KeyHandler(const std::vector<int>& active_keys);
        ~KeyHandler();
        void UpdateState(int key, int scancode, int action, int mods);
        const bool IsKeyDown(int key) const;
    };

}