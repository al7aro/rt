#include <stdio.h>
#include <memory>
#include <random>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "engine/rt_engine.hpp"
#include "camera/Camera.hpp"
#include "compute_shader/ComputeShader.hpp"

bool update_camera(rt::Window& win, rt::Camera& cam, rt::KeyHandler& wasd, rt::MouseHandler& mouse, float delta_time);
void setup_scene(rt::Scene& scene);

int main(void)
{
    rt::Window win("rt", 900, 900);
    auto wasd = std::make_shared<rt::KeyHandler>(std::vector<int>({ GLFW_KEY_U, GLFW_KEY_W, GLFW_KEY_A, GLFW_KEY_S, GLFW_KEY_D, GLFW_KEY_SPACE, GLFW_KEY_LEFT_SHIFT, GLFW_KEY_ENTER }));
    auto mouse = std::make_shared<rt::MouseHandler>(std::vector<int>({GLFW_MOUSE_BUTTON_RIGHT, GLFW_MOUSE_BUTTON_LEFT}));
    win.AddListenTo(wasd);
    win.AddListenTo(mouse);
    rt::Timer timer, delta_timer, aux_timer;
    float delta_time = 0.0;

    rt::Canvas canvas;
    rt::ComputeShader c_sh(ASSETS_DIRECTORY"/shaders/rt/basic.c.glsl", 900, 900);
    rt::Camera camera;

    /* CREATE SCENE */
    float EXPOSURE_T = 0.5;
    float ISO = 200;
    float SENSOR_K = 12.5;
    float APERTURE = 4.5;

    rt::Scene scene;
    setup_scene(scene);

    /* LOAD SCENE TO GPU */
    unsigned int ubo[1];
    glCreateBuffers(1, ubo);
    glNamedBufferData(ubo[0], scene.GetSceneSizeInBytes(), scene.GetData(), GL_STATIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, 1, ubo[0]);
    c_sh.SetUniform("u_shape_cnt", (int)scene.GetShapeCount());

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

        canvas.SetUniform("u_t", EXPOSURE_T);
        canvas.SetUniform("u_S", ISO);
        canvas.SetUniform("u_K", SENSOR_K);
        canvas.SetUniform("u_N", APERTURE);

        canvas.SetUniform("u_frame_cnt", (float)win.GetFrameCount());
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

void setup_scene(rt::Scene& scene)
{
    // int sph0 = scene.CreateSphere(glm::vec3(0.0, -0.5, -3.0), 0.5);
    // scene.SetColor(sph0, glm::vec4(0.0, 0.0, 1.0, 1.0));
    // int sph1 = scene.CreateSphere(glm::vec3(0.0, -10.5, -3.0), 9.5);
    // scene.SetColor(sph1, glm::vec4(0.0, 1.0, 0.0, 1.0));
    int sph0 = scene.CreateSphere(glm::vec3(-1.25, 1.25, -3.0), 0.5);
    scene.SetColor(sph0, glm::vec4(0.0, 0.0, 1.0, 1.0));
    int sph1 = scene.CreateSphere(glm::vec3(0.0, 0.0, -3.0), 1.0);
    scene.SetColor(sph1, glm::vec4(0.0, 1.0, 0.0, 1.0));
    int sph2 = scene.CreateSphere(glm::vec3(1.25, 1.25, -3.0), 0.5);
    scene.SetColor(sph2, glm::vec4(1.0, 0.0, 0.0, 1.0));
    int sph3 = scene.CreateSphere(glm::vec3(0.0, 2.0, 0.0), 1.0);
    scene.SetColor(sph3, glm::vec4(glm::vec3(10.0), 1.0));
    scene.SetEmissive(sph3, 1.0);
    /* INFINITE PLANE BOTTOM */
    int pl0 = scene.CreatePlane(glm::vec3(0.0, -1.0, 0.0), glm::vec3(0.0, 1.0, 0.0));
    scene.SetColor(pl0, glm::vec4(1.0));
    // /* INFINITE PLANE TOP */
    int pl1 = scene.CreatePlane(glm::vec3(0.0, 5.0, 0.0), glm::vec3(0.0, -1.0, 0.0));
    scene.SetColor(pl1, glm::vec4(1.0));
    /* INFINITE PLANE LEFT */
    int pl2 = scene.CreatePlane(glm::vec3(-5.0, 0.0, 0.0), glm::vec3(1.0, 0.0, 0.0));
    scene.SetColor(pl2, glm::vec4(1.0));
    /* INFINITE PLANE RIGHT */
    int pl3 = scene.CreatePlane(glm::vec3(5.0, 0.0, 0.0), glm::vec3(-1.0, 0.0, 0.0));
    scene.SetColor(pl3, glm::vec4(1.0));
    /* INFINITE PLANE BACK */
    int pl4 = scene.CreatePlane(glm::vec3(0.0, 0.0, -10.0), glm::vec3(0.0, 0.0, 1.0));
    scene.SetColor(pl4, glm::vec4(1.0));
}

bool update_camera(rt::Window& win, rt::Camera& cam, rt::KeyHandler& wasd, rt::MouseHandler& mouse, float delta_time)
{
    float cam_speed = 7.5;
    glm::vec2 cursor_dir = mouse.GetCursorDir();
    bool updated = false;
    if (wasd.IsKeyDown(GLFW_KEY_U))
        updated = true;
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