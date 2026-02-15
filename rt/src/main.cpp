#include <stdio.h>
#include <memory>
#include <random>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "engine/rt_engine.hpp"
#include "camera/Camera.hpp"
#include "compute_shader/ComputeShader.hpp"

bool update_camera(rt::Window& win, rt::Camera& cam, rt::KeyHandler& wasd, rt::MouseHandler& mouse, float delta_time);

int main(void)
{
    rt::Window win("rt", 800, 800);
    auto wasd = std::make_shared<rt::KeyHandler>(std::vector<int>({ GLFW_KEY_W, GLFW_KEY_A, GLFW_KEY_S, GLFW_KEY_D, GLFW_KEY_SPACE, GLFW_KEY_LEFT_SHIFT }));
    auto mouse = std::make_shared<rt::MouseHandler>(std::vector<int>({GLFW_MOUSE_BUTTON_RIGHT, GLFW_MOUSE_BUTTON_LEFT}));
    win.AddListenTo(wasd);
    win.AddListenTo(mouse);
    rt::Timer timer, delta_timer, aux_timer;
    float delta_time = 0.0;

    rt::Canvas canvas;
    rt::ComputeShader c_sh(ASSETS_DIRECTORY"/shaders/rt/basic.c.glsl", 800, 800);
    rt::Camera camera;

    float test0[] = {
        1.0, 0.0, 0.0,      /* pos */
        0.0,                /* type (padding) */
        1.0, 0.0, 0.0,      /* v0*/
        0.0,                /* enabled (padding) */
        1.0, 0.0, 0.0,      /* v1 */
        0.0,                /* f0 (padding) */
        1.0, 0.0, 0.0,      /* v2 */
        1.0,                /* f1 (padding) */
        0.0, 0.0, 1.0, 1.0, /* mat.color */
        1.0,                /* mat.emissive */
        0.0, 0.0, 0.0,       /* (padding) */

        0.0, 0.0, 0.0, /* padding */

        1.0, 0.0, 0.0,      /* pos */
        0.0,                /* type (padding) */
        1.0, 0.0, 0.0,      /* v0*/
        0.0,                /* enabled (padding) */
        1.0, 0.0, 0.0,      /* v1 */
        0.0,                /* f0 (padding) */
        1.0, 0.0, 0.0,      /* v2 */
        1.0,                /* f1 (padding) */
        0.0, 0.0, 1.0, 1.0, /* mat.color */
        1.0,                /* mat.emissive */
        0.0, 0.0, 0.0       /* (padding) */
    };
    unsigned int ubo[1];
    glCreateBuffers(1, ubo);
    glNamedBufferData(ubo[0], 84*2+12, test0, GL_STATIC_DRAW);
    glBindBufferRange(GL_UNIFORM_BUFFER, 0, ubo[0], 0, 84*2+12);
    // glBindBufferRange(GL_UNIFORM_BUFFER, 0, ubo[0], 256, 84);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, ubo[0]);

    /* RANDOM */
    srand(static_cast<unsigned int>(28022021));

    glClearColor(0.9, 0.6, 0.3, 1.0);
    while (!win.IsRunning())
    {
        delta_timer.Restart();
        win.PollEvents();
        if (update_camera(win, camera, *wasd, *mouse, delta_time))
            win.ResetFrameCount();

        glClear(GL_COLOR_BUFFER_BIT);
        c_sh.Bind();
        c_sh.SetUniform("cam.pos", camera.GetPosition());
        c_sh.SetUniform("cam.aspect", camera.GetAspect());
        c_sh.SetUniform("cam.fov", camera.GetFOV());
        c_sh.SetUniform("cam.rot", camera.GetRotationMatrix());
        c_sh.SetUniform("u_frame_cnt", (float)win.GetFrameCount());
        c_sh.SetUniform("u_time", (float)timer.EllapsedSeconds());
        c_sh.SetUniform("u_rand", static_cast<float>(rand())/static_cast<float>(RAND_MAX));
        c_sh.WaitFinished();

        canvas.SetUniform("u_frame_cnt", win.GetFrameCount());
        canvas.Render(c_sh.GetTextureId());

        win.SwapBuffers();
        delta_time = delta_timer.EllapsedSeconds();
        if (aux_timer.EllapsedSeconds() > 0.5)
        {
            win.SetTitleSuffix(" [" + std::to_string(delta_time) + "s | " + std::to_string(1.0/delta_time) + "fps]");
            aux_timer.Restart();
        }
    }
    return (0);
}

bool update_camera(rt::Window& win, rt::Camera& cam, rt::KeyHandler& wasd, rt::MouseHandler& mouse, float delta_time)
{
    float cam_speed = 7.5 + mouse.GetScrollOffset().y / 10.0;
    glm::vec2 cursor_dir = mouse.GetCursorDir();
    bool updated = false;
    if (wasd.IsKeyDown(GLFW_KEY_W))
    {
        cam.Move(glm::vec3(0.0, 0.0, -1.0) * delta_time * cam_speed);
        updated = true;
    }
    if (wasd.IsKeyDown(GLFW_KEY_S))
    {
        cam.Move(glm::vec3(0.0, 0.0, 1.0) * delta_time * cam_speed);
        updated = true;
    }
    if (wasd.IsKeyDown(GLFW_KEY_A))
    {
        cam.Move(glm::vec3(-1.0, 0.0, 0.0) * delta_time * cam_speed);
        updated = true;
    }
    if (wasd.IsKeyDown(GLFW_KEY_D))
    {
        cam.Move(glm::vec3(1.0, 0.0, 0.0) * delta_time * cam_speed);
        updated = true;
    }
    if (wasd.IsKeyDown(GLFW_KEY_LEFT_SHIFT))
    {
        cam.Move(glm::vec3(0.0, -1.0, 0.0) * delta_time * cam_speed);
        updated = true;
    }
    if (wasd.IsKeyDown(GLFW_KEY_SPACE))
    {
        cam.Move(glm::vec3(0.0, 1.0, 0.0) * delta_time * cam_speed);
        updated = true;
    }
    if (mouse.IsButtonDown(GLFW_MOUSE_BUTTON_LEFT))
    {
        win.SetCursorMode(GLFW_CURSOR_DISABLED);
        cam.Yaw(-cursor_dir.x * 0.005);
        cam.Pitch(-cursor_dir.y * 0.005);
        if (glm::length(cursor_dir) > 0.0)
            updated = true;
    }
    else
        win.SetCursorMode(GLFW_CURSOR_NORMAL);
    return (updated);
}