#version 330 core
out vec4 FragColor;

varying vec2 v_uv;

uniform sampler2D u_albedo_map;

void main()
{
    FragColor = vec4(texture(u_albedo_map, v_uv).xyz, 1.0);
}
