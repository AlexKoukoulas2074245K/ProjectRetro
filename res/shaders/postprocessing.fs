#version 330 core

// has to have same name as vertex shader
in vec2 uv_frag;

// our texture
uniform sampler2D tex;
uniform float swirlRadius;
uniform float swirlAngle;
uniform float blurIntensity;
uniform vec2 swirlCenter;
uniform vec2 swirlDimensions;

// actual output
// gl_FragColor is deprecated
out vec4 frag_color;

float PI = 3.14159265359;
float EULER = 2.71828182846;

// Swirl post-processing effect
vec4 swirlEffect()
{
    vec2 tc = uv_frag * swirlDimensions;
    tc -= swirlCenter;
    float dist = length(tc);
    if (dist < swirlRadius) 
    {
        float percent = (swirlRadius - dist)/swirlRadius;
        float theta = percent * percent * swirlAngle * 8.0;
        float angleSine = sin(theta);
        float angleCosine = cos(theta);
        tc = vec2(dot(tc, vec2(angleCosine, -angleSine)), dot(tc, vec2(angleSine, angleCosine)));
    }

    tc += swirlCenter;
    return vec4(texture(tex, tc/swirlDimensions).rgb, 1.);
}

// Box blur post-processing effect
float mapBlurIntensityToPixelContribution(float intensity)
{	
	return 0.111111 + 0.888888 * (1.0 - intensity);
}

vec4 boxBlur(vec4 frag_color, float intensity)
{
	float hor_offset = 0.0015625;
	float ver_offset = 0.0027777;
		
	float mainPixelContribution = mapBlurIntensityToPixelContribution(intensity);
	float otherPixelsContribution = (1.0 - mainPixelContribution)/8.0;
	
	vec4 blurredColor = 
		frag_color * mainPixelContribution +
		vec4(texture(tex, vec2(uv_frag.x - hor_offset, uv_frag.y - ver_offset)).rgb, 1.0) * otherPixelsContribution +
		vec4(texture(tex, vec2(uv_frag.x             , uv_frag.y - ver_offset)).rgb, 1.0) * otherPixelsContribution +
		vec4(texture(tex, vec2(uv_frag.x + hor_offset, uv_frag.y - ver_offset)).rgb, 1.0) * otherPixelsContribution +
		vec4(texture(tex, vec2(uv_frag.x - hor_offset, uv_frag.y             )).rgb, 1.0) * otherPixelsContribution +
		vec4(texture(tex, vec2(uv_frag.x + hor_offset, uv_frag.y             )).rgb, 1.0) * otherPixelsContribution +
		vec4(texture(tex, vec2(uv_frag.x - hor_offset, uv_frag.y + ver_offset)).rgb, 1.0) * otherPixelsContribution +
		vec4(texture(tex, vec2(uv_frag.x             , uv_frag.y + ver_offset)).rgb, 1.0) * otherPixelsContribution +
		vec4(texture(tex, vec2(uv_frag.x + hor_offset, uv_frag.y + ver_offset)).rgb, 1.0) * otherPixelsContribution;
		
	return blurredColor;
	
}

float calculateGaussianValue(int x, int y, float sigma)
{
    return (1.0/(2.0 * PI * (sigma * sigma))) * exp(-((x * x + y * y)/(2.0 * (sigma * sigma))));
}

vec4 gaussianBlur(vec4 frag_color, float intensity)
{
    // Avoid division by zero
    if (intensity < 0.001)
    {
        return frag_color;
    }
    
    // Pixel offsets
    float hor_offset = 0.0015625;
    float ver_offset = 0.0027777;
    
    // Multidim arrays not supported my ass
    float kernelRow0[9];
    float kernelRow1[9];
    float kernelRow2[9];
    float kernelRow3[9];
    float kernelRow4[9];
    float kernelRow5[9];
    float kernelRow6[9];
    float kernelRow7[9];
    float kernelRow8[9];
    
    // Build weights array and accumulate generated weights
    float weightAccumulator = 0.0;
    for (int x = -4; x <= 4; ++x)
    {
        kernelRow0[x + 4] = calculateGaussianValue(x, +4, intensity); weightAccumulator += kernelRow0[x + 4];
        kernelRow1[x + 4] = calculateGaussianValue(x, +3, intensity); weightAccumulator += kernelRow1[x + 4];
        kernelRow2[x + 4] = calculateGaussianValue(x, +2, intensity); weightAccumulator += kernelRow2[x + 4];
        kernelRow3[x + 4] = calculateGaussianValue(x, +1, intensity); weightAccumulator += kernelRow3[x + 4];
        kernelRow4[x + 4] = calculateGaussianValue(x, +0, intensity); weightAccumulator += kernelRow4[x + 4];
        kernelRow5[x + 4] = calculateGaussianValue(x, -1, intensity); weightAccumulator += kernelRow5[x + 4];
        kernelRow6[x + 4] = calculateGaussianValue(x, -2, intensity); weightAccumulator += kernelRow6[x + 4];
        kernelRow7[x + 4] = calculateGaussianValue(x, -3, intensity); weightAccumulator += kernelRow7[x + 4];
        kernelRow8[x + 4] = calculateGaussianValue(x, -4, intensity); weightAccumulator += kernelRow8[x + 4];
    }
    
    frag_color = vec4(0.0);
    for (int x = -4; x <= 4; ++x)
    {
        frag_color += vec4(texture(tex, vec2(uv_frag.x + x * hor_offset, uv_frag.y + 4.0 * ver_offset)).rgb, 1.0) * (kernelRow0[x + 4]/weightAccumulator);
        frag_color += vec4(texture(tex, vec2(uv_frag.x + x * hor_offset, uv_frag.y + 3.0 * ver_offset)).rgb, 1.0) * (kernelRow1[x + 4]/weightAccumulator);
        frag_color += vec4(texture(tex, vec2(uv_frag.x + x * hor_offset, uv_frag.y + 2.0 * ver_offset)).rgb, 1.0) * (kernelRow2[x + 4]/weightAccumulator);
        frag_color += vec4(texture(tex, vec2(uv_frag.x + x * hor_offset, uv_frag.y + 1.0 * ver_offset)).rgb, 1.0) * (kernelRow3[x + 4]/weightAccumulator);
        frag_color += vec4(texture(tex, vec2(uv_frag.x + x * hor_offset, uv_frag.y + 0.0 * ver_offset)).rgb, 1.0) * (kernelRow4[x + 4]/weightAccumulator);
        frag_color += vec4(texture(tex, vec2(uv_frag.x + x * hor_offset, uv_frag.y - 1.0 * ver_offset)).rgb, 1.0) * (kernelRow5[x + 4]/weightAccumulator);
        frag_color += vec4(texture(tex, vec2(uv_frag.x + x * hor_offset, uv_frag.y - 2.0 * ver_offset)).rgb, 1.0) * (kernelRow6[x + 4]/weightAccumulator);
        frag_color += vec4(texture(tex, vec2(uv_frag.x + x * hor_offset, uv_frag.y - 3.0 * ver_offset)).rgb, 1.0) * (kernelRow7[x + 4]/weightAccumulator);
        frag_color += vec4(texture(tex, vec2(uv_frag.x + x * hor_offset, uv_frag.y - 4.0 * ver_offset)).rgb, 1.0) * (kernelRow8[x + 4]/weightAccumulator);
    }
    
    return frag_color;
}

void main()
{
    frag_color = swirlEffect();
    frag_color = gaussianBlur(frag_color, blurIntensity);
}
