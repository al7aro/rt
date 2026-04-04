#version 460 core

out vec4 frag_color;
in vec2 v_tex_coord;

uniform sampler2D u_texture;
uniform float u_t;
uniform float u_S;
uniform float u_N;
uniform float u_K;

void main()
{
    float hdr_scale = (u_S * u_t) / (u_N * u_N * u_K);
    frag_color = hdr_scale * texture(u_texture, v_tex_coord);
}