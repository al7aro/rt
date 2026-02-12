#version 430 core

layout (local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

layout(rgba32f, binding = 0) uniform image2D img_output;

/* ********** CAMERA ********** */
struct s_camera
{
    vec3 pos;
    mat3 rot;
    float aspect;
    float fov;
};
/* **************************** */
/* *********** RAY ************ */
struct s_ray
{
    vec3 pos;
    vec3 dir;
};
/* **************************** */
/* ********* LIGHTING ********* */
struct s_point_light
{
    vec3 pos;
    vec4 color;
};
/* **************************** */
/* ******* PRIMITIVES ********* */
struct s_sphere
{
    vec3 pos;
    float radius;
    vec4 color;
};
/* **************************** */

uniform s_camera cam;

const int NUM_OF_SPHERES = 2;
s_sphere spheres[NUM_OF_SPHERES];

vec4 ray_trace(s_ray ray)
{
    float offset = 0.2;
    vec4 c = vec4(0.0, 0.0, 0.0, 1.0);

    for (int i = 0; i < 200; i++)
    {
        for (int s = 0; s < NUM_OF_SPHERES; s++)
        {
            float l = length(ray.pos - spheres[s].pos);
            if (l <= spheres[s].radius)
                return (1.5*spheres[s].color / length(cam.pos - spheres[s].pos));
        }
        ray.pos += offset * ray.dir;
    }
    return (c);
}

void scene_setup()
{
    spheres[0].pos = vec3(0.0, 0.0, -5.0);
    spheres[0].radius = 1.0;
    spheres[0].color = vec4(0.3961, 0.8941, 0.3961, 1.0);
    spheres[1].pos = vec3(2.0, 2.0, -5.0);
    spheres[1].radius = 0.5;
    spheres[1].color = vec4(0.9608, 0.2902, 0.2902, 1.0);
}

void main()
{
    scene_setup();

/* SCREEN COORDINATES [-1, 1] */
    vec2 uv;
    vec4 color = vec4(0.0, 0.0, 0.0, 1.0);
    ivec2 texel_coord = ivec2(gl_GlobalInvocationID.xy);
    uv.x = float(texel_coord.x)/(gl_NumWorkGroups.x);
    uv.y = float(texel_coord.y)/(gl_NumWorkGroups.y);
/* SCREEN COORDINATES IN 3D SPACE */
    float w = tan(cam.fov/2.0);
    float h = tan(cam.fov/2.0) / cam.aspect;
    uv = 2.0 * uv - vec2(1.0);
    uv.x *= w;
    uv.y *= h;
    // vec3 screen = cam.rot * vec3(uv.x, uv.y, 0.0) + cam.pos + cam.dir; // THE SAME
    vec3 screen = vec3(uv.xy, -1.0) * cam.rot + cam.pos;
/* RAY DIRECTION */
    s_ray ray;
    ray.pos = screen;
    ray.dir = normalize(screen - cam.pos);
    color = ray_trace(ray);

/* COMPUTE COLOR */
    imageStore(img_output, texel_coord, color);
}