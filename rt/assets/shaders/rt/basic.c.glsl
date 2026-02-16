#version 430 core

layout (local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

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
// SPHERE           : type = 1
//      - int (type)        ->  type
//      - float (radius)    ->  f0
// PLANE (infinite) : type = 2
//      - int (type)        ->  type
//      - vec3 (normal)     ->  v0

struct s_shape
{
                    // BASE ALIGNMENT           // OFFSET
    vec3 pos;       //  16                  -   0
    int type;       //  4 (padding)
    vec3 v0;        //  16                  -   16
    float f0;       //  4 (padding)
    vec3 v1;        //  16                  -   32
    float f1;       //  4 (padding)
    vec3 v2;        //  16                  -   48
    float f2;       //  4 (padding)
    vec3 v3;        //  16                  -   48+16
    float f3;       //  4 (padding)
    s_material mat; //  16                  -   64+16  (COLOR)
                    //  4                   -   80+16  (EMISSIVE)
                    //  4                   -   84+16  (PADDING)
                    //  4                   -   88+16  (PADDING)
                    //  4                   -   92+16  (PADDING)
                    // TOTAL = 92+16 + 4 = 96+16
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
    float dist;     // Distance from ray origin
    s_ray ray;      // Ray that originated this intersection
    vec3 pos;       // Position of intersection point
    vec3 normal;    // Normal vector at intersection point
};
/* **************************** */

const int SHAPE_POINT = 0;
const int SHAPE_SPHERE = 1;
const int SHAPE_PLANE = 2;
const vec4 AMBIENT_LIGHT_COLOR = vec4(0.1, 0.1, 0.1, 1.0);
const int RAY_MAX_BOUNCES = 20;
const int RAY_NO_HIT = -1;
const int MAX_SHAPES = 10;

uniform float u_gamma;
uniform float u_frame_cnt;
uniform float u_time;
uniform float u_rand;
uniform s_camera cam;
uniform int u_shape_cnt;
layout(rgba32f, binding = 0) uniform image2D u_img_output;
layout (std140, binding = 1) uniform u_scene
{
    s_shape u_shapes[MAX_SHAPES];
};

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

/* INFINITE PLANE - SPHERE INTERSECTION */
s_hit intersect_plane(s_ray ray, int s)
{
    s_hit hit;
    vec3 p0;
    vec3 p1;
    vec3 p_co;
    vec3 p_no;
    p0 = ray.pos;
    p1 = ray.pos + ray.dir;
    p_co = u_shapes[s].pos;
    p_no = u_shapes[s].v0;
    hit.hit = RAY_NO_HIT;

    vec3 u = p1 - p0;
    float dotp = dot(p_no, u);

    if (abs(dotp) >= 0.001) // RECTA Y PLANO -> PARALELOS
    {
        vec3 w = p0 - p_co;
        float fac = -dot(p_no, w) / dotp;
        if (fac >= 0.0)
        {
            u = u * fac;
            hit.pos = p0 + u;
            hit.hit = s;
            hit.dist = length(hit.pos - ray.pos);
            hit.normal = p_no;
            hit.ray = ray;
        }
    }
    return (hit);
}

/* RAY - SPHERE INTERSECTION */
s_hit intersect_sphere(s_ray ray, int s)
{
    s_hit hit;
    hit.hit = RAY_NO_HIT;

    vec3 normal = ray.pos - u_shapes[s].pos;
    float b = dot(normal, ray.dir);
    float c = dot(normal, normal) - (u_shapes[s].f0 * u_shapes[s].f0);
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
            hit.normal = normalize(hit.pos - u_shapes[s].pos);
            hit.hit = s;
            hit.ray = ray;
        }
    }
    return hit;
}

s_hit intersect_scene(s_ray ray, int ignore)
{
    s_hit closest_hit;
    closest_hit.dist = 9999;
    closest_hit.hit = RAY_NO_HIT;

    for (int s = 0; s < u_shape_cnt; s++)
    {
        if (s == ignore)
            continue;
        s_hit current_hit;
        if (u_shapes[s].type == SHAPE_SPHERE)
            current_hit = intersect_sphere(ray, s);
        else if (u_shapes[s].type == SHAPE_PLANE)
            current_hit = intersect_plane(ray, s);
        // Check if this hit is closer than the previous one
        if (current_hit.hit != RAY_NO_HIT && current_hit.dist < closest_hit.dist)
            closest_hit = current_hit;
    }
    return closest_hit;
}

vec3 random_bounce(s_hit hit, float seed)
{
    return (random_vector(seed));
}

vec4 ray_trace(s_ray ray)
{
    s_hit hit;
    float seed = dot(ray.pos.xy, vec2(12.9898, 78.233)) + u_time;
    vec4 c = vec4(0.0); // takes care of light found along the path
    vec4 m = vec4(1.0); // takes care of color absortion along the path

    // float a = 0.5 * (ray.dir.y + 1.0);
    // vec4 ambient = (1.0 - a) * vec4(1.0) + a * vec4(0.5, 0.7, 1.0, 1.0);
    vec4 ambient = AMBIENT_LIGHT_COLOR;

    for (int b = 0; b < RAY_MAX_BOUNCES; b++)
    {
        hit = intersect_scene(ray, RAY_NO_HIT);

        if (hit.hit == RAY_NO_HIT)
        {
            c += m * ambient;
            break;
        }
        // COMPUTE LIGHT ATTENUATION
        if (u_shapes[hit.hit].mat.emissive != 0)
        {
            c += m * u_shapes[hit.hit].mat.color;
            break;
        }

        // RANDOM BOUNCE DIRECTION
        ray.dir = normalize(random_bounce(hit, seed + b));
        if (dot(hit.normal, ray.dir) < 0) ray.dir = -ray.dir;
        ray.pos = hit.pos + hit.normal * 0.001;

        // This hardcoded diffuse calculation asumes all objects are diffuse
        //      - When using distributed ray bouncing each material will send rays to their
        //      - most common angle of reflection/difraction
        //      - diffuse will be calculated naturally
        //  EXAMPLE: a mterial sending rays to a certain direction is more prone to have recieved the previous ray from a certain direction
        //          so we will backtrace towards that more possible direction
        float diffuse = max(dot(hit.normal, ray.dir), 0.0);
        // COMPUTE COLOR (using new ray since we are backtracing)
        m *= u_shapes[hit.hit].mat.color * diffuse;
    }
    return (c);
}

void main()
{
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
    vec4 prev_color = imageLoad(u_img_output, texel_coord);

    // reinhard tone mapping
    vec4 mapped = color / (color + vec4(1.0));
    color = pow(mapped, vec4(1.0 / u_gamma));

    if (int(u_frame_cnt) > 1)
        color += prev_color;

    imageStore(u_img_output, texel_coord, color);
}