#version 330 core

// has to have same name as vertex shader
in vec2 uv_frag;

// our texture
uniform sampler2D tex;

uniform vec4 global_white_color  = vec4(0.9725, 0.9725, 0.9725, 1.0);
uniform vec4 global_blue_color   = vec4(0.3450, 0.7215, 0.9725, 1.0);
uniform vec4 global_black_color  = vec4(0.0941, 0.0941, 0.0941, 1.0);
uniform vec4 global_yellow_color = vec4(0.9725, 0.9803, 0.0000, 1.0);
uniform vec4 intro_yellow_color  = vec4(0.97254902, 0.97254902, 0.0000, 1.0);
uniform vec4 intro_red_color     = vec4(0.97254902, 0.031372549, 0.031372549, 1.0);

uniform vec4 current_level_color;
uniform vec4 primary_light_color;
uniform vec4 primary_dark_color;
uniform vec4 overridden_light_color;
uniform vec4 overridden_dark_color;

uniform int should_override_primary_colors;
uniform int transition_progression_step;
uniform int black_and_white_mode;
uniform int dark_flip_progression_step;
uniform int white_flip_progression_step;
uniform int intro_flip_progression_step;

uniform float global_x_offset;
uniform float global_y_offset;

// Texture flipping booleans
uniform bool flip_tex_hor;
uniform bool flip_tex_ver;

// actual output
// gl_FragColor is deprecated
out vec4 frag_color;

vec4 getIntroSequenceStep1Color()
{
	if (distance(intro_yellow_color, frag_color) < 0.01)
    {
        return intro_yellow_color;
    }
	else if (distance(intro_red_color, frag_color) < 0.01)
	{
		return intro_yellow_color;
	}
	else if (distance(global_black_color, frag_color) < 0.01)
	{
		return intro_yellow_color;
	}
	else
	{
		return frag_color;
	}
}

vec4 getIntroSequenceStep2Color()
{
	if (distance(intro_yellow_color, frag_color) < 0.01)
    {
        return intro_red_color;
    }
	else if (distance(intro_red_color, frag_color) < 0.01)
	{
		return intro_red_color;
	}
	else if (distance(global_black_color, frag_color) < 0.01)
	{
		return intro_red_color;
	}
	else
	{
		return frag_color;
	}
}

vec4 getIntroSequenceStep3Color()
{
	if (distance(intro_yellow_color, frag_color) < 0.01)
    {
        return global_black_color;
    }
	else if (distance(intro_red_color, frag_color) < 0.01)
	{
		return global_black_color;
	}
	else if (distance(global_black_color, frag_color) < 0.01)
	{
		return global_black_color;
	}
	else
	{
		return frag_color;
	}
}

void main()
{
    float finalUvX = uv_frag.x;
    if (flip_tex_hor) finalUvX = 1.00 - finalUvX;

    float finalUvY = 1.00 - uv_frag.y;
    if (flip_tex_ver) finalUvY = 1.00 - finalUvY;

    frag_color = texture(tex, vec2(finalUvX, finalUvY));
	
	if (intro_flip_progression_step == 0)
	{
		return getIntroSequenceStep1Color();
	}
	else if (intro_flip_progression_step == 1)
	{
		return getIntroSequenceStep2Color();
	}
	else if (intro_flip_progression_step == 2)
	{
		return getIntroSequenceStep3Color();
	}	
}
