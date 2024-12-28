#version 330 core
layout(location = 0) out float ssaoResult;

const int nSamples = 64;
const float radius = 1.0f;

uniform int screenWidth;
uniform int screenHeight;
uniform float zNear;
uniform float zFar;
uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gDepth;
uniform sampler2D noiseMap;
uniform vec3 sampleVecs[64];
uniform mat4 projection;

in vec2 screenTexCoord;

void main() {
    // TODO: perform SSAO
    vec2 noiseScale = vec2(screenWidth / 4.0, screenHeight / 4.0);
    vec3 fragPos = texture(gPosition, screenTexCoord).xyz;
    vec3 normal = texture(gNormal, screenTexCoord).rgb;
    vec3 randomVec = texture(noiseMap, screenTexCoord * noiseScale).xyz;
    vec3 tangent = normalize(randomVec - normal * dot(randomVec, normal));
    vec3 bitangent = cross(normal, tangent);
    mat3 TBN = mat3(tangent, bitangent, normal);
    
    float occlusion = 0.0;
    for (int i = 0; i < nSamples; i++)
    {
        // 获取样本位置
        vec3 sample = TBN * sampleVecs[i]; // 切线->观察空间
        sample = fragPos + sample * radius; 
        vec4 offset = vec4(sample, 1.0);
        offset = projection * offset; // 观察->裁剪空间
        offset.xyz /= offset.w; // 透视划分
        offset.xyz = offset.xyz * 0.5 + 0.5; // 变换到0.0 - 1.0的值域
        float sampleDepth = texture(gDepth, offset.xy).r;
        float z = sampleDepth * 2.0 - 1.0; // 回到NDC
        sampleDepth = (2.0 * zNear * zFar) / (zFar + zNear - z * (zFar - zNear));
        float rangeCheck = smoothstep(0.0, 1.0, radius / abs(fragPos.z + sampleDepth));
        occlusion += (-sampleDepth >= sample.z ? 1.0 : 0.0) * rangeCheck;
        // occlusion += (sampleDepth >= sample.z ? 1.0 : 0.0);
    }
    ssaoResult = 1.0 - (occlusion / nSamples);
}