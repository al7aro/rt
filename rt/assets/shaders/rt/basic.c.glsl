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
/* ********* MATERIAL ********* */
struct s_material
{
    vec4 color;
    int emissive;
    float p0;
    float p1;
    float p2;
};
/* **************************** */
/* ******* PRIMITIVES ********* */
// ALL PRIMITIVES HAS COLOR (MATERIAL) AND POS
const int SHAPE_POINT = 0;
const int SHAPE_SPHERE = 1;
const int SHAPE_PLANE = 2;
// SPHERE : type 0
//      - float (radius) -> f0
// PLANE : type 1
//      - vec3 (v1)
//      - vec3 (v2)
//      - vec3 (v3)
struct s_shape
{
                    // BASE ALIGNMENT           // OFFSET
    vec3 pos;       //  16                  -   0
    int type;       //  4 (padding)
    vec3 v0;        //  16                  -   16
    int enabled;    //  4 (padding)
    vec3 v1;        //  16                  -   32
    float f0;       //  4 (padding)
    vec3 v2;        //  16                  -   48
    float f1;       //  4 (padding)
    s_material mat; //  16                  -   64  (COLOR)
                    //  4                   -   68  (EMISSIVE)
                    //  4                   -   72  (PADDING)
                    //  4                   -   76  (PADDING)
                    //  4                   -   80  (PADDING)
                    // TOTAL = 80 + 4 = 84

    // vec3 pos;    //  16                  -   96
};

const int MAX_SHAPES = 10;
layout (std140, binding = 0) uniform u_scene
{
    //          BASE ALIGNMENT  OFFSET
    //  [0]:    16              0
    //  [2]:    16              112
    //  [3]:    16              224
    s_shape u_shapes[MAX_SHAPES];
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
    float dist;
    vec3 pos;
    vec3 normal;

    s_material mat;
    int enabled;
};
/* **************************** */

uniform float u_frame_cnt;
uniform float u_time;
uniform float u_rand;
uniform s_camera cam;
// const int MAX_NUM_OF_SHAPES = 3;
// uniform int u_shape_cnt;
// uniform s_shape shapes[MAX_NUM_OF_SHAPES];

// // A LIST OF !!!VISIBLE!!! SHAPES SHOULD BE SENT FROM CPU
const vec4 AMBIENT_LIGHT_COLOR = vec4(0.1, 0.1, 0.1, 1.0);
const int RAY_MAX_BOUNCES = 100;
const int RAY_NO_HIT = -1;
const int NUM_OF_SHAPES = 3;
s_shape shapes[NUM_OF_SHAPES];

// This gives the same number per frame so seed should change per different value
float random(float seed)
{
    return fract(sin((seed + u_rand) * 12.9898) * 43758.5453123);
}

vec3 random_vector(float seed)
{
    float z = random(seed) * 2.0 - 1.0;          // Range -1 to 1
    float a = random(seed + 0.123) * 6.283185;   // Range 0 to 2*PI
    float r = sqrt(1.0 - z * z);
    return vec3(r * cos(a), r * sin(a), z);
}

s_hit intersect_sphere(s_ray ray, int s)
{
    s_hit hit;
    hit.hit = RAY_NO_HIT;

    vec3 normal = ray.pos - shapes[s].pos;
    float b = dot(normal, ray.dir);
    float c = dot(normal, normal) - (shapes[s].f0 * shapes[s].f0);
    float discriminant = b * b - c;

    if (discriminant > 0.0)
    {
        // Calculate the distance to the closest hit point
        float t = -b - sqrt(discriminant);
        
        // If t is negative, the hit is behind the camera, so we ignore it
        if (t > 0.001) 
        {
            hit.dist = t;
            hit.pos = ray.pos + t * ray.dir;
            hit.normal = normalize(hit.pos - shapes[s].pos);
            hit.mat = shapes[s].mat;
            hit.hit = s;
            hit.enabled = shapes[s].enabled;
        }
    }
    return hit;
}

s_hit intersect_scene(s_ray ray, int ignore, int ignore_disabled_objects)
{
    s_hit closest_hit;
    closest_hit.dist = 9999;
    closest_hit.hit = RAY_NO_HIT;
    closest_hit.enabled = 0;
    closest_hit.mat.color = AMBIENT_LIGHT_COLOR;

    for (int s = 0; s < NUM_OF_SHAPES; s++)
    {
        if (s == ignore)
            continue;
        s_hit current_hit;
        if (shapes[s].type == SHAPE_SPHERE)
            current_hit = intersect_sphere(ray, s);
        // Check if this hit is closer than the previous one
        if (current_hit.enabled == 0 && ignore_disabled_objects == 1)
            continue;
        if (current_hit.hit != RAY_NO_HIT && current_hit.dist < closest_hit.dist)
            closest_hit = current_hit;
    }
    return closest_hit;
}

vec3 random_bounce(s_material mat, float seed)
{
    return (random_vector(seed));
}

vec4 ray_trace(s_ray ray)
{
    s_hit hit;
    float seed = dot(ray.pos.xy, vec2(12.9898, 78.233)) + u_time;
    vec4 c = vec4(0.0);
    vec4 m = vec4(1.0);

    hit = intersect_scene(ray, RAY_NO_HIT, 1);
    if (hit.hit == RAY_NO_HIT) return (AMBIENT_LIGHT_COLOR);
    if (hit.enabled == 0) return (AMBIENT_LIGHT_COLOR);
    if (hit.mat.emissive == 1) return (hit.mat.color);
    m *= hit.mat.color;
    for (int b = 0; b < RAY_MAX_BOUNCES; b++)
    {
        // RANDOM BOUNCE DIRECTION
        ray.dir = normalize(random_bounce(hit.mat, seed + b));
        // ray.dir = normalize(shapes[2].pos - hit.pos);
        if (dot(hit.normal, ray.dir) < 0) ray.dir = -ray.dir;
        ray.pos = hit.pos;

        // INTERSECT NEW RAY WITH THE SCENE
        s_hit new_hit = intersect_scene(ray, hit.hit, 0);

        // COMPUTE LIGHT
        float diffuse = max(dot(hit.normal, ray.dir), 0.0); // IF LIGHT IS NOT POINT ADDITIONAL TERM IS NEEDED
        m *= new_hit.mat.color * diffuse;

        // CHECK IF RAY IS GOING TO SKY OR LIGHT
        if (new_hit.hit == RAY_NO_HIT || new_hit.mat.emissive != 0)
            break;
        hit = new_hit;
    }
    return (m);
}

void scene_setup()
{
    shapes[0].type = SHAPE_SPHERE;
    shapes[0].pos = vec3(0.0, 0.0, -3.0);
    shapes[0].f0 = 1.0;
    shapes[0].mat.color = vec4(0.3961, 0.8941, 0.3961, 1.0);
    shapes[0].mat.emissive = 0;
    shapes[0].enabled = 1;
    shapes[1].type = SHAPE_SPHERE;
    shapes[1].pos = vec3(1.25, 1.25, -3.0);
    shapes[1].f0 = 0.5;
    shapes[1].mat.color = vec4(1.0, 0.3843, 0.5882, 1.0);
    shapes[1].mat.emissive = 0;
    shapes[1].enabled = 1;
    shapes[2].type = SHAPE_SPHERE;
    shapes[2].pos = vec3(0.0, 2.0, 0.0);
    shapes[2].f0 = 1.0;
    shapes[2].mat.color = 10.0*vec4(1.0);
    shapes[2].mat.emissive = 1;
    shapes[2].enabled = 0;
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
    color = u_shapes[1].mat.color;
    imageStore(img_output, texel_coord, color);
}