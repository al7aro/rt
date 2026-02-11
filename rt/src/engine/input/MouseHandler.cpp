#include "MouseHandler.hpp"

namespace rt {

    MouseHandler::MouseHandler()
        : InputHandler(true), _pos(0.0, 0.0), _prev_pos(0.0, 0.0), _dir(0.0), _len(0.0), _scroll_offset(0.0)
    {
    }

    MouseHandler::MouseHandler(const std::vector<int>& active_buttons)
        : MouseHandler()
    {
        for (int key : active_buttons)
            _active_buttons[key] = false;
    }

    MouseHandler::~MouseHandler()
    {}

    void MouseHandler::Update()
    {
        _dir = _pos - _prev_pos;
        _len = length(_pos - _prev_pos);
        _prev_pos.x = _pos.x;
        _prev_pos.y = _pos.y;
    }

    void MouseHandler::SetCursorPosition(double xpos, double ypos)
    {
        _pos.x = static_cast<float>(xpos);
        _pos.y = static_cast<float>(ypos);
    }

    void MouseHandler::UpdateScrollState(double xoffset, double yoffset)
    {
        _scroll_offset.x += static_cast<float>(xoffset);
        _scroll_offset.y += static_cast<float>(yoffset);
    }

    void MouseHandler::UpdateButtonState(int button, int action, int mods)
    {
        std::map<int, bool>::iterator it = _active_buttons.find(button);
        if (it != _active_buttons.end())
            it->second = (action != GLFW_RELEASE);
    }

    const vec2 MouseHandler::GetCursorPos() const
    {
        return (_pos);
    }

    const vec2 MouseHandler::GetCursorDir() const
    {
        return (_dir);
    }

    const float MouseHandler::GetCursorLen() const
    {
        return (_len);
    }

    const vec2 MouseHandler::GetCursorPrevPos() const
    {
        return (_pos - _dir);
    }

    const vec2 MouseHandler::GetScrollOffset() const
    {
        return (_scroll_offset);
    }

    void MouseHandler::ResetScrollOffset()
    {
        _scroll_offset = vec2(0.0);
    }

    const bool MouseHandler::IsButtonDown(int button) const
    {
        if (!_is_active)
            return false;
        std::map<int, bool>::const_iterator it = _active_buttons.find(button);
        if (it != _active_buttons.end())
            return (it->second);
        return (false);
    }


}