#version 330 core
layout(location = 0) out vec4 fragColor;

in vec2 screenTexCoord;

struct AmbientLight {
    float intensity;
    vec3 color;
};

struct DirectionalLight {
    vec3 direction;
    float intensity;
    vec3 color;
};

struct PointLight {
    vec3 position;
    float intensity;
    vec3 color;
    float kc;
    float kl;
    float kq;
};

struct SpotLight {
    vec3 position;
    vec3 direction;
    float intensity;
    vec3 color;
    float angle;
    float kc;
    float kl;
    float kq;
};

uniform AmbientLight ambientLight;
uniform int nDirectionalLight;
uniform DirectionalLight directionalLights[10];
uniform int nPointLight;
uniform PointLight pointLights[10];
uniform int nSpotLight;
uniform SpotLight spotLights[10];
uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedo;
uniform sampler2D gKa;
uniform sampler2D gKs;
uniform sampler2D gNs;
uniform sampler2D ssaoResult;
uniform vec3 viewPos;

vec3 calcDirectionalLight(DirectionalLight light, vec3 normal, vec3 viewDir, vec3 albedo, vec3 ks, float ns) {
    vec3 lightDir = normalize(-light.direction);
    vec3 reflectDir = reflect(-lightDir, normal);
    vec3 diffuse = max(dot(lightDir, normal), 0.0f) * light.color * albedo;
    vec3 specular = pow(max(dot(viewDir, reflectDir), 0.0), ns) * light.color * ks;
    return (diffuse + specular) * light.intensity;
}

vec3 calcPointLight(PointLight light, vec3 position, vec3 normal, vec3 viewDir, vec3 albedo, vec3 ks, float ns) {
    vec3 lightDir = normalize(light.position - position);
    vec3 reflectDir = reflect(-lightDir, normal);
    vec3 diffuse = max(dot(normal, lightDir), 0.0f) * light.color * albedo;
    vec3 specular = pow(max(dot(viewDir, reflectDir), 0.0), ns) * light.color * ks;
    float distance = length(light.position - position);
    float attenuation = 1.0f / (light.kc + light.kl * distance + light.kq * distance * distance);
    return (diffuse + specular) * light.intensity * attenuation;
}

vec3 calcSpotLight(SpotLight light, vec3 position, vec3 normal, vec3 viewDir, vec3 albedo, vec3 ks, float ns) {
    vec3 lightDir = normalize(light.position - position);
    vec3 reflectDir = reflect(-lightDir, normal);
    float theta = acos(-dot(lightDir, normalize(light.direction)));
    if (theta > light.angle) {
        return vec3(0.0f);
    }
    vec3 diffuse = max(dot(lightDir, normal), 0.0f) * light.color * albedo;
    vec3 specular = pow(max(dot(viewDir, reflectDir), 0.0), ns) * light.color * ks;
    float distance = length(light.position - position);
    float attenuation = 1.0f / (light.kc + light.kl * distance + light.kq * distance * distance);
    return (diffuse + specular) * light.intensity * attenuation;
}

void main() {
    vec3 position = texture(gPosition, screenTexCoord).xyz;
    vec3 normal = texture(gNormal, screenTexCoord).xyz;
    vec3 albedo = texture(gAlbedo, screenTexCoord).rgb;
    vec3 ka = texture(gKa, screenTexCoord).rgb;
    vec3 ks = texture(gKs, screenTexCoord).rgb;
    float ns = texture(gNs, screenTexCoord).r;
    float occlusion = texture(ssaoResult, screenTexCoord).x;
    vec3 viewDir = normalize(viewPos - position);
    
    if (length(position) == 0.0) {
        // Ìì¿ÕºÐÏñËØ
        fragColor = vec4(albedo, 1.0);
        return;
    }
    
    vec3 ambient = occlusion * ka * ambientLight.color * ambientLight.intensity;
    vec3 result  = ambient;
    
    for (int i = 0; i < nDirectionalLight; i++) {
        result += calcDirectionalLight(directionalLights[i], normal, viewDir, albedo, ks, ns);
    }
    for (int i = 0; i < nPointLight; i++) {
        result += calcPointLight(pointLights[i], position, normal, viewDir, albedo, ks, ns);
    }
    for (int i = 0; i < nSpotLight; i++) {
        result += calcSpotLight(spotLights[i], position, normal, viewDir, albedo, ks, ns);
    }

    fragColor = vec4(result, 1.0);
}