#version 330 core
layout(location = 0) out vec3 gPosition;
layout(location = 1) out vec3 gNormal;
layout(location = 2) out vec3 gAlbedo;
layout(location = 3) out vec3 gKa;
layout(location = 4) out vec3 gKs;
layout(location = 5) out float gNs;

in vec3 position;
in vec3 normal;
in vec2 texCoord;

struct Material {
    vec3 ka;
    vec3 kd;
    vec3 ks;
    float ns; 
    sampler2D texture;
};

uniform Material material;

void main() {
    gPosition = position;
    gNormal = gl_FrontFacing ? normal : -normal;
    gAlbedo = texture(material.texture, texCoord).rgb * material.kd; // Œ∆¿Ì—’…´
    gKa = material.ka;
    gKs = material.ks;
    gNs = material.ns;
}