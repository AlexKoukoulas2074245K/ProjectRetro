#version 330 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 uv;

uniform mat4 world;

// will be used in fragment shader
out vec2 uv_frag;

void main()
{
    uv_frag = uv;
    gl_Position = vec4(position, 1.0);
}
