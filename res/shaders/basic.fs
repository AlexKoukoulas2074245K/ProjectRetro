#version 330 core

// has to have same name as vertex shader
in vec2 uv_frag;

// our texture
uniform sampler2D tex;

// Texture flipping booleans
uniform vec4 global_white_color = vec4(0.9725, 0.9725, 0.9725, 1.0);
uniform vec4 global_blue_color  = vec4(0.3450, 0.7215, 0.9725, 1.0);
uniform vec4 global_black_color = vec4(0.0941, 0.0941, 0.0941, 1.0);

uniform vec4 current_level_color;
uniform int transition_progression_step;
uniform bool flip_tex_hor;
uniform bool flip_tex_ver;

// actual output
// gl_FragColor is deprecated
out vec4 frag_color;

vec4 getTransitionAnimationColor()
{
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
	if (abs(smallestDistance - whiteColorDistance) < 0.001)             currentColorIndex = 0;
	else if (abs(smallestDistance - currentLevelColorDistance) < 0.001) currentColorIndex = 1;
	else if (abs(smallestDistance - blueColorDistance) < 0.001)         currentColorIndex = 2;
	else                                                                currentColorIndex = 3;	
	
	return colorPool[max((currentColorIndex + transition_progression_step), 3)];
}

void main()
{
    float final_uv_x = uv_frag.x;
    if (flip_tex_hor) final_uv_x = 1.00 - final_uv_x;

    float final_uv_y = 1.00 - uv_frag.y;
    if (flip_tex_ver) final_uv_y = 1.00 - final_uv_y;

    frag_color = texture(tex, vec2(final_uv_x, final_uv_y));
	
	if (transition_progression_step > 0)
	{
		frag_color = getTransitionAnimationColor();
	}	
}
