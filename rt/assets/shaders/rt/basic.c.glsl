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
/* ********* LIGHTING ********* */
struct s_light_point
{
    vec3 pos;
    vec4 color;
};
/* **************************** */
/* ******* PRIMITIVES ********* */
struct s_shape
{
    int type; // SPHERE IS TYPE 0
    vec3 pos;
    float radius;
    vec4 color;
};
/* **************************** */
/* *********** RAY ************ */
struct s_ray
{
    vec3 pos;
    vec3 dir;
};
struct s_hit
{
    int hit;
    vec3 pos;
    vec3 normal;
    // s_material material;
    vec4 color;
    s_shape shape;
};
/* **************************** */

uniform s_camera cam;
uniform float u_frame_cnt;

// A LIST OF !!!VISIBLE!!! SHAPES SHOULD BE SENT FROM CPU
const int NUM_OF_SHAPES = 2;
s_shape shapes[NUM_OF_SHAPES];
const int NUM_OF_POINT_LIGHTS = 1;
s_light_point lights[NUM_OF_POINT_LIGHTS];

s_hit intersect_scene(s_ray ray)
{
    s_hit hit;
    float offset = 0.2;
    vec4 c = vec4(0.0, 0.0, 0.0, 1.0);
    hit.color = c;
    hit.hit = 0;
    hit.pos = vec3(0.0);
    hit.normal = vec3(0.0);
    for (int i = 0; i < 200; i++)
    {
        for (int s = 0; s < NUM_OF_SHAPES; s++)
        {
            /* IF SHAPE IS SPHERE */
            if (shapes[s].type == 0)
            {
                vec3 normal = ray.pos - shapes[s].pos;
                if (length(normal) <= shapes[s].radius)
                {
                    hit.pos = ray.pos;
                    hit.normal = normal;
                    hit.color = shapes[s].color;
                    hit.hit = 1;
                    return (hit);
                }
            }
        }
        ray.pos += offset * ray.dir;
    }
    return (hit);
}

vec4 ray_trace(s_ray ray)
{
    s_hit hit;

    // Find what light is reaching this pixel (no bounces)
    hit = intersect_scene(ray);

    // adds the contribution of light bouncing on a chain of objects objects
    // for (int depth = 0; depth < 3; depth++) {
    //}
    return (hit.color);
}

void scene_setup()
{
    shapes[0].type = 0;
    shapes[0].pos = vec3(0.0, 0.0, -5.0);
    shapes[0].radius = 1.0;
    shapes[0].color = vec4(0.3961, 0.8941, 0.3961, 1.0);
    shapes[1].type = 0;
    shapes[1].pos = vec3(2.0, 2.0, -5.0);
    shapes[1].radius = 0.5;
    shapes[1].color = vec4(0.9608, 0.2902, 0.2902, 1.0);

    lights[0].pos = vec3(5.0, 5.0, 5.0);
    lights[0].color = vec4(1.0, 1.0, 1.0, 1.0);
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
    vec4 prev_color = imageLoad(img_output, texel_coord);
    if (int(u_frame_cnt) > 1)
        color += prev_color;
    imageStore(img_output, texel_coord, color);
}