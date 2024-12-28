#version 330 core
out vec4 FragColor;

in vec2 screenTexCoord;

uniform sampler2D image;

uniform bool horizontal;

const float weight[5] = float[] (
    0.2270270270f, 0.1945945946f, 0.1216216216f, 0.0540540541f, 0.0162162162f);

void main() {
    // TODO: perform gaussian blur
    vec2 tex_offset = 1.0 / textureSize(image, 0); // gets size of single texel
    vec3 result = texture(image, screenTexCoord).rgb * weight[0]; // current fragment's contribution
    if(horizontal)
    {
        for(int i = 1; i < 5; ++i)
        {
            result += texture(image, screenTexCoord + vec2(tex_offset.x * i, 0.0)).rgb * weight[i];
            result += texture(image, screenTexCoord - vec2(tex_offset.x * i, 0.0)).rgb * weight[i];
        }
    }
    else
    {
        for(int i = 1; i < 5; ++i)
        {
            result += texture(image, screenTexCoord + vec2(0.0, tex_offset.y * i)).rgb * weight[i];
            result += texture(image, screenTexCoord - vec2(0.0, tex_offset.y * i)).rgb * weight[i];
        }
    }
    FragColor = vec4(result, 1.0);
}