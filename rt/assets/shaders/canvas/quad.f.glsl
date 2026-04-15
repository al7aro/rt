#version 460 core

out vec4 frag_color;
in vec2 v_tex_coord;

uniform sampler2D u_texture;
// uniform float u_t;
// uniform float u_S;
// uniform float u_N;
// uniform float u_K;
uniform float u_hdr_cte;

void main()
{
    // float hdr_cte = (u_S * u_t) / (u_N * u_N * u_K);
    float hdr_cte = u_hdr_cte;
    frag_color = hdr_cte * texture(u_texture, v_tex_coord);
}