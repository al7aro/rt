#include <stdio.h>
#include <memory>
#include <random>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb_image.h>
#include <stb_image_write.h>

#include <imgui.h>
#include <imgui_impl_glfw.h>

// --- ADD THIS LINE ---
#define IMGUI_IMPL_OPENGL_LOADER_GLAD 
// ---------------------
#include <imgui_impl_opengl3.h>

#include "engine/rt_engine.hpp"
#include "camera/Camera.hpp"
#include "compute_shader/ComputeShader.hpp"

bool update_camera(rt::Window& win, rt::Camera& cam, rt::KeyHandler& wasd, rt::MouseHandler& mouse, float delta_time);
void setup_scene(rt::Scene& scene);
void export_image(unsigned int id, int w, int h, const std::string& path);
std::string image_filename(float t, float exp, float iso, float k, float n);

int main(void)
{
    int WIDTH = 900;
    int HEIGHT = 900;

    rt::Window win("rt", WIDTH, HEIGHT);
    auto wasd = std::make_shared<rt::KeyHandler>(std::vector<int>({ GLFW_KEY_U, GLFW_KEY_W, GLFW_KEY_A, GLFW_KEY_S, GLFW_KEY_D, GLFW_KEY_SPACE, GLFW_KEY_LEFT_SHIFT, GLFW_KEY_ENTER }));
    auto mouse = std::make_shared<rt::MouseHandler>(std::vector<int>({GLFW_MOUSE_BUTTON_RIGHT, GLFW_MOUSE_BUTTON_LEFT}));
    win.AddListenTo(wasd);
    win.AddListenTo(mouse);
    rt::Timer timer, delta_timer, aux_timer;
    float delta_time = 0.0;
    bool enable_input = false;

    rt::Canvas canvas;
    rt::ComputeShader c_sh(ASSETS_DIRECTORY"/shaders/rt/basic.c.glsl", WIDTH, HEIGHT);
    rt::Camera camera;
    camera.SetAspect(float(WIDTH)/float(HEIGHT));
    camera.SetPosition(glm::vec3(0.0, 0.0, 4.25));

    /* CREATE SCENE */
    bool updated_scene = 0;
    rt::Timer calculation_timer;

    float EXPOSURE_T = 2.;
    float ISO = 200;
    float APERTURE = 3.0;
    float SENSOR_K = 12.5;

    rt::Scene scene;
    setup_scene(scene);

    /* LOAD SCENE TO GPU */
    unsigned int ubo[1];
    glCreateBuffers(1, ubo);
    glNamedBufferData(ubo[0], scene.GetSceneSizeInBytes(), scene.GetData(), GL_STATIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, 2, ubo[0]);
    c_sh.SetUniform("u_shape_cnt", (int)scene.GetShapeCount());

    /* RANDOM */
    srand(static_cast<unsigned int>(28022021));

/* ********** SETUP IMGUI ********** */
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    ImGui_ImplGlfw_InitForOpenGL(win.GetWindowPointer(), true);
    ImGui_ImplOpenGL3_Init("#version 430 core");
    ImGui::StyleColorsDark();
/* ********************************* */
    glClearColor(0.9, 0.6, 0.3, 1.0);
    while (!win.IsRunning())
    {
        if (updated_scene)
        {
            calculation_timer.Restart();
            win.ResetFrameCount();
            updated_scene = 0;
        }
        delta_timer.Restart();
        win.PollEvents();

        if (enable_input)
            updated_scene |= update_camera(win, camera, *wasd, *mouse, delta_time);

        glClear(GL_COLOR_BUFFER_BIT);
/* ********** IMGUI FRAME SETUP ********** */
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
/* ********************************** */
        ImGui::Begin("Camera Details");
        updated_scene |= ImGui::SliderFloat("Exposure time", &EXPOSURE_T, 0.0, 10.0);
        updated_scene |= ImGui::SliderFloat("ISO", &ISO, 0.0, 1000.0);
        updated_scene |= ImGui::SliderFloat("Aperture", &APERTURE, 0.0, 50.0);
        updated_scene |= ImGui::SliderFloat("Sensor Constant", &SENSOR_K, 0.0, 100.0);
        ImGui::Checkbox("Enable Input", &enable_input);
        if (ImGui::Button("Export"))
            export_image(c_sh.GetDisplayTextureId(), c_sh.GetWidth(), c_sh.GetHeight(),
                RT_DIRECTORY"/renders/" + image_filename(calculation_timer.EllapsedSeconds(), EXPOSURE_T, ISO, APERTURE, SENSOR_K));
        ImGui::End();
/* ********************************** */

        c_sh.Bind();
        c_sh.SetUniform("cam.pos", camera.GetPosition());
        c_sh.SetUniform("cam.aspect", camera.GetAspect());
        c_sh.SetUniform("cam.fov", camera.GetFOV());
        c_sh.SetUniform("cam.rot", camera.GetRotationMatrix());
        c_sh.SetUniform("u_time", (float)timer.EllapsedSeconds());
        c_sh.SetUniform("u_rand", static_cast<float>(rand())/static_cast<float>(RAND_MAX));        
        c_sh.SetUniform("u_frame_cnt", (float)win.GetFrameCount());
        c_sh.SetUniform("u_t", EXPOSURE_T);
        c_sh.SetUniform("u_S", ISO);
        c_sh.SetUniform("u_K", SENSOR_K);
        c_sh.SetUniform("u_N", APERTURE);

        c_sh.WaitFinished();
        canvas.Render(c_sh.GetDisplayTextureId(), rt::ComputeShader::DISPLAY_TEXTURE);

/* ********** IMGUI RENDER ********** */
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
/* ********** IMGUI RENDER ********** */

        win.SwapBuffers();
        delta_time = delta_timer.EllapsedSeconds();
        if (aux_timer.EllapsedSeconds() > 0.5)
        {
            win.SetTitleSuffix(" [" + std::to_string(delta_time) + "s | " + std::to_string(1.0/delta_time) + "fps]");
            aux_timer.Restart();
        }
    }
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    return (0);
}

std::string image_filename(float t, float exp, float iso, float k, float n)
{
    std::string ret;
    ret += "t" + std::to_string((int)(t * 100));
    ret += "_exp" + std::to_string((int)(exp * 100));
    ret += "_iso" + std::to_string((int)(iso * 100));
    ret += "_k" + std::to_string((int)(k * 100));
    ret += "_n" + std::to_string((int)(n * 100));
    return (ret + ".hdr");
}

void export_image(unsigned int id, int w, int h, const std::string& path)
{
    std::vector<float> data(w * h * 4);
    glBindTexture(GL_TEXTURE_2D, id);
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, data.data());
    stbi_flip_vertically_on_write(true);
    stbi_write_hdr(path.c_str(), w, h, 4, data.data());
}

void setup_scene(rt::Scene& scene)
{
    // int sph0 = scene.CreateSphere(glm::vec3(-1.25, 1.25, 0.0), 0.5);
    // scene.SetColor(sph0, glm::vec4(0.9, 0.9, 0.9, 1.0));
    // scene.SetModel(sph0, rt::Material::BRDFModel::SPECULAR);
    std::vector<unsigned int> b0 = scene.CreateBox(glm::vec3(-1.0, 1.0, 0.0), 0.5, 0.5, 0.5, glm::vec3(3.14/4.0));
    scene.SetColor(b0, glm::vec4(0.8, 0.0, 0.0, 1.0));
    scene.SetModel(b0, rt::Material::BRDFModel::LAMBERTIAN);

    
    int sph1 = scene.CreateSphere(glm::vec3(0.0, 0.0, 0.0), 1.0);
    scene.SetColor(sph1, glm::vec4(0.75, 0.75, 0.90, 1.0));
    scene.SetModel(sph1, rt::Material::BRDFModel::SPECULAR);
    
    int sph2 = scene.CreateSphere(glm::vec3(1.25, 1.25, 0.0), 0.5);
    scene.SetColor(sph2, glm::vec4(0.8, 0.8, 0.8, 1.0));
    scene.SetModel(sph2, rt::Material::BRDFModel::BLINN_PHONG);
    
    // LIGHT SOURCE
    int sph3 = scene.CreateSphere(glm::vec3(0.0, 2, 0.0), 0.25);
    scene.SetColor(sph3, glm::vec4(glm::vec3(10.0), 1.0));
    scene.SetEmissive(sph3, 1.0);

    int q_bot = scene.CreateQuad(
        glm::vec3(2.0, -2.0, -2.0),
        glm::vec3(-2.0, -2.0, -2.0),
        glm::vec3(-2.0, -2.0, 2.0),
        glm::vec3(2.0, -2.0, 2.0)
    );
    scene.SetColor(q_bot, glm::vec4(0.5, 0.5, 0.5, 1.0));
    int q_top = scene.CreateQuad(
        glm::vec3(-2.0, 2.0, -2.0),
        glm::vec3(2.0, 2.0, -2.0),
        glm::vec3(2.0, 2.0, 2.0),
        glm::vec3(-2.0, 2.0, 2.0)
    );
    scene.SetColor(q_top, glm::vec4(0.5, 0.5, 0.5, 1.0));
    int q_back = scene.CreateQuad(
        glm::vec3(-2.0, -2.0, -2.0),
        glm::vec3(2.0, -2.0, -2.0),
        glm::vec3(2.0, 2.0, -2.0),
        glm::vec3(-2.0, 2.0, -2.0)
    );
    scene.SetColor(q_back, glm::vec4(0.5, 0.5, 0.5, 1.0));
    int q_left = scene.CreateQuad(
        glm::vec3(-2.0, -2.0, -2.0),
        glm::vec3(-2.0, 2.0, -2.0),
        glm::vec3(-2.0, 2.0, 2.0),
        glm::vec3(-2.0, -2.0, 2.0)
    );
    scene.SetColor(q_left, glm::vec4(0.5, 0.5, 0.5, 1.0));
    int q_right = scene.CreateQuad(
        glm::vec3(2.0, 2.0, -2.0),
        glm::vec3(2.0, -2.0, -2.0),
        glm::vec3(2.0, -2.0, 2.0),
        glm::vec3(2.0, 2.0, 2.0)
    );
    scene.SetColor(q_right, glm::vec4(0.5, 0.5, 0.5, 1.0));
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