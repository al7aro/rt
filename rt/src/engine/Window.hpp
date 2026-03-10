#pragma once

#include <string>
#include <vector>
#include <memory>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "input/Input.hpp"
#include "input/MouseHandler.hpp"
#include "input/KeyHandler.hpp"
#include "ogl_debug/Debug.hpp"

namespace rt {

    class Window
    {
    private:
        GLFWwindow* _win;
        std::string _title, _title_suffix;
        int _width, _height;
        long long unsigned int _frame_cnt;

        std::vector<std::shared_ptr<MouseHandler> > _mouse_handlers;
        std::vector<std::shared_ptr<KeyHandler> > _key_handlers;
    public:
        /* Requires a inited GLFW*/
        Window(const std::string& title, int width, int height);

        bool IsRunning() const;
        void PollEvents() const;
        void SwapBuffers();

        void SetTitleSuffix(const std::string& suffix);

        void SetCursorMode(unsigned int value);
        bool IsValid() const;
        void SetClearcolor(float r, float g, float b, float a) const;
        long long unsigned int GetFrameCount() const;
        void ResetFrameCount();

        /* Does not terminate GLFW */
        void Destroy() const;

        GLFWwindow* GetWindowPointer() const;

        /* INPUT */
        void AddListenTo(std::shared_ptr<MouseHandler> handler);
        void AddListenTo(std::shared_ptr<KeyHandler> handler);
        /* CALLBACKS */
        static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
        static void cursor_callback(GLFWwindow* window, double xpos, double ypos);
        static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
        static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
    };

}