#include "Window.hpp"

namespace rt {

    Window::Window(const std::string& title, int width, int height)
        : _win(nullptr), _title(title), _title_suffix(), _width(width), _height(height)
    {
        if (!glfwInit())
            return ;
        _win = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
        if (!_win)
            return ;
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, true);
        glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        glfwMakeContextCurrent(_win);
        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
        {
            glfwDestroyWindow(_win);
            _win = nullptr;
        }
        glfwSwapInterval(0);
        glClearColor(0.0, 0.0, 0.0, 0.0);

        /* DEBUG SETUP */
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallback(DebugOutput, nullptr);
        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);

        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        
        /* SET CALLBACKS */
        glfwSetWindowUserPointer(_win, this);
        glfwSetKeyCallback(_win, key_callback);
        glfwSetCursorPosCallback(_win, cursor_callback);
        glfwSetScrollCallback(_win, scroll_callback);
        glfwSetMouseButtonCallback(_win, mouse_button_callback);
    }

    void Window::SetClearcolor(float r, float g, float b, float a) const
    {
        glClearColor(r, g, b, a);
    }

    bool Window::IsValid() const
    {
        return (_win != nullptr);
    }

    bool Window::IsRunning() const
    {
        if (!_win) return (false);
        return (glfwWindowShouldClose(_win));
    }
    void Window::PollEvents() const
    {
        glfwPollEvents();
        for (std::shared_ptr<MouseHandler> h : _mouse_handlers)
            h->Update();
    }
    void Window::SwapBuffers() const
    {
        glfwSwapBuffers(_win);
    }

    void Window::Destroy() const
    {
        glfwDestroyWindow(_win);
    }

    void Window::SetCursorMode(unsigned int value)
    {
        glfwSetInputMode(_win, GLFW_CURSOR, value);
    }

    void Window::SetTitleSuffix(const std::string& suffix)
    {
        _title_suffix = suffix;
        glfwSetWindowTitle(_win, (_title + _title_suffix).c_str());
    }

    /* INPUT */
    void Window::AddListenTo(std::shared_ptr<MouseHandler> handler)
    {
        _mouse_handlers.push_back(handler);
    }
    void Window::AddListenTo(std::shared_ptr<KeyHandler> handler)
    {
        _key_handlers.push_back(handler);
    }

    /* CALLBACKS */
    void Window::key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
    {
        Window* w = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
        if (!w)
            return ;
        for (std::shared_ptr<KeyHandler> h : w->_key_handlers)
            h->UpdateState(key, scancode, action, mods);
    }
    void Window::cursor_callback(GLFWwindow* window, double xpos, double ypos)
    {
        Window* w = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
        if (!w)
            return ;
        for (std::shared_ptr<MouseHandler> h : w->_mouse_handlers)
            h->SetCursorPosition(xpos, ypos);
    }
    
    void Window::scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
    {
        Window* w = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
        if (!w)
            return ;
        for (std::shared_ptr<MouseHandler> h : w->_mouse_handlers)
            h->UpdateScrollState(xoffset, yoffset);
    }

    void Window::mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
    {
        Window* w = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
        if (!w)
            return ;
        for (std::shared_ptr<MouseHandler> h : w->_mouse_handlers)
            h->UpdateButtonState(button, action, mods);
    }

}