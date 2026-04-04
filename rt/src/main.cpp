#include <stdio.h>
#include <memory>
#include <map>
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
void setup_scene0(rt::Scene& scene);
void setup_scene1(rt::Scene& scene);
void setup_scene2(rt::Scene& scene);
void cornellbox_scene(rt::Scene& scene);
void export_image(unsigned int id, int w, int h, const std::string& path);
std::string image_filename(float t, float exp, float iso, float k, float n);
void export_data(const std::vector<std::pair<float, float> >& variance, const std::string& path);
float get_variance(unsigned int id, int w, int h);
float get_mean_color(unsigned int id, int w, int h, int radius);

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
    bool display_mode = false;
    unsigned int display_texture = 0;
    std::vector<std::pair<float, float> > variance_data, mean_color_data;
    int export_start = 0;
    int export_end = 0;
    char file_name[512] = "filename\0";
    bool exporting = true;
    bool importance_sampling = true;
    bool mean_color_export = true;

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
    // cornellbox_scene(scene);
    // setup_scene0(scene);
    // setup_scene1(scene);
    setup_scene2(scene);

    /* LOAD SCENE TO GPU */
    unsigned int ubo[1];
    glCreateBuffers(1, ubo);
    glNamedBufferData(ubo[0], scene.GetSceneSizeInBytes(), scene.GetData(), GL_STATIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, 5, ubo[0]);
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
        ImGui::SliderFloat("Exposure time", &EXPOSURE_T, 0.0, 10.0);
        ImGui::SliderFloat("ISO", &ISO, 0.0, 1000.0);
        ImGui::SliderFloat("Aperture", &APERTURE, 0.0, 50.0);
        ImGui::SliderFloat("Sensor Constant", &SENSOR_K, 0.0, 100.0);
        ImGui::Checkbox("Input", &enable_input);
        ImGui::SameLine();
        ImGui::Checkbox("Importance Sampling", &importance_sampling);
        ImGui::Checkbox("Display Variance", &display_mode);
        ImGui::SameLine();
        ImGui::Checkbox("Export mean color", &mean_color_export);
        display_texture = c_sh.GetDisplayTextureId();
        if (display_mode)
            display_texture = c_sh.GetVarianceTextureId();
        ImGui::PushItemWidth(100);
        ImGui::InputText(":", file_name, 16);
        ImGui::SameLine();
        if (!exporting)
        {
            variance_data.clear();
            mean_color_data.clear();
            if (ImGui::Button("Start Data Export"))
            {
                exporting = !exporting;
                export_start = win.GetFrameCount();
            }
        }
        else if (exporting)
        {
            if (!(win.GetFrameCount() % 10)) // one sample every 5 frames
            {
                variance_data.push_back(std::make_pair((float)win.GetFrameCount(), get_variance(c_sh.GetVarianceTextureId(), c_sh.GetWidth(), c_sh.GetHeight())));
                if (mean_color_export)
                    mean_color_data.push_back(std::make_pair((float)win.GetFrameCount(), get_mean_color(c_sh.GetDisplayTextureId(), c_sh.GetWidth(), c_sh.GetHeight(), 10)));
            }
            if (ImGui::Button("End Data Export"))
            {
                export_end = win.GetFrameCount();
                export_data(variance_data, RT_DIRECTORY"/renders/variance/" + std::string(file_name)  + "_t" + std::to_string(export_start) +  "_" + std::to_string(export_end) + ".csv");
                if (mean_color_export)
                    export_data(mean_color_data, RT_DIRECTORY"/renders/mean_color/" + std::string(file_name)  + "_t" + std::to_string(export_start) +  "_" + std::to_string(export_end) + ".csv");
                exporting = !exporting;
                export_start = 0;
                export_end = 0;
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Render"))
            export_image(display_texture, c_sh.GetWidth(), c_sh.GetHeight(),
                RT_DIRECTORY"/renders/" + std::string(file_name)  + "_" + image_filename(calculation_timer.EllapsedSeconds(), EXPOSURE_T, ISO, APERTURE, SENSOR_K));
        ImGui::SameLine();
        updated_scene |= ImGui::Button("Reload");

        ImGui::End();

        /* VARIANCE DATA EXPORT */
        /********************** */
/* ********************************** */

        c_sh.Bind();
        c_sh.SetUniform("cam.pos", camera.GetPosition());
        c_sh.SetUniform("cam.aspect", camera.GetAspect());
        c_sh.SetUniform("cam.fov", camera.GetFOV());
        c_sh.SetUniform("cam.rot", camera.GetRotationMatrix());
        c_sh.SetUniform("u_time", (float)timer.EllapsedSeconds());
        c_sh.SetUniform("u_rand", static_cast<float>(rand())/static_cast<float>(RAND_MAX));
        c_sh.SetUniform("u_frame_cnt", (float)win.GetFrameCount());
        c_sh.SetUniform("u_ambient_light_color", scene.GetAmbientColor());
        c_sh.SetUniform("u_importance_sampling", float(importance_sampling));

        c_sh.WaitFinished();
        canvas.SetUniform("u_t", EXPOSURE_T);
        canvas.SetUniform("u_S", ISO);
        canvas.SetUniform("u_K", SENSOR_K);
        canvas.SetUniform("u_N", APERTURE);
        canvas.Render(display_texture, rt::ComputeShader::DISPLAY_TEXTURE);

/* ********** IMGUI RENDER ********** */
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
/* ********** IMGUI RENDER ********** */

        win.SwapBuffers();
        delta_time = delta_timer.EllapsedSeconds();
        if (aux_timer.EllapsedSeconds() > 0.5)
        {
            win.SetTitleSuffix(" [" + std::to_string(delta_time) + "s | " + std::to_string(1.0/delta_time) + "fps | " + std::to_string(win.GetFrameCount()) + " frames]");
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

void export_data(const std::vector<std::pair<float, float> >& data, const std::string& path)
{
    if (data.empty()) return;
    std::ofstream file(path);
    // file << "#frame" << ", " << "data" << "\n";
    for (auto v : data)
        file << v.first << ", " << v.second << "\n";
    file.close();
}

float get_variance(unsigned int id, int w, int h)
{
    std::vector<float> data(w * h * 4);
    glBindTexture(GL_TEXTURE_2D, id);
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, data.data());
    float total = 0.0;
    for (float p : data)
        total += p;
    total /= data.size();
    return (total);
}

float get_mean_color(unsigned int id, int w, int h, int radius)
{
    std::vector<glm::vec4> data(w * h * 4);
    glBindTexture(GL_TEXTURE_2D, id);
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, data.data());
    float total = 0.0;
    int i = 0, cnt = 0;
    for (const auto d : data)
    {
        float tmp = (d.r+d.g+d.b)/3.0;
        float x = float(i / w) - float(w/2);
        float y = float(i % w) - float(h/2);
        if (x*x + y*y < radius*radius)
        {
            total += tmp;
            cnt++;
        }
        i++;
    }
    total /= float(cnt);
    return (total);
}

void export_image(unsigned int id, int w, int h, const std::string& path)
{
    std::vector<float> data(w * h * 4);
    glBindTexture(GL_TEXTURE_2D, id);
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, data.data());
    stbi_flip_vertically_on_write(true);
    stbi_write_hdr(path.c_str(), w, h, 4, data.data());
}

void cornellbox_scene(rt::Scene& scene)
{
    // LIGHT SOURCE
    float tmp = 0.75;
    int q_light = scene.CreateQuad(glm::vec3(-tmp, 2.01, tmp), glm::vec3(-tmp, 2.01, -tmp), glm::vec3(tmp, 2.01, -tmp), glm::vec3(tmp, 2.01, tmp));
    scene.SetColor(q_light, glm::vec4(glm::vec3(10.0), 1.0));
    scene.SetEmissive(q_light, 1.0);

    int q_bot = scene.CreateQuad(
        glm::vec3(2.0, -2.0, -2.0),
        glm::vec3(-2.0, -2.0, -2.0),
        glm::vec3(-2.0, -2.0, 2.0),
        glm::vec3(2.0, -2.0, 2.0)
    );
    scene.SetColor(q_bot, glm::vec4(0.725));

    float l_size = 0.4;
    int q_top1 = scene.CreateQuad( glm::vec3(-2.0, 2.0, -2.0), glm::vec3(2.0, 2.0, -2.0), glm::vec3(2.0, 2.0, -l_size), glm::vec3(-2.0, 2.0, -l_size));
    scene.SetColor(q_top1, glm::vec4(0.725));
    int q_top2 = scene.CreateQuad( glm::vec3(-2.0, 2.0, l_size), glm::vec3(2.0, 2.0, l_size), glm::vec3(2.0, 2.0, 2.0), glm::vec3(-2.0, 2.0, 2.0));
    scene.SetColor(q_top2, glm::vec4(0.725));
    int q_top3 = scene.CreateQuad( glm::vec3(-2.0, 2.0, -2.0), glm::vec3(-l_size, 2.0, -2.0), glm::vec3(-l_size, 2.0, 2.0), glm::vec3(-2.0, 2.0, 2.0));
    scene.SetColor(q_top3, glm::vec4(0.725));
    int q_top4 = scene.CreateQuad( glm::vec3(l_size, 2.0, -2.0), glm::vec3(2.0, 2.0, -2.0), glm::vec3(2.0, 2.0, 2.0), glm::vec3(l_size, 2.0, 2.0));
    scene.SetColor(q_top4, glm::vec4(0.725));

    int q_back = scene.CreateQuad(
        glm::vec3(-2.0, -2.0, -2.0),
        glm::vec3(2.0, -2.0, -2.0),
        glm::vec3(2.0, 2.0, -2.0),
        glm::vec3(-2.0, 2.0, -2.0)
    );
    scene.SetColor(q_back, glm::vec4(0.725));
    int q_left = scene.CreateQuad(
        glm::vec3(-2.0, -2.0, -2.0),
        glm::vec3(-2.0, 2.0, -2.0),
        glm::vec3(-2.0, 2.0, 2.0),
        glm::vec3(-2.0, -2.0, 2.0)
    );
    scene.SetColor(q_left, glm::vec4(0.75, 0.05, 0.05, 1.0));
    int q_right = scene.CreateQuad(
        glm::vec3(2.0, 2.0, -2.0),
        glm::vec3(2.0, -2.0, -2.0),
        glm::vec3(2.0, -2.0, 2.0),
        glm::vec3(2.0, 2.0, 2.0)
    );
    scene.SetColor(q_right, glm::vec4(0.15, 0.65, 0.15, 1.0));
}

void setup_scene0(rt::Scene& scene)
{
    auto b0 = scene.CreateBox(glm::vec3(-0.6, -1.0, -0.6), glm::vec3(1.0, 2.0, 1.0), glm::vec3(3.14/6.0, 0.0, 0.0));
    scene.SetColor(b0, glm::vec4(0.6, 0.6, 0.6, 1.0));
    scene.SetModel(b0, rt::Material::BRDFModel::SPECULAR);
    scene.SetParamM(b0, 500);
    scene.SetRefracti(b0, 15);
    scene.SetKd(b0, 0.1);
    scene.SetKs(b0, 0.9);
    auto b1 = scene.CreateBox(glm::vec3(0.6, -1.5, 0.6), glm::vec3(1.0), glm::vec3(-3.14/6.0, 0.0, 0.0));
    scene.SetColor(b1, glm::vec4(0.6, 0.6, 0.6, 1.0));
    scene.SetModel(b1, rt::Material::BRDFModel::LAMBERTIAN);
}

void setup_scene1(rt::Scene& scene)
{
    auto b0 = scene.CreateSphere(glm::vec3(-0.6, -1.0, -0.6), 1.0);
    scene.SetColor(b0, glm::vec4(0.6, 0.6, 0.6, 1.0));
    scene.SetModel(b0, rt::Material::BRDFModel::COOK_TORRANCE);
    scene.SetParamM(b0, 0.1);
    scene.SetRefracti(b0, 15);
    scene.SetKd(b0, 0.25);
    scene.SetKs(b0, 0.75);
    auto b1 = scene.CreateSphere(glm::vec3(0.75, -1.5, 0.75), 0.5);
    scene.SetColor(b1, glm::vec4(0.6, 0.6, 0.6, 1.0));
    scene.SetModel(b1, rt::Material::BRDFModel::BLINN_PHONG);
}

void setup_scene2(rt::Scene& scene)
{
    scene.SetAmbientColor(glm::vec4(1.0));
    auto b0 = scene.CreateSphere(glm::vec3(0.0), 1.0);
    scene.SetColor(b0, glm::vec4(0.8, 0.8, 0.8, 1.0));
    scene.SetModel(b0, rt::Material::BRDFModel::BLINN_PHONG);
    scene.SetParamM(b0, 0.1);
    scene.SetRefracti(b0, 15);
    scene.SetKd(b0, 0.01);
    scene.SetKs(b0, 0.99);
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