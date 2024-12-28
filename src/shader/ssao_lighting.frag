#version 330 core
layout(location = 0) out vec4 fragColor;

in vec2 screenTexCoord;

struct PointLight {
    vec3 position;
    vec3 color;
    float intensity;
    float kc;
    float kl;
    float kq;
};

uniform PointLight light;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedo;
uniform sampler2D ssaoResult;

void main() {
    vec3 position = texture(gPosition, screenTexCoord).xyz;
    vec3 normal = texture(gNormal, screenTexCoord).xyz;
    vec3 albedo = texture(gAlbedo, screenTexCoord).rgb;
    float occlusion = texture(ssaoResult, screenTexCoord).x;

    // TODO: perform lambert shading
    vec3 ambient = vec3(0.3 * occlusion);
    vec3 lighting  = ambient;
    // Âþ·´Éä
    vec3 lightDir = normalize(light.position - position);
    vec3 diffuse = max(dot(normal, lightDir), 0.0) * albedo * light.color * light.intensity;
    float dist = length(light.position - position);
    float attenuation = 1.0 / (light.kc + light.kl * dist + light.kq * dist * dist);
    diffuse  *= attenuation;
    lighting += diffuse;

    fragColor = vec4(lighting, 1.0);
}