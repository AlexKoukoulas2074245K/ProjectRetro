#version 330 core

// has to have same name as vertex shader
in vec2 uv_frag;

// our texture
uniform sampler2D tex;

// Texture flipping booleans
uniform vec4 global_white_color  = vec4(0.9725, 0.9725, 0.9725, 1.0);
uniform vec4 global_blue_color   = vec4(0.3450, 0.7215, 0.9725, 1.0);
uniform vec4 global_black_color  = vec4(0.0941, 0.0941, 0.0941, 1.0);
uniform vec4 global_yellow_color = vec4(0.9725, 0.9803, 0.0000, 1.0);

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

uniform bool flip_tex_hor;
uniform bool flip_tex_ver;

// actual output
// gl_FragColor is deprecated
out vec4 frag_color;

vec4 getTransitionAnimationColor()
{
	if (transition_progression_step > 0)
	{
		return frag_color;
	}
	
	if (transition_progression_step < -3)
	{
		return global_white_color;
	}

	vec4 colorPool[4]; 
	colorPool[0] = global_white_color;
	colorPool[1] = overridden_light_color;
	colorPool[2] = overridden_dark_color;
	colorPool[3] = global_black_color;
	
	float whiteColorDistance           = distance(colorPool[0], frag_color);
	float overriddenLightColorDistance = distance(colorPool[1], frag_color);
	float overriddenDarkColorDistance  = distance(colorPool[2], frag_color);
	float blackColorDistance           = distance(colorPool[3], frag_color);
	
	float smallestDistance = min(whiteColorDistance, min(overriddenLightColorDistance, min(overriddenDarkColorDistance, blackColorDistance)));
	
	int currentColorIndex = 0;
	if (abs(smallestDistance - whiteColorDistance) < 0.01)                currentColorIndex = 0;
	else if (abs(smallestDistance - overriddenLightColorDistance) < 0.01) currentColorIndex = 1;
	else if (abs(smallestDistance - overriddenDarkColorDistance) < 0.01)  currentColorIndex = 2;
	else                                                                  currentColorIndex = 3;
	 
	return colorPool[max(0, min((currentColorIndex + transition_progression_step), 3))];
}

void main()
{
    float finalUvX = uv_frag.x;
    if (flip_tex_hor) finalUvX = 1.00 - finalUvX;

    float finalUvY = 1.00 - uv_frag.y;
    if (flip_tex_ver) finalUvY = 1.00 - finalUvY;

    frag_color = texture(tex, vec2(finalUvX, finalUvY));
	
	if (frag_color.w > 0.5)
	{
        if (transition_progression_step != 0 && should_override_primary_colors != 0)
        {
            frag_color = getTransitionAnimationColor();
			return;
        }
	}	
}
