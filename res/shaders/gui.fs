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

vec4 getWhiteFlipStep1Color()
{
	if (distance(global_black_color, frag_color) < 0.1)
	{
		return overridden_dark_color;
	}
	else if (distance(primary_light_color, frag_color) < 0.1)
	{
		return global_white_color;
	}
	else if (distance(primary_dark_color, frag_color) < 0.1)
	{
		return overridden_light_color;
	}
	
	return global_white_color;
}

vec4 getWhiteFlipStep2Color()
{
	if (distance(global_black_color, frag_color) < 0.1)
    {
        return global_white_color;
    }
    else if (distance(primary_dark_color, frag_color) < 0.1)
    {
        return primary_light_color;
    }
    else if (distance(primary_light_color, frag_color) < 0.1)
    {
        return primary_dark_color;
    }
    else if (distance(global_white_color, frag_color) < 0.1)
    {
        return global_black_color;
    }
    
    return frag_color;
}

vec4 getWhiteFlipStep3Color()
{
	return global_white_color;
}

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

vec4 getIntroSequenceStep4Color()
{
	if (distance(intro_yellow_color, frag_color) < 0.01)
    {
        return intro_red_color;
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

vec4 getIntroSequenceStep5Color()
{
	if (distance(intro_yellow_color, frag_color) < 0.01)
    {
        return intro_yellow_color;
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

vec4 getIntroSequenceStep6Color()
{
	if (distance(intro_yellow_color, frag_color) < 0.01)
    {
        return global_white_color;
    }
	else if (distance(intro_red_color, frag_color) < 0.01)
	{
		return intro_yellow_color;
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

vec4 getIntroSequenceStep7Color()
{
	if (distance(intro_yellow_color, frag_color) < 0.01)
    {
        return global_white_color;
    }
	else if (distance(intro_red_color, frag_color) < 0.01)
	{
		return global_white_color;
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

vec4 getIntroSequenceStep8Color()
{
	return global_white_color;
}

vec4 getTransitionAnimationColor()
{
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
        if (black_and_white_mode != 0)
        {
            frag_color = getBlackAndWhiteModeColor();
        }
        else if (transition_progression_step != 0 && should_override_primary_colors != 0)
        {
            frag_color = getTransitionAnimationColor();
			return;
        }
		else if (intro_flip_progression_step != 0)
		{
			if (intro_flip_progression_step == 1)
			{
				frag_color = getIntroSequenceStep1Color();
			}
			else if (intro_flip_progression_step == 2)
			{
				frag_color = getIntroSequenceStep2Color();
			}	
			else if (intro_flip_progression_step == 3)
			{
				frag_color = getIntroSequenceStep3Color();
			}
			else if (intro_flip_progression_step == 4)
			{
				frag_color = getIntroSequenceStep4Color();
			}
			else if (intro_flip_progression_step == 5)
			{
				frag_color = getIntroSequenceStep5Color();
			}
			else if (intro_flip_progression_step == 6)
			{
				frag_color = getIntroSequenceStep6Color();
			}
			else if (intro_flip_progression_step == 7)
			{
				frag_color = getIntroSequenceStep7Color();
			}	
			else if (intro_flip_progression_step == 8)
			{
				frag_color = getIntroSequenceStep8Color();
			}		
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
		
		if (white_flip_progression_step != 0)
		{
			if (white_flip_progression_step == 1)
			{
				frag_color = getWhiteFlipStep1Color();
			}
			else if (white_flip_progression_step == 2)
			{
				frag_color = getWhiteFlipStep2Color();
			}
			else if (white_flip_progression_step == 3)
			{
				frag_color = getWhiteFlipStep3Color();
			}
		}		
		else if (should_override_primary_colors != 0)
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
