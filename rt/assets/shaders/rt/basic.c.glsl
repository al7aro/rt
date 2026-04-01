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
const int LAMBERTIAN = 0;
const int SPECULAR = 1;
const int BLINN_PHONG = 2;
const int COOK_TORRANCE = 3;
struct s_material
{
    vec4 color;
    int emissive;
    float p0;   // BRDF model
    float ks;   // ks componente especular
    float kd;   // kd componente difusa
    float m;    // m rugosidad
    float eta;  // refracti
    float p5;   //
    float p6;   //
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
const int SHAPE_QUAD = 3;
const int SHAPE_BOX = 4;
const vec4 AMBIENT_LIGHT_COLOR = vec4(0.1, 0.1, 0.1, 1.0);
const int RAY_MAX_BOUNCES = 20;
const int RAY_NO_HIT = -1;
const int MAX_SHAPES = 10;
const float PI = 3.1415926;

uniform float u_t;
uniform float u_S;
uniform float u_N;
uniform float u_K;

uniform float u_frame_cnt;
uniform float u_time;
uniform float u_rand;
uniform s_camera cam;
uniform int u_shape_cnt;
layout(rgba32f, binding = 0) uniform image2D u_img_display;
layout(rgba32f, binding = 1) uniform image2D u_img_accumulated;
layout (std140, binding = 2) uniform u_scene
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

float trig_area(vec3 a, vec3 b, vec3 c)
{
    return 0.5*(length(cross(b-a, c-a)));
}

int is_point_in_trig(vec3 point, vec3 a, vec3 b, vec3 c)
{
    float a_trig = trig_area(a, b, c);
    float a1 = abs(trig_area(point, b, c));
    float a2 = abs(trig_area(a, point, c));
    float a3 = abs(trig_area(a, b, point));
    float a_tot = a1+a2+a3;
    if (a_tot <= a_trig + 1e-3)
        return (1);
    return (0);
}

s_hit intersect_box(s_ray ray, int s)
{
    s_hit hit;
    hit.hit = RAY_NO_HIT;
    return hit;
}

s_hit intersect_quad(s_ray ray, int s)
{
    s_hit hit;
    vec3 p0;
    vec3 p1;
    p0 = ray.pos;
    p1 = ray.pos + ray.dir;
    hit.hit = RAY_NO_HIT;

    vec3 a = u_shapes[s].v0;
    vec3 b = u_shapes[s].v1;
    vec3 c = u_shapes[s].v2;
    vec3 d = u_shapes[s].v3;
    vec3 p_co = u_shapes[s].pos;
    vec3 p_no = normalize(cross(a-b, a-c));

    vec3 u = p1 - p0;
    float dotp = dot(p_no, u);

    if (dotp < 0.001) // RECTA Y PLANO -> PARALELOS
    {
        vec3 w = p0 - p_co;
        float fac = -dot(p_no, w) / dotp;
        if (fac >= 0.0)
        {
            u = u * fac;
            vec3 pos = p0 + u;  // punto de intereseccion con el plano
                                    // hay que comprobar si esta en el quad
            if (is_point_in_trig(pos, a, b, c) == 0 && is_point_in_trig(pos, a, c, d) == 0)
                return (hit);
            hit.pos = pos;
            hit.hit = s;
            hit.dist = length(hit.pos - ray.pos);
            hit.normal = p_no;
            hit.ray = ray;
        }
    }
    return (hit);
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
        else if (u_shapes[s].type == SHAPE_QUAD)
            current_hit = intersect_quad(ray, s);
        else if (u_shapes[s].type == SHAPE_BOX)
            current_hit = intersect_box(ray, s);
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

// LAMBERT
vec3 lambert_SAMPLE(s_hit hit, float seed)
{
    vec3 ray;
    ray = normalize(random_bounce(hit, seed));
    if (dot(hit.normal, ray) < 0) ray = -ray;
    return (ray);
}
vec4 lambert_BRDF(s_hit hit, vec3 new_dir)
{
    // f_r = color / Pi
    vec4 albedo = u_shapes[hit.hit].mat.color;
    return (albedo / PI);
}
float lambert_PDF(s_hit hit)
{
    // PDF = 1.0 / (2*PI)
    return 1.0 / (2.0*PI);
}

// SPECULAR
vec3 specular_SAMPLE(s_hit hit, float seed)
{
    vec3 ray;
    ray = reflect(hit.ray.dir, hit.normal);
    return (ray);
}
vec4 specular_BRDF(s_hit hit, vec3 new_dir)
{
    vec3 normal = hit.normal;
    vec3 dir = new_dir;
    vec4 albedo = u_shapes[hit.hit].mat.color;
    return (albedo / (dot(normal, dir)));
}
float specular_PDF(s_hit hit)
{
    // PDF = 1.0
    return 1.0;
}

// BLINN PHONG
vec3 blinnphong_SAMPLE(s_hit hit, float seed)
{
    // Lanzo rayos al azar uniformemente en el hemisferio superior
    vec3 ray;
    ray = normalize(random_bounce(hit, seed));
    if (dot(hit.normal, ray) < 0) ray = -ray;
    return (ray);
}
vec4 blinnphong_BRDF(s_hit hit, vec3 new_dir)
{
    s_material mat = u_shapes[hit.hit].mat;
    vec4 albedo = mat.color;
    vec3 n = hit.normal;
    vec3 h = normalize(new_dir - hit.ray.dir);
    float dotNH = max(dot(n, h), 0.0);

// TODO: ANADIR ESTO AL MATERIAL DESDE EL LADO DE C++
    float m = mat.m;  // RUGOSIDAD                -   DEPDENDE DEL MATERIAL
    float kd = mat.kd;     // COMPONENTE DE DIFUSA     -   DEPDENDE DEL MATERIAL
    float ks = mat.ks;     // COMPONENTE DE ESPECULAR  -   DEPDENDE DEL MATERIAL

    float diffuse = kd/PI;
    float specular = ks * ((m+2) / (2*PI)) * pow(dotNH, m);

    return albedo*(diffuse + specular);
}
float blinnphong_PDF(s_hit hit)
{
    return 1.0 / (2.0*PI);
}

// COOK TORRANCE
vec3 cooktorrance_SAMPLE(s_hit hit, float seed)
{
    // Lanzo rayos al azar uniformemente en el hemisferio superior
    vec3 ray;
    ray = normalize(random_bounce(hit, seed));
    if (dot(hit.normal, ray) < 0) ray = -ray;
    return (ray);
}
vec4 cooktorrance_BRDF(s_hit hit, vec3 new_dir)
{
    s_material mat = u_shapes[hit.hit].mat;
    vec4 albedo = mat.color;
    float m = mat.m;      // RUGOSIDAD                -   DEPDENDE DEL MATERIAL
    float eta = mat.eta;     // REF. INDEX               -   DEPDENDE DEL MATERIAL
    float kd = mat.kd;    // COMPONENTE DE DIFUSA     -   DEPDENDE DEL MATERIAL
    float ks = mat.ks;    // COMPONENTE DE ESPECULAR  -   DEPDENDE DEL MATERIAL

    vec3 omega_i = normalize(new_dir);
    vec3 omega_r = normalize(-hit.ray.dir);
    vec3 n = hit.normal;
    vec3 h = normalize(omega_i + omega_r);
    float dotNH = max(dot(n, h), 0.0001);
    float dotNR = max(dot(n, omega_r), 0.0001);
    float dotNI = max(dot(n, omega_i), 0.0001);
    float dotRH = max(dot(omega_r, h), 0.0001);
    float alpha = acos(clamp(dotNH, 0.0001, 0.9999));
    float c = dotRH;
    float g = sqrt(pow(eta, 2) + pow(c, 2) - 1);

    float D = 1.0/(pow(m, 2)*pow(dotNH, 4)) * exp(-pow(tan(alpha) / m, 2));
    float G = min(min(1, (2*dotNH*dotNR/dotRH)), (2*dotNH*dotNI/dotRH));
    float F = (pow(g-c,2)/(2*pow(g+c,2))) * (1 + pow(c*(g+c)-1, 2)/pow(c*(g-c)+1, 2));

    float diffuse = kd/PI;
    float specular = ks * (F*D*G)/(PI*dotNR*dotNI);

    return (albedo*(diffuse + specular));
}
float cooktorrance_PDF(s_hit hit)
{
    // PDF = 1.0 / (2*PI)
    return 1.0 / (2.0*PI);
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

        float cos_theta = 0.0;
        vec3 new_dir;
        vec4 brdf_val;
        float pdf_val;
        // LMABERT
        if (int(u_shapes[hit.hit].mat.p0) == LAMBERTIAN)
        {
            new_dir = lambert_SAMPLE(hit, seed + b);
            brdf_val = lambert_BRDF(hit, new_dir);
            pdf_val = lambert_PDF(hit);
        }
        if (int(u_shapes[hit.hit].mat.p0) == SPECULAR)
        {
            new_dir = specular_SAMPLE(hit, seed + b);
            brdf_val = specular_BRDF(hit, new_dir);
            pdf_val = specular_PDF(hit);
        }
        // BLINN PHONG
        else if (int(u_shapes[hit.hit].mat.p0) == BLINN_PHONG)
        {
            new_dir = blinnphong_SAMPLE(hit, seed + b);
            brdf_val = blinnphong_BRDF(hit, new_dir);
            pdf_val = blinnphong_PDF(hit);
        }
        // COOK TORRANCE
        else if (int(u_shapes[hit.hit].mat.p0) == COOK_TORRANCE)
        {
            new_dir = cooktorrance_SAMPLE(hit, seed + b);
            brdf_val = cooktorrance_BRDF(hit, new_dir);
            pdf_val = cooktorrance_PDF(hit);
        }


        // COMPUTE COLOR ABSORTION
        cos_theta = max(dot(hit.normal, new_dir), 0.0);
        ray.dir = new_dir;
        ray.pos = hit.pos + hit.normal * 0.001;

        // LEY UNIVERSAL DE MONTECARLO: m *= (BRDF * cos) / PDF
        if (pdf_val > 0.001) // Evitar dividir por cero si el rayo sale mal
            m *= (brdf_val * cos_theta) / pdf_val; 
        else
            break;
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
    vec4 prev_color = imageLoad(u_img_accumulated, texel_coord);

    if (int(u_frame_cnt) > 1)
        color = mix(prev_color, color, 1.0/u_frame_cnt);

    imageStore(u_img_accumulated, texel_coord, color);
    color.rgb = color.rgb * (u_S * u_t) / (u_N * u_N * u_K);
    imageStore(u_img_display, texel_coord, color);
}