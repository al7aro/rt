#include <stdio.h>
#include <memory>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "engine/rt_engine.hpp"
#include "camera/Camera.hpp"

void update_camera(rt::Window& win, rt::Camera& cam, rt::KeyHandler& wasd, rt::MouseHandler& mouse, float delta_time);

int main(void)
{
    rt::Window win("rt", 800, 800);
    auto wasd = std::make_shared<rt::KeyHandler>(std::vector<int>({ GLFW_KEY_W, GLFW_KEY_A, GLFW_KEY_S, GLFW_KEY_D, GLFW_KEY_SPACE, GLFW_KEY_LEFT_SHIFT }));
    auto mouse = std::make_shared<rt::MouseHandler>(std::vector<int>({GLFW_MOUSE_BUTTON_RIGHT, GLFW_MOUSE_BUTTON_LEFT}));
    win.AddListenTo(wasd);
    win.AddListenTo(mouse);
    rt::Timer timer, aux_timer;
    float delta_time = 0.0;

    rt::Canvas canvas;
    rt::ComputeShader c_sh(ASSETS_DIRECTORY"/shaders/rt/basic.c.glsl", 500, 500);
    rt::Camera camera;

    glClearColor(0.9, 0.6, 0.3, 1.0);
    while (!win.IsRunning())
    {
        timer.Restart();
        win.PollEvents();
        update_camera(win, camera, *wasd, *mouse, delta_time);

        glClear(GL_COLOR_BUFFER_BIT);
        c_sh.Bind();
        c_sh.SetUniform("cam.pos", camera.GetPosition());
        c_sh.SetUniform("cam.aspect", camera.GetAspect());
        c_sh.SetUniform("cam.fov", camera.GetFOV());
        c_sh.SetUniform("cam.rot", camera.GetRotationMatrix());
        c_sh.WaitFinished();
        canvas.Render(c_sh.GetTextureId());

        win.SwapBuffers();
        delta_time = timer.EllapsedSeconds();
        if (aux_timer.EllapsedSeconds() > 0.5)
        {
            win.SetTitleSuffix(" [" + std::to_string(delta_time) + "s | " + std::to_string(1.0/delta_time) + "fps]");
            aux_timer.Restart();
        }
    }
    return (0);
}

void update_camera(rt::Window& win, rt::Camera& cam, rt::KeyHandler& wasd, rt::MouseHandler& mouse, float delta_time)
{
/* ****************** BASIC TEST MOVEMENT ****************** */
    float cam_speed = 7.5 + mouse.GetScrollOffset().y / 10.0;
    glm::vec2 cursor_dir = mouse.GetCursorDir();
    if (wasd.IsKeyDown(GLFW_KEY_W))
        cam.Move(glm::vec3(0.0, 0.0, -1.0) * delta_time * cam_speed);
    if (wasd.IsKeyDown(GLFW_KEY_S))
        cam.Move(glm::vec3(0.0, 0.0, 1.0) * delta_time * cam_speed);
    if (wasd.IsKeyDown(GLFW_KEY_A))
        cam.Move(glm::vec3(-1.0, 0.0, 0.0) * delta_time * cam_speed);
    if (wasd.IsKeyDown(GLFW_KEY_D))
        cam.Move(glm::vec3(1.0, 0.0, 0.0) * delta_time * cam_speed);
    if (wasd.IsKeyDown(GLFW_KEY_LEFT_SHIFT))
        cam.Move(glm::vec3(0.0, -1.0, 0.0) * delta_time * cam_speed);
    if (wasd.IsKeyDown(GLFW_KEY_SPACE))
        cam.Move(glm::vec3(0.0, 1.0, 0.0) * delta_time * cam_speed);
    if (mouse.IsButtonDown(GLFW_MOUSE_BUTTON_LEFT))
    {
        win.SetCursorMode(GLFW_CURSOR_DISABLED);
        cam.Yaw(-cursor_dir.x * 0.005);
        cam.Pitch(-cursor_dir.y * 0.005);
    }
    else
        win.SetCursorMode(GLFW_CURSOR_NORMAL);
/* ********************************************************* */
}