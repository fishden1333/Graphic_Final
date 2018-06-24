#version 410 core

uniform sampler2D tex;
uniform int mode;
uniform int compare;
uniform int mouse_x;
uniform int mouse_y;
uniform float time_val;

out vec4 color;

in VS_OUT
{
    vec2 texcoord;
} fs_in;

const vec3 grayscale = vec3(0.212, 0.715, 0.072);

vec4 compare_bar(vec4 inColor)
{
    if (compare == 1)
    {
        if (gl_FragCoord.x < 396)
        {
            vec4 texture_color = texture(tex, fs_in.texcoord);
            color = texture_color;
        }
        else if (gl_FragCoord.x >= 396 && gl_FragCoord.x <= 405)
        {
            color = vec4(1.0, 0.0, 0.0, 1.0);
        }
        else
        {
            color = inColor;
        }
    }
    else
    {
        color = inColor;
    }
    return color;
}

vec4 idle()
{
    vec4 texture_color = texture(tex, fs_in.texcoord);
    vec4 color = texture_color;
    return color;
}

vec4 abstraction()
{
    // Median filter
    int half_size = 2;
    vec4 color_sum = vec4(0);
    for (int i = -half_size; i <= half_size; i++)
    {
        for (int j = -half_size; j <= half_size; j++)
        {
            ivec2 coord = ivec2(gl_FragCoord.xy) + ivec2(i, j);
            color_sum += texelFetch(tex, coord, 0);
        }
    }
    int sample_count = (half_size * 2 + 1) * (half_size * 2 + 1);
    vec4 color = color_sum / sample_count;
    
    // Quantization
    float nbins = 8.0;
    vec3 tex_color = color.rgb;
    tex_color = floor(tex_color * nbins) / nbins;
    color = vec4(tex_color, 1.0);
    
    // Difference of Gaussian
    float sigma_e = 2.0f;
    float sigma_r = 2.8f;
    float phi = 3.4f;
    float tau = 0.99f;
    float twoSigmaESquared = 2.0 * sigma_e * sigma_e;
    float twoSigmaRSquared = 2.0 * sigma_r * sigma_r;
    int halfWidth = int(ceil(2.0 * sigma_r));
    int img_size = 800;
    vec2 sum = vec2(0.0);
    vec2 norm = vec2(0.0);
    for (int i = -halfWidth; i <= halfWidth; i++)
    {
        for (int j = -halfWidth; j <= halfWidth; j++)
        {
            float d = length(vec2(i,j));
            vec2 kernel = vec2(exp(-d * d / twoSigmaESquared), exp(-d * d / twoSigmaRSquared));
            vec4 c = texture(tex, fs_in.texcoord + vec2(i, j) / img_size);
            vec2 L = vec2(0.299 * c.r + 0.587 * c.g + 0.114 * c.b);
            norm += 2.0 * kernel;
            sum += kernel * L;
        }
    }
    sum /= norm;
    float H = 100.0 * (sum.x - tau * sum.y);
    float edge = (H > 0.0) ? 1.0 : 2.0 * smoothstep(-2.0, 2.0, phi * H);
    color = mix(vec4(0.0, 0.0, 0.0, 1.0), color, edge);
    return color;
}

vec4 stereo()
{
    vec4 texture_color_Left = texture(tex, fs_in.texcoord - vec2(0.005, 0.0));
    vec4 texture_color_Right = texture(tex, fs_in.texcoord + vec2(0.005, 0.0));
    vec4 texture_color = vec4(texture_color_Left.r * 0.29 + texture_color_Left.g * 0.58 + texture_color_Left.b * 0.114, texture_color_Right.g, texture_color_Right.b, 0.0);
    vec4 color = texture_color;
    return color;
}

vec4 pixelize()
{
    int half_size = 3;
    int full_size = half_size * 2 + 1;
    vec4 color_sum = vec4(0);
    
    for (int i = -half_size; i <= half_size; i++)
    {
        for (int j = -half_size; j <= half_size; j++)
        {
            float x = gl_FragCoord.x / full_size;
            float y = gl_FragCoord.y / full_size;
            ivec2 coord = ivec2(floor(x) * full_size , floor(y) * full_size) + ivec2(i, j);
            color_sum += texelFetch(tex, coord, 0);
        }
    }
    int sample_count = full_size * full_size;
    vec4 color = color_sum / sample_count;
    return color;
}

vec4 laplacian()
{
    vec4 color_sum = vec4(0.0);
    vec3 tex_color;
    
    for (int i = -1; i <= 1; i++)
    {
        for (int j = -1; j <= 1; j++)
        {
            ivec2 coord = ivec2(gl_FragCoord.xy) + ivec2(i, j);
            tex_color = texelFetch(tex, coord, 0).rgb;
            float gray_tex = dot(tex_color, grayscale);
            vec3 pixel_color = vec3(gray_tex);
            if (i == 0 && j == 0)
            {
                color_sum += vec4(pixel_color.r * 8, pixel_color.g * 8, pixel_color.b * 8, 1.0);
            }
            else
            {
                color_sum += vec4(pixel_color.r * -1, pixel_color.g * -1, pixel_color.b * -1, 1.0);
            }
        }
    }
    vec4 color = vec4(tex_color.r * (1 - color_sum.r), tex_color.g * (1 - color_sum.g), tex_color.b * (1 - color_sum.b), 1.0);
    return color;
}

vec4 bloom()
{
    vec4 original_color = texture(tex, fs_in.texcoord);
    
    int half_size = 5;
    vec4 color_sum = vec4(0);
    for (int i = -half_size; i <= half_size; i++)
    {
        for (int j = -half_size; j <= half_size; j++)
        {
            ivec2 coord = ivec2(gl_FragCoord.xy) + ivec2(i, j);
            color_sum += texelFetch(tex, coord, 0);
        }
    }
    int sample_count = (half_size * 2 + 1) * (half_size * 2 + 1);
    vec4 blur_color1 = color_sum / sample_count;
    
    half_size = 10;
    color_sum = vec4(0);
    for (int i = -half_size; i <= half_size; i++)
    {
        for (int j = -half_size; j <= half_size; j++)
        {
            ivec2 coord = ivec2(gl_FragCoord.xy) + ivec2(i, j);
            color_sum += texelFetch(tex, coord, 0);
        }
    }
    sample_count = (half_size * 2 + 1) * (half_size * 2 + 1);
    vec4 blur_color2 = color_sum / sample_count;
    
    vec4 color = original_color * 0.8 + blur_color1 * 0.3 + blur_color2 * 0.2;
    return color;
}

vec4 magnifier()
{
    float x = gl_FragCoord.x;
    float y = gl_FragCoord.y;
    vec4 color;
    
    if (abs(gl_FragCoord.x - mouse_x) <= 100 && abs(gl_FragCoord.y + mouse_y - 600) <= 100)
    {
        
        ivec2 coord = ivec2(floor(x), floor(y)) + ivec2(round((mouse_x - floor(x)) / 2), round((mouse_y - floor(y)) / 2));
        vec4 texture_color = texelFetch(tex, coord, 0);
        color = texture_color;
    }
    else
    {
        vec4 texture_color = texture(tex, fs_in.texcoord);
        color = texture_color;
    }
    return color;
}

// Generate noise
// Reference: http://byteblacksmith.com/improvements-to-the-canonical-one-liner-glsl-rand-for-opengl-es-2-0/
highp float rand(vec2 co)
{
    highp float a = 12.9898;
    highp float b = 78.233;
    highp float c = 43758.5453;
    highp float dt= dot(co.xy, vec2(a, b));
    highp float sn= mod(dt, 3.14);
    return fract(sin(sn) * c);
}

vec4 add_noise()
{
    vec4 texture_color = texture(tex, fs_in.texcoord);
    float random = rand(vec2(gl_FragCoord.x * (time_val + 1), gl_FragCoord.y * (time_val + 1))) + 0.5;
    vec4 color = texture_color * random;
    return color;
}

// Reference: http://www.geeks3d.com/20110428/shader-library-swirl-post-processing-filter-in-glsl/
vec4 twist()
{
    vec2 tex_size = vec2(800, 600);
    vec2 center = vec2(400, 300);
    float radius = 300.0;
    float angle = 0.5;
    vec2 tc = fs_in.texcoord * tex_size;
    tc -= center;
    float dist = length(tc);
    if (dist < radius)
    {
        float percent = (radius - dist) / radius;
        float theta = percent * percent * angle * 8.0;
        float s = sin(theta);
        float c = cos(theta);
        tc = vec2(dot(tc, vec2(c, -s)), dot(tc, vec2(s, c)));
    }
    tc += center;
    vec4 texture_color = texture(tex, tc / tex_size);
    vec4 color = vec4(texture_color.rgb, 1.0);
    return color;
}

// Reference: http://www.geeks3d.com/20091027/shader-library-posterization-post-processing-effect-glsl/
vec4 posterize()
{
    float gamma = 0.6;
    float numColors = 8.0;
    vec3 c = texture(tex, fs_in.texcoord).rgb;
    c = pow(c, vec3(gamma, gamma, gamma));
    c = c * numColors;
    c = floor(c);
    c = c / numColors;
    c = pow(c, vec3(1.0 / gamma));
    color = vec4(c, 1.0);
    return color;
}

void main()
{
    // Idle
    if (mode == 0)
    {
        color = idle();
        color = compare_bar(color);
    }
    
    // Abstraction (Blur + Quantization + Difference of Gaussian)
    else if (mode == 1)
    {
        color = abstraction();
        color = compare_bar(color);
    }
    
    // Red-Blue Stereo
    else if (mode == 2)
    {
        color = stereo();
        color = compare_bar(color);
    }
    
    // Pixelization
    else if (mode == 3)
    {
        color = pixelize();
        color = compare_bar(color);
    }
    
    // Laplacian Filter
    else if (mode == 4)
    {
        color = laplacian();
        color = compare_bar(color);
    }

    // Bloom effect
    else if (mode == 5)
    {
        color = bloom();
        color = compare_bar(color);
    }
    
    // Magnifier
    // else if (mode == 6)
    // {
    //     color = magnifier();
    //     color = compare_bar(color);
    // }
    
    // Add Noise
    else if (mode == 6)
    {
        color = add_noise();
        color = compare_bar(color);
    }
    
    // Twist Effect
    else if (mode == 7)
    {
        color = twist();
        color = compare_bar(color);
    }
    
    // Posterization
    else if (mode == 8)
    {
        color = posterize();
        color = compare_bar(color);
    }
    
    else
    {
        vec4 texture_color = texture(tex, fs_in.texcoord);
        color = texture_color;
    }
    
    if (gl_FragCoord.x >= 20 && gl_FragCoord.x <= 50 && gl_FragCoord.y >= 20 && gl_FragCoord.y <= 50)
    {
        color = vec4(0.0, 0.0, 1.0, 1.0);
    }
    else if (gl_FragCoord.x >= 70 && gl_FragCoord.x <= 100 && gl_FragCoord.y >= 20 && gl_FragCoord.y <= 50)
    {
        color = vec4(0.0, 0.0, 1.0, 1.0);
    }
    else if (gl_FragCoord.x >= 750 && gl_FragCoord.x <= 780 && gl_FragCoord.y >= 20 && gl_FragCoord.y <= 50)
    {
        color = vec4(0.0, 1.0, 0.0, 1.0);
    }
    else if (gl_FragCoord.x >= 20 && gl_FragCoord.x <= 50 && gl_FragCoord.y >= 550 && gl_FragCoord.y <= 580)
    {
        color = vec4(1.0, 1.0, 1.0, 1.0);
    }
    else if (gl_FragCoord.x >= 750 && gl_FragCoord.x <= 780 && gl_FragCoord.y >= 550 && gl_FragCoord.y <= 580)
    {
        color = vec4(1.0, 0.0, 0.0, 1.0);
    }
    else
    {
        color = color;
    }
}
