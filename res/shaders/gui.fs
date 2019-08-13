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

uniform float global_x_offset;
uniform float global_y_offset;

uniform bool flip_tex_hor;
uniform bool flip_tex_ver;

// actual output
// gl_FragColor is deprecated
out vec4 frag_color;

vec4 getBlackAndWhiteModeColor()
{
    if (distance(global_white_color, frag_color) < 0.01)
    {
        return global_white_color;
    }
    else
    {
        return global_black_color;
    }
}

vec4 getTransitionAnimationColor()
{
    if (transition_progression_step < -3)
    {
        return global_white_color;
    }
    
	vec4 colorPool[4]; 
	colorPool[0] = global_white_color;
	colorPool[1] = current_level_color;
	colorPool[2] = global_blue_color;
	colorPool[3] = global_black_color;
	
	float whiteColorDistance        = distance(colorPool[0], frag_color);
	float currentLevelColorDistance = distance(colorPool[1], frag_color);
	float blueColorDistance         = distance(colorPool[2], frag_color);
	float blackColorDistance        = distance(colorPool[3], frag_color);
	
	float smallestDistance = min(whiteColorDistance, min(currentLevelColorDistance, min(blueColorDistance, blackColorDistance)));
	
	int currentColorIndex = 0;
	if (abs(smallestDistance - whiteColorDistance) < 0.01)             currentColorIndex = 0;
	else if (abs(smallestDistance - currentLevelColorDistance) < 0.01) currentColorIndex = 1;
	else if (abs(smallestDistance - blueColorDistance) < 0.01)         currentColorIndex = 2;
	else                                                               currentColorIndex = 3;
	
	return colorPool[max(0, min((currentColorIndex + transition_progression_step), 3))];
}

vec4 getPaletteColor()
{
    if (distance(global_black_color, frag_color) < 0.1)
    {
        return global_black_color;
    }
    else if (distance(global_blue_color, frag_color) < 0.1)
    {
        return global_blue_color;
    }
    else if (distance(global_white_color, frag_color) < 0.1)
    {
        return global_white_color;
    }
    
    return current_level_color;
}

vec4 getDarkFlipStep1Color()
{
    if (distance(global_black_color, frag_color) < 0.1)
    {
        return global_yellow_color;
    }
    else if (distance(global_white_color, frag_color) < 0.1)
    {
        return global_black_color;
    }
    
    return frag_color;
}

vec4 getDarkFlipStep2Color()
{
    if (distance(global_black_color, frag_color) < 0.1)
    {
        return global_white_color;
    }
    else if (distance(global_white_color, frag_color) < 0.1)
    {
        return global_black_color;
    }
    
    return frag_color;
}

vec4 getDarkFlipStep3Color()
{
    return global_white_color;
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
        if (black_and_white_mode != 0)
        {
            frag_color = getBlackAndWhiteModeColor();
        }
        else if (transition_progression_step != 0)
        {
            frag_color = getTransitionAnimationColor();
        }
		else if (dark_flip_progression_step != 0)
		{
			if (dark_flip_progression_step == 1)
			{
				frag_color = getDarkFlipStep1Color();
			}
			else if (dark_flip_progression_step == 2)
			{
				frag_color = getDarkFlipStep2Color();
			}
			else if (dark_flip_progression_step == 3)
			{
				frag_color = getDarkFlipStep3Color();
			}
		}
		if (should_override_primary_colors != 0)
		{			
			if (distance(primary_light_color, frag_color) < 0.1)
			{
				frag_color = overridden_light_color;
			}
			else if (distance(primary_dark_color, frag_color) < 0.1)
			{
				frag_color = overridden_dark_color;
			}
		}
	}	
}
