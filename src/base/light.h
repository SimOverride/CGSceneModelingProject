#pragma once

#include "object.h"
#include "imgui.h"

class Light : public Object {
public:
    float intensity = 1.0f;
    glm::vec3 color = {1.0f, 1.0f, 1.0f};

    Light(const std::string& name) : Object(name) {}

    void renderInspector() override {
        Object::renderInspector();

        ImGui::SliderFloat("Intensity", &intensity, 0.0f, 5.0f);
        ImGui::ColorEdit3("Color", &color[0]);
    }
};

class AmbientLight : public Light {
public:
    AmbientLight(const std::string& name) : Light(name) {}
};

class DirectionalLight : public Light {
public:
    DirectionalLight(const std::string& name) : Light(name) {}
};

class PointLight : public Light {
public:
    float kc = 1.0f;
    float kl = 0.0f;
    float kq = 1.0f;

    PointLight(const std::string& name) : Light(name) {}
};

class SpotLight : public Light {
public:
    float angle = glm::radians(60.0f);
    float kc = 1.0f;
    float kl = 0.0f;
    float kq = 1.0f;

    SpotLight(const std::string& name) : Light(name) {}

    void renderInspector() override {
        Light::renderInspector();

        ImGui::SliderFloat("Angle", &angle, 0.0f, glm::radians(180.0f), "%f rad");
    }
};