#version 460 core

out vec4 frag_color;
in vec2 v_tex_coord;

uniform sampler2D u_texture;
uniform float u_frame_cnt;

uniform float u_t;
uniform float u_S;
uniform float u_N;
uniform float u_K;

void main()
{
    vec4 color = texture(u_texture, v_tex_coord);
    color.rgb = color.rgb / u_frame_cnt;
    color.rgb = color.rgb * (u_S * u_t) / (u_N * u_N * u_K);

    frag_color = vec4(color.rgb, 1.0);
}