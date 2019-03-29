#version 330 core

// has to have same name as vertex shader
in vec2 uv_frag;

// our texture
uniform sampler2D tex;

// Texture flipping booleans
uniform bool flip_tex_hor;
uniform bool flip_tex_ver;

// actual output
// gl_FragColor is deprecated
out vec4 frag_color;

void main()
{
    float final_uv_x = uv_frag.x;
    if (flip_tex_hor) final_uv_x = 1.00 - final_uv_x;

    float final_uv_y = 1.00 - uv_frag.y;
    if (flip_tex_ver) final_uv_y = 1.00 - final_uv_y;

    frag_color = texture(tex, vec2(final_uv_x, final_uv_y));
}
