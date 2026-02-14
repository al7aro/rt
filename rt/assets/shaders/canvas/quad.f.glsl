#version 460 core

out vec4 frag_color;
in vec2 v_tex_coord;

uniform sampler2D u_texture;
uniform float u_frame_cnt;

void main()
{
        frag_color = texture(u_texture, v_tex_coord) / u_frame_cnt;
        // frag_color = texture(u_texture, v_tex_coord);
}